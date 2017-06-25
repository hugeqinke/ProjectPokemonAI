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

    extern std::vector<std::queue<Parcel>> fq; 
 
    extern int active; 
    extern int flush; 
    extern bool terminateflag; 
    extern bool loginitialized; 

    extern pthread_mutex_t mqmutex; 
    extern pthread_mutex_t activemutex; 
    extern pthread_mutex_t flushmutex;
    extern pthread_mutex_t terminatemutex; 

    extern pthread_t consumer_tid; 
    extern pthread_t swapper_tid; 

    // thread tasks 
    struct consumer_args {
        char processname[100];
        int pid; 
    };

    inline void * consumer (void * arg);
    inline void * swap (void * arg); 
    inline void sigTerminate(); 
    inline bool isTerminated(); 
    inline void finalFlush(std::ofstream& ofs); 
    inline void flush_stream(int queue_id, std::ofstream& ofs); 

    // helper functions
    inline void sigTerminate(); 
    inline void flush_stream(int queue_id, std::ofstream& ofs); 
    
    class Log {
    public:
        Log (const Log&) = delete; 
        Log& operator= (const Log&) = delete; 
 
        void logError(std::string entry); 
        void logException(std::string entry); 
        void logWarn(std::string entry); 
        void logDebug(std::string entry); 
        void logInfo(std::string entry);
        void logSysError(std::string entry);
        // we've got no reflection, so use this to initialize logging
        void activate(const char* filename); 

        Log () { }
        ~Log (); 
    private:
        int _pid; 
        void write(std::string msg, std::string level); 
    };

}

#endif // Logger_Hpp
