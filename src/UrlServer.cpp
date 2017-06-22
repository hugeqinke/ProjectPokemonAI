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
#include "ProcessPool_UrlServlet.hpp"
#include "DataBucket.hpp"
#include "Core/Logger.hpp"
#include "Core/ProcessPool.hpp" 

// Children =.=
#define CHILDREN 1

// TODO: move everything to a separate "Socket" class
// Create a bunch of sockets
int main() {
    auto start = Clock::now(); 

    // Clear directory for testing purposes
    std::system("exec rm -f ./datalogs/*");

    UrlServletPool* processPool = new UrlServletPool(1); 
    // check for messages from the process 
    while (true) { 
        processPool->posixSelect(); 
    //     for (auto it = receivedData.begin(); it != receivedData.end(); it++) {
    //         // for this parcel, determine where we should put this to work
    //         // TODO: persistent storage
    //         if (db->insert(*it)) {
    //             // hash this motherfuckering string and find out which child process to toss back to      
    //             // note: we start at 1 because the string here will be quoted (unicode characters)
    //             char firstChar = it->at(1); 
    //             int hash = firstChar % CHILDREN;
    //     
    //             // create new urls here and write to child
    //             std::string urln = *it; 
    // 
    //             if (write(fds[hash], urln.c_str(), 1024) < 0) {
    //                 tlog::Log::Instance().logSysError("Could not write a url back to child"); 
    //                 continue; 
    //             }
    //         }  
    //     }

        // std::cout << db->bucket.size() << std::endl;
        // if (db->bucket.size() >= 2500) {
        //     auto finish = Clock::now(); 

        //     std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (finish- start).count() << std::endl;
        //     std::ofstream fstr;
        //     fstr.open("logged.txt"); 
        //     for (auto it = db->bucket.begin(); it != db->bucket.end(); it++) {
        //         fstr << *it << std::endl;                     
        //     }

        //     fstr.close(); 
        //     exit(1); 
        // }
    }
    return 0;
 }

