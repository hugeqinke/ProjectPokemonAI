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
namespace battlecrawler {
    extern bool running;

    inline void halt(int signo); 
}

// use common params with UrlServlet or something
struct params_battle {
    int fd;     
};

extern std::queue<std::string> _wqBattle;

inline void * downloadWorker(void * arg); 

// handles receiving requests from Crawlers
// doesn't even need to communicate with main server...we'll have at most 2 duplicates. 
// in fact, i don't think we'll even have duplicates since it'll be handled the user duplicate checker
class BattleCrawler {
public: 
    BattleCrawler(const char* sockname); 
    void start(); 
private: 
    int _fd; 
    pthread_t _dwtid; 
    std::vector<std::string> traversed;
};

#endif // Url_Servlet_Hpp_2
