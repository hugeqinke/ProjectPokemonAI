#ifndef Logger_Hpp
#define Logger_Hpp
// impltion for a core logger
// reinvinting the wheel here for thread-safe practice 

// continuing to use posix here for sake of consistency

#include <iostream>
#include <sstream> 
#include <fstream>
#include <ctime>
#include <vector> 
#include <pthread.h> 
#include <unistd.h>
#include <sys/msg.h>
#include <queue>
#include <csignal>

namespace tlog {
    // global to the namespace
    // we could create a separate class for a flip queue,
    // but no...
    struct Parcel {
        std::string msg;
        std::string level;  
        time_t timer;  

        Parcel (std::string msg, std::string level, time_t timer)
                : msg(msg), level(level), timer(timer) {

        }
    };

    std::vector<std::queue<Parcel>> fq = {
        std::queue<Parcel>(),
        std::queue<Parcel>()
    };

    int active = 0; 
    int flush = 1; 
    bool terminateflag = false;
    bool loginitialized = false;

    pthread_mutex_t mqmutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t activemutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t flushmutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t terminatemutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_t consumer_tid; 
    pthread_t swapper_tid; 

    // thread tasks 
    struct consumer_args {
        char processname[100];
    };

    void * consumer (void * arg);
    void * swap (void * arg); 

    // helper functions
    void sigTerminate(); 
    void flush_stream(int queue_id, std::ofstream& ofs); 
    class Log {
    public:
        static Log & Instance();
 
        Log (const Log&) = delete; 
        Log& operator= (const Log&) = delete; 
 
        void logError(std::string entry) {
            write(entry, "Error"); 
        }

        void logException(std::string entry) {
            write(entry, "Exception");
        }

        void logWarn(std::string entry) {
            write(entry, "Warn");
        }

        void logDebug(std::string entry) {
            write(entry, "Debug"); 
        }

        void logInfo(std::string entry) { 
            write(entry, "Info");
        }

        void logSysError(std::string entry) {
            int error_num; 
            char err_buff[200]; 
            
            error_num = strerror_r ( errno, err_buff, 200 );
            
            std::stringstream ss;  
            ss << entry << ": " << err_buff;
            write(ss.str(), "SysErr");  
        }
        // we've got no reflection, so use this to initialize logging
        void activate(const char* filename) {   
            struct consumer_args * args = new struct consumer_args(); 
            strcpy(args->processname, filename); 

            pthread_create (&tlog::consumer_tid, 0, consumer, args); 
            pthread_create (&tlog::swapper_tid, 0, swap, 0);
        } 
    private:
        Log () { }

        ~Log () {
            // activate the kill switch
            sigTerminate(); 

            pthread_join(tlog::swapper_tid, NULL); 
            pthread_join(tlog::consumer_tid, NULL); 
        }
        
        void write(std::string msg, std::string level) {
            time_t timer; 
            time(&timer); 
            pthread_mutex_lock(&activemutex); 
            fq.at(active).emplace(msg, level, timer); 
            pthread_mutex_unlock(&activemutex);
        }
    };

    Log & Log::Instance () {
        static Log instance; 
        return instance;     
    } 

    void sigTerminate() {
        pthread_mutex_lock (&terminatemutex); 
        terminateflag = true; 
        pthread_mutex_unlock (&terminatemutex); 
    }

    bool isTerminated() {
        bool ret; 
        pthread_mutex_lock (&terminatemutex);  
        // this flag won't change after this flag is true, so its fine if we store this separately
        ret = terminateflag;
        pthread_mutex_unlock (&terminatemutex); 
        return ret;
    }

    void * swap (void * arg) {
        while (true) {
            if (isTerminated()) break;
            
            sleep(2);  // we swap every 2 seconds
        
            pthread_mutex_lock (&activemutex); 
            pthread_mutex_lock (&flushmutex);

            // flip both indices       
            active = 1 - active; 
            flush = 1 - flush; 
             
            pthread_mutex_unlock (&flushmutex); 
            pthread_mutex_unlock (&activemutex); 
        }
        pthread_exit(NULL);  
    } 

    void finalFlush(std::ofstream& ofs) {
        pthread_mutex_lock (&activemutex); 
        pthread_mutex_lock (&flushmutex);
        flush_stream(flush, ofs); 
        flush_stream(active, ofs); 
        ofs.close(); 
        pthread_mutex_unlock (&flushmutex); 
        pthread_mutex_unlock (&activemutex); 
    } 

    void * consumer (void * arg) {
        struct consumer_args* threadinfo = (struct consumer_args*) arg; 
       
        pid_t pid = getpid(); 
   
        time_t timer; 
        char time_buffer[80];
        struct tm* tm_info; 

        time (&timer); 
        tm_info = localtime(&timer);

        strftime(time_buffer, 80, "%Y%m%dT%H%M%S", tm_info); 
          
        std::stringstream filename; 
        filename << threadinfo->processname
                 << "." << pid << "." << time_buffer
                 << ".log";

        std::ofstream ofs;  
        ofs.open(filename.str()); 
        
        while (true) {
            if (isTerminated()) {
                finalFlush(ofs); 
                break;
            }
            
            // don't think we need to lock this 
            pthread_mutex_lock (&flushmutex); 
            flush_stream(flush, ofs); 
            pthread_mutex_unlock (&flushmutex); 
        }
        pthread_exit(NULL); 
    } 

    void flush_stream(int queue_id, std::ofstream& ofs) {
        while (!fq.at(queue_id).empty()) { 
            struct Parcel* parcel = &fq.at(queue_id).front(); 
            fq.at(queue_id).pop();

            struct tm* tm_info; 
            char time_buffer[80];
            tm_info = localtime(&parcel->timer);
            // no millisecond resolution
            // add it if necessary
            strftime(time_buffer, 80, "[%Y-%m-%d %H:%M:%S]", tm_info); 

            ofs << time_buffer << " " 
                << "<" << parcel->level << "> : "
                << parcel->msg 
                << std::endl;
            ofs.flush(); 
        }
    }
}

#endif // Logger_Hpp
