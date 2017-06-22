#include "Logger.hpp" 

int tlog::active = 0; 
int tlog::flush = 1; 
bool tlog::terminateflag = false; 
bool tlog::loginitialized = false; 

pthread_mutex_t tlog::mqmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tlog::activemutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tlog::flushmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tlog::terminatemutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t tlog::consumer_tid = NULL; 
pthread_t tlog::swapper_tid = NULL; 

std::vector<std::queue<tlog::Parcel>> tlog::fq = {
        std::queue<tlog::Parcel>(),
        std::queue<tlog::Parcel>()
};

void * tlog::swap (void * arg) {
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


void * tlog::consumer (void * arg) {
    struct consumer_args* threadinfo = (struct consumer_args*) arg; 
   
    time_t timer; 
    char time_buffer[80];
    struct tm* tm_info; 

    time (&timer); 
    tm_info = localtime(&timer);

    strftime(time_buffer, 80, "%Y%m%dT%H%M%S", tm_info); 
      
    std::stringstream filename; 
    filename << threadinfo->processname
             << "." << threadinfo->pid << "." << time_buffer
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

void tlog::sigTerminate() {
    pthread_mutex_lock (&terminatemutex); 
    terminateflag = true; 
    pthread_mutex_unlock (&terminatemutex); 
}

bool tlog::isTerminated() {
    bool ret; 
    pthread_mutex_lock (&terminatemutex);  
    // this flag won't change after this flag is true, so its fine if we store this separately
    ret = terminateflag;
    pthread_mutex_unlock (&terminatemutex); 
    return ret;
}


void tlog::finalFlush(std::ofstream& ofs) {
    pthread_mutex_lock (&activemutex); 
    pthread_mutex_lock (&flushmutex);
    flush_stream(flush, ofs); 
    flush_stream(active, ofs); 
    ofs.close(); 
    pthread_mutex_unlock (&flushmutex); 
    pthread_mutex_unlock (&activemutex); 
} 

void tlog::flush_stream(int queue_id, std::ofstream& ofs) {
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

tlog::Log::~Log() {
    // activate the kill switch
    std::cout << "This shit has been called" << std::endl; 
    sigTerminate(); 

    pthread_join(tlog::swapper_tid, NULL); 
    pthread_join(tlog::consumer_tid, NULL); 
} 

void tlog::Log::activate(const char* filename) {
    struct consumer_args * args = new struct consumer_args(); 
    strcpy(args->processname, filename); 

    // if we want to call a child process, make sure we rename the threads 
    // and everything
    int pid = getpid();  
    if (_pid != pid) {
        _pid = pid;

        // if we're using 'activate' correctly, we don't need anything fancy 
        // queues won't be active at all, so we can just cancel em
        pthread_cancel(tlog::consumer_tid); 
        pthread_cancel(tlog::swapper_tid); 
        pthread_join(tlog::consumer_tid, NULL); 
        pthread_join(tlog::swapper_tid, NULL); 
    }

    args->pid = _pid; 
    pthread_create (&tlog::consumer_tid, 0, consumer, args); 
    pthread_create (&tlog::swapper_tid, 0, swap, 0);
}

void tlog::Log::write(std::string msg, std::string level) {
    time_t timer; 
    time(&timer); 
    pthread_mutex_lock(&activemutex); 
    fq.at(active).emplace(msg, level, timer); 
    pthread_mutex_unlock(&activemutex);
} 

void tlog::Log::logError(std::string entry) {
    write(entry, "Error"); 
}

void tlog::Log::logException(std::string entry) {
    write(entry, "Exception");
}

void tlog::Log::logWarn(std::string entry) {
    write(entry, "Warn");
}

void tlog::Log::logDebug(std::string entry) {
    write(entry, "Debug"); 
}

void tlog::Log::logInfo(std::string entry) { 
    write(entry, "Info");
}

void tlog::Log::logSysError(std::string entry) {
    int error_num; 
    char err_buff[200]; 
    
    error_num = strerror_r ( errno, err_buff, 200 );
    
    std::stringstream ss;  
    ss << entry << ": " << err_buff;
    write(ss.str(), "SysErr");  
}


tlog::Log& tlog::Log::Instance() {
    static Log instance; 
    return instance;     
}

