#ifndef Url_Servlet_Hpp
#define Url_Servlet_Hpp

// a servlet class that initalizes a server that parses requests, stuff like that
#include<iostream>
#include<sstream>
#include<string>
#include<poll.h>
#include<unistd.h> 
#include<pthread.h>
#include<sys/msg.h>
#include<sys/un.h>
#include<sys/socket.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unordered_map>
#include<unordered_set> 
#include<vector> 
#include<queue> 

#include "Core/Logger.hpp" 

namespace urlservlet {
    extern bool running; 

    inline void halt(int signo) {
        exit(0);     
    }
} 

extern std::queue<std::string> _wq;

struct params {
    int fd; 
}; 

inline void * urlLoader(void* arg) {
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

class UrlServlet {
public: 
    UrlServlet (const char* sockname, int fdes, std::string seed) {
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
            exit(-1); 
        }
        
        struct params* p = new struct params();
        p->fd = fdes;  
        // populate the queue asynchronously
        if(pthread_create(&_tid, NULL, urlLoader, p) < 0) {
            // _log.logError("Could not initialize background task");
        }
    }


    // duplexed pipe back to the parent for processing
    void start() {
        int sock; 

        while (urlservlet::running) {
            try {
                sockaddr_un incoming; 
                socklen_t size; 
                if ((sock = accept(_fd, (struct sockaddr*)&incoming, &size)) < 0) {
                    // _log.logSysError("Could not accept incoming request"); 
                }
          
                // give some work to the client 
                if (serveUrl(sock) < 0) {
                    // _log.logWarn("start - Failed to serve url back to python crawlers"); 
                    continue; 
                }
                    
                // wait on the work from the client and send to main thread for processing 'em 
                int urlsLen;
                std::string urlsMsg; 
                if ((urlsLen = receiveUrls(sock, urlsMsg)) < 0) {
                    // _log.logWarn("start - Failed to receive urls"); 
                    continue; 
                }

                parseUrls(urlsMsg); 
                // create threads here?
                notifyParent(urlsMsg);
      
                close(sock);
            }
            catch (const std::exception& ex) {
                std::cout << "Uncaught exception!!!" << std::endl; 
                // _log.logError ("start - uncaught exception: " + std::string(ex.what())); 
            }
        }
    }

    void stop() {
        urlservlet::running = false; 
    }
private:
    // Parameters: 
    //  1. The file descriptor for maintaining the socket connection with client
    //  2. The wq holds the urls that will be served to the client
    //        a. main thread will add contents into the wq after verifying that it's a unique url
    //  3. The fdes passes messages (this case individual user names) back to parent
    int _fd, _fdes; 
    pthread_t _tid; 

    // tlog::Log _log; // logging instance 

    void notifyParent(std::string urlMsg) {
        try { 
            // chuck the message to fixed buffer size of 1024
            // warning: make sure we consider any usernames with \n in it.. check what happens
            std::stringstream ss(urlMsg); 
            std::string line;
            std::vector<std::string> chunks; 
            while (std::getline(ss, line, '\n')) {
                if (line.length() > 1024) { // this shouldn't happen, as all names are < 19 chars
                    // _log.logWarn("notifyParent - " + line + " > 1024 chars");  
                    continue; 
                }

                chunks.push_back(line); 
            }

            for(int i = 0; i < chunks.size(); i++) {
                const char* chunk = chunks.at(i).c_str(); 
                // std::cout << "Writing " << chunks.at(i) << std::endl;
                if (write(_fdes, chunk, 1024) < 0) {
                    // _log.logSysError("Child could not pass information back to parent"); 
                    // _log.logSysError("Dropped packets: " + std::string(chunk)); 
                    continue; 
                }
            }
        } 
        catch (const std::exception& ex) {
            // _log.logSysError("notifyParent - " + std::string(ex.what())); 
        }
    } 

    // TODO: remove, do this in the parent
    void parseUrls(std::string msg) {
        try {
            std::vector<std::string> names; 

            std::istringstream iss (msg); 
            std::string line; 

            while (std::getline(iss, line, '\n')) {
                names.push_back(line); 
            }    
        } 
        catch (const std::exception& ex) {
            // _log.logInfo("parseUrls - " + std::string(ex.what())); 
        }
    }

    // When a client requests the url, this method will try to serve one (if queue is not empty)
    // While queue is empty, we inefficiently poll until we arrive at some result
    int serveUrl(int sock) {
        try { 
            int msgSize; 
            char msgBuff[4]; 
            const char* urlMsg;

            if ((msgSize = recv(sock, msgBuff, 4, 0)) < 0) {
                // _log.logSysError("Failed to receive incoming message");
                return -1;  
            }            

            // keep polling on this mofo
            // TODO: cond here
            while (_wq.empty()) {
                continue; 
            }

            std::string recentMsg = _wq.front();
            // _log.logInfo("serveUrl - serving: " + recentMsg); 
            urlMsg = recentMsg.c_str(); 
            int urlMsgLen = strlen(urlMsg); 
            if (send(sock, urlMsg, urlMsgLen, 0) < 0) {
                // _log.logSysError("Failed to send back message");
                return -1; 
            }
        }
        catch (const std::exception& ex) {
            // _log.logInfo("serveUrl - " + std::string(ex.what())); 
        }
        return 1;
    }     

    // for now, processUrls should be the very last thing that happens in this chain
    // to stop message receiving, we simply close the connectoin on the client end
    // if we need to expand this method here, figure out an end transmission code (\r\r\n maybe)
    int receiveUrls(int sock, std::string& buff) {
        try {
            // _log.logInfo("receiveUrls - receiving from python crawler"); 
            int msgSize; 
            char msgBuff[1024]; 
            std::stringstream ss; 
       
            bool receiving = true; 
             
            do {
                if ((msgSize = recv(sock, msgBuff, 1024, 0)) <= 0) {
                    receiving = false; 
                
                    if (msgSize < 0) {
                        std::cout << "Logging error!KLJDKLJF" << std::endl;
                        // _log.logError("receiveUrls - Failed to receive incoming urls");
                        return -1; 
                    }
                } 
                else {
                    ss << msgBuff;
                }
            } 
            while (receiving); 
            // We only pop from the queue if we've successfully received a list of new values
            _wq.pop(); 
            // We'll have to keep the value here in case we fail anywhere in the parent side
            buff = ss.str(); 
        }
        catch (const std::exception& ex) {
            std::cout << "Exception caught!" << std::endl;
            // _log.logError("receiveUrls - " + std::string(ex.what())); 
        }
        // _log.logInfo("receiveUrls - received from python crawler"); 
        return buff.length(); 
    } 
}; 

#endif // Url_Servlet_Hpp
