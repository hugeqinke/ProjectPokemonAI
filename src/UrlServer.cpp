#include<iostream>
#include<sstream>
#include<fstream>
#include<string>
#include<poll.h>
#include<unistd.h> 
#include<sys/msg.h>
#include<sys/un.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<pthread.h>
#include<stdio.h>
#include<unordered_map>
#include<unordered_set> 
#include<vector> 
#include<queue> 
#include<algorithm> 
#include<ctime> 
#include<chrono>
typedef std::chrono::high_resolution_clock Clock;

#include "UrlServlet.hpp"
#include "UrlServlet2.hpp"
#include "UrlAllocator.hpp"
#include "Multiprocess_UrlServlet.hpp"
#include "Multiprocess_BattleCrawler.hpp"
#include "ProcessPool_UrlServlet.hpp"
#include "ProcessPool_BattleServlet.hpp" 
#include "ProcessPool_PythonCrawler.hpp"
#include "DataBucket.hpp"
#include "Core/Logger.hpp"
#include "Core/ProcessPool.hpp" 

// Children =.=
#define CHILDREN 1

namespace mainkiller {
    bool running = true;

    void killbill(int signo) {
        running = false; 
    }
}

// TODO: move everything to a separate "Socket" class
// Create a bunch of sockets
int main() {
    // tlog::Log log; 
    // log.activate("UrlServer"); 

    // log.logInfo("main - Starting the web crawler"); 
    // log.logInfo("main - Clearing file systems"); 
    // Clear directory for testing purposes
    std::system("exec rm -f ./datalogs/*");
    std::system("exec rm -f ./sockets/*"); 
   
    // log.logInfo("main - Initializing user crawler process pool");  
    UrlServletPool* processPool = new UrlServletPool(1); 
    // log.logInfo("main - Initializing battle crawler process pool"); 
    BattleServletPool* battleServletPool = new BattleServletPool(1); 
    // log.logInfo("main - Initializing python crawler pool"); 
    PythonCrawlerPool* processCrawlerPool = new PythonCrawlerPool(1, battleServletPool, processPool);
 
    signal(SIGTERM, mainkiller::killbill); 
    
    pid_t mainpid = getpid(); 
    std::cout << "MAIN PID: " << mainpid << std::endl;
    // log.logInfo("main - mainpid: " + std::to_string(mainpid)); 

    // check for messages from the process 
    while (mainkiller::running) { 
        processPool->posixSelect();
    }

    // log.logInfo("main - killing all child processes now..."); 
    
    processPool->stop(); 
    battleServletPool->stop(); 

    // log.logInfo("main - Done!"); 
    return 0;
 }

