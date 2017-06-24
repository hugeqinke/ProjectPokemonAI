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
}

class UrlServlet {
public: 
    UrlServlet (const char* sockname, int fdes, std::string seed) {
        signal(SIGTERM, urlservlet::halt); 
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
            tlog::Log::Instance().logSysError("Could not create socket"); 
            exit(-1); 
        }    

        // force unlink, we only intend to have this open once per socket id
        // if someone else is bound to this socket file, forcibly evict this mofo
        unlink(sockname); 

        size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path); 
        if (bind(_fd, (struct sockaddr*)&un, size) < 0) {
            tlog::Log::Instance().logSysError("Could not bind socket"); 
            exit(-1); 
        }

        std::cout << "Unix socket has been successfully bound to " << sockname << std::endl; 

        if(listen(_fd, 5) < 0) {
            tlog::Log::Instance().logSysError("Could not listen on socket"); 
            exit(-1); 
        }
        
        std::cout << "keep on rolling" << std::endl;

        struct params* p = new struct params();
        p->fd = fdes;  
        // populate the queue asynchronously
        if(pthread_create(&_tid, NULL, urlLoader, p) < 0) {
            std::cout << "Could not initialize background task" << std::endl;
        }
    }


    // duplexed pipe back to the parent for processing
    void start() {
        int sock; 

        while (urlservlet::running) {
            sockaddr_un incoming; 
            socklen_t size; 
            if ((sock = accept(_fd, (struct sockaddr*)&incoming, &size)) < 0) {
                tlog::Log::Instance().logSysError("Could not accept incoming request"); 
            }
      
            // give some work to the client 
            if (serveUrl(sock) < 0) {
                continue; 
            }
                
            // wait on the work from the client and send to main thread for processing 'em 
            int urlsLen;
            std::string urlsMsg; 
            if ((urlsLen = receiveUrls(sock, urlsMsg)) < 0) {
                continue; 
            }

            parseUrls(urlsMsg); 
            // create threads here?
            notifyParent(urlsMsg);
  
            close(sock);
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

    void notifyParent(std::string urlMsg) {
        // chuck the message to fixed buffer size of 1024
        // warning: make sure we consider any usernames with \n in it.. check what happens
        std::stringstream ss(urlMsg); 
        std::string line;
        std::vector<std::string> chunks; 
        while (std::getline(ss, line, '\n')) {
            if (line.length() > 1024) { // this shouldn't happen, as all names are < 19 chars
                std::cout << "This name is greater than 1024 bytes" << std::endl;
                continue; 
            }

            chunks.push_back(line); 
        }

        for(int i = 0; i < chunks.size(); i++) {
            const char* chunk = chunks.at(i).c_str(); 
            // std::cout << "Writing " << chunks.at(i) << std::endl;
            if (write(_fdes, chunk, 1024) < 0) {
                tlog::Log::Instance().logSysError("Child could not pass information back to parent"); 
                std::cout << "Dropped packets: " << chunk << std::endl;
                continue; 
            }
        }
    } 

    // TODO: remove, do this in the parent
    void parseUrls(std::string msg) {
        std::vector<std::string> names; 

        std::istringstream iss (msg); 
        std::string line; 

        while (std::getline(iss, line, '\n')) {
            names.push_back(line); 
        }    
    }

    // When a client requests the url, this method will try to serve one (if queue is not empty)
    // While queue is empty, we inefficiently poll until we arrive at some result
    int serveUrl(int sock) {
        int msgSize; 
        char msgBuff[4]; 
        const char* urlMsg;

        if ((msgSize = recv(sock, msgBuff, 4, 0)) < 0) {
            tlog::Log::Instance().logSysError("Failed to receive incoming message");
            return -1;  
        }            

        std::cout << "Received signal: " << msgBuff << std::endl;

        // keep polling on this mofo
        while (_wq.empty()) {
            continue; 
        }
 
        urlMsg = _wq.front().c_str(); 
        int urlMsgLen = strlen(urlMsg); 
        if (send(sock, urlMsg, urlMsgLen, 0) < 0) {
            tlog::Log::Instance().logSysError("Failed to send back message");
            return -1; 
        }

        return 1;
    }     

    // for now, processUrls should be the very last thing that happens in this chain
    // to stop message receiving, we simply close the connectoin on the client end
    // if we need to expand this method here, figure out an end transmission code (\r\r\n maybe)
    int receiveUrls(int sock, std::string& buff) {
        int msgSize; 
        char msgBuff[1024]; 
        std::stringstream ss; 
   
        bool receiving = true; 
         
        do {
            if ((msgSize = recv(sock, msgBuff, 1024, 0)) <= 0) {
                receiving = false; 
            
                if (msgSize < 0) {
                    tlog::Log::Instance().logError("processUrls - Failed to receive incoming urls");
                    return -1; 
                }
            } 
            else {
                // std::cout << strlen(msgBuff) << std::endl; 
                ss << msgBuff;
            }
        } 
        while (receiving); 

        // TODO
        _wq.pop(); // We only pop from the queue if we've successfully received a list of new values
                   // We'll have to keep the value here in case we fail anywhere in the parent side
        buff = ss.str(); 
 
        return buff.length(); 
    } 
}; 

#endif // Url_Servlet_Hpp
