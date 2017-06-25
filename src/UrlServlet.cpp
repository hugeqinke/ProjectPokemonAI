#include "UrlServlet.hpp"

bool urlservlet::running = true; 

std::queue<std::string> _wq;

void urlservlet::halt(int signo) {
    urlservlet::running = false; 

    exit(0); 
}

void * urlLoader(void * arg) {
    struct params* p = (struct params*) arg; 
    int n;
    char buff[1024]; 
    int len = 1024; 
    while(urlservlet::running) {
        if ((n = read(p->fd, buff, len)) < 0) {
            std::cout << "Wop wop error on thread" << std::endl;
        }
        // std::cout << buff << std::endl;
        std::string buffString(buff, strlen(buff));

        _wq.push(buffString); 
    }   

    return (void*) 0;  
}

UrlServlet::UrlServlet(const char* sockname, int fdes, std::string seed) {
    signal(SIGTERM, urlservlet::halt); 
    // _log.activate("UrlServlet"); 

    _fdes = fdes; 

    // for testing purposes, temporarily populate some fake shit into the queue
    _wq.push(seed);

    // open up a couple of named unix domains sockets
    struct sockaddr_un un; 
    int size;
    int threadid = 0; 
    socklen_t rsize; 
 
    // add some string length checking code
    un.sun_len = sizeof(un); 
    un.sun_family = AF_UNIX; 

    strcpy(un.sun_path, sockname); 
    if ( (_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        // _log.logSysError("Could not create socket"); 
        exit(-1); 
    }    

    // force unlink, we only intend to have this open once per socket id
    // if someone else is bound to this socket file, forcibly evict this mofo
    unlink(sockname); 

    size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path); 
    if (bind(_fd, (struct sockaddr*)&un, size) < 0) {
        // _log.logSysError("Could not bind socket"); 
        exit(-1); 
    }

    // _log.logInfo("Unix socket has been successfully bound to " + std::string(sockname)); 

    if(listen(_fd, 5) < 0) {
        // _log.logSysError("Could not listen on socket"); 
        perror("Could not listen on the socket"); 
        exit(-1); 
    }
    
    struct params* p = new struct params();
    p->fd = fdes;  
    // populate the queue asynchronously
    if(pthread_create(&_tid, NULL, urlLoader, p) < 0) {
        // _log.logError("Could not initialize background task");
    }
}
