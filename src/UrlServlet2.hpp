#ifndef Url_Servlet_2_Hpp
#define Url_Servlet_2_Hpp 

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

// TODO: need to make some abstract schenanigans here...some crawler interface or whatnot

// use common params with UrlServlet or something
struct params_battle {
    int fd;     
};

std::queue<std::string> _wqBattle;

// handles receiving requests from Crawlers
// doesn't even need to communicate with main server...we'll have at most 2 duplicates. 
// in fact, i don't think we'll even have duplicates since it'll be handled the user duplicate checker
class BattleCrawler {
public: 
    BattleCrawler(const char* sockname, int id) {
        // listen on the socket for the shit coming out from the UserCrawler 
        _id = id; 
        struct sockaddr_un un; 
        
        un.sun_family = AF_UNIX; 
        un.sun_len = sizeof(un); 

        strcpy(un.sun_path, sockname);    
        int size = offsetof(struct sockaddr_un, sun_path) + strlen(sockname);
       
        if ( (_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
            tlog::Log::Instance().logSysError("Could not listen on sock stream");
            exit(-1);  
        }
        // use the information take from the crawler to wget some webpages
        unlink(sockname); 
 
        if(bind(_fd, (struct sockaddr*)&un, size) < 0) {
            tlog::Log::Instance().logSysError("Could not bind the the socket battle crawler thing");
            exit(-1);
        }

        std::cout << "Battle Crawler successfully binded " << sockname <<  std::endl; 

        if (listen(_fd, 5) < 0) {
            tlog::Log::Instance().logSysError("Could not listen on battle crawler socket");
            exit(-1);   
        } 
    }

    void start() {
        tlog::Log::Instance().logInfo("Starting battle crawler child"); 
        while (true) {
            int sock;
            socklen_t size;
            struct sockaddr_un incoming; 
      
            if ((sock = accept(_fd, (struct sockaddr*)&incoming, &size)) < 0) {
                tlog::Log::Instance().logSysError("Could not accept incoming connection");
                continue; 
            } 
            
            bool receiving = true;
            std::stringstream ss; 
            
            do {
                char buff[1024]; 
                int buffLen = 1024; 
                int ret; 
                if ((ret = recv(sock, buff, buffLen, 0)) <= 0) {
                    receiving = false;
                    if (ret < 0) {
                        tlog::Log::Instance().logSysError("Could not read battle url from the parent");
                        exit(-1); 
                        break;
                    }
                }
                ss << buff;  
            } while(receiving); 
      
            // TODO: wget on separate thread...request and forget 
            std::string gameName; 
            while (getline(ss, gameName, '\n')) {
                std::string wg = "wget -O "; 
                std::string dir = " ./datalogs/";
                // std::string wgetlog = " -o ./logs/wgetlog" + std::to_string(_id) + ".log"; 
         
                std::string execution = wg + dir + gameName + " http://replay.pokemonshowdown.com/" + gameName; 
                tlog::Log::Instance().logInfo("Executing - " + execution); 
                std::system(execution.c_str());
            }
        }    
    }

private: 
    int _fd, _id; 
    pthread_t _tid; 
    std::vector<std::string> traversed;
};

#endif // Url_Servlet_Hpp_2
