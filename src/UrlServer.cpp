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
#include "DataBucket.hpp"
#include "Core/Logger.hpp"

// Children =.=
#define CHILDREN 1

// TODO: move everything to a separate "Socket" class
// Create a bunch of sockets
int main() {
    tlog::Log::Instance().activate("UrlServer"); 
    
    auto start = Clock::now(); 

    // Clear directory for testing purposes
    std::system("exec rm -f ./datalogs/*");

    // inital setup
    // no seeds with white spaces...if there are any special characters, 
    // replace with url percent encoding
    std::vector<std::string> initialUsers = {
        "Gym%20Ldr.%20Pulse",
        "kdarewolf",
        "Onox",
        "hard", 
        "Aimvad",
        "vitico",
        "wtfmangg", 
        "Nazara",
        "Guchi",
        "Kevinskie",
        "lapp94",
        "Enmx",
        "darkhuy"
    }; 

    fd_set rset; 
    int children[CHILDREN]; 
    int childrenBattle[CHILDREN]; 
    int fds[CHILDREN];  
    int maxfd = -1; 

    DataBucket* db = new DataBucket();     
    
    for (int i = 0; i < CHILDREN; i++) {
        std::string seed = initialUsers.at(i);   
        db->insert(seed); 

        int fd[2]; 
        int child;

        std::string socketname = "test.socket";
        std::string socketnameBattle = "battle.socket";
        std::string pythonCall = "python UserCrawler.py ";

        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
            tlog::Log::Instance().logSysError("Could not create socketpair in main"); 
            exit(-1); 
        } 

        if ((children[i] = fork()) < 0) {
            tlog::Log::Instance().logSysError("Could not fork child servlet"); 
            exit(-1); 
        }

        if (children[i]) { // this is the parent
            close(fd[1]); 
            fds[i] = fd[0];
            maxfd = std::max(maxfd, fd[0]); 
        } 
        else {  // This is the child
            close(fd[0]);
            std::string socketName = "test.socket" + std::to_string(i); 
            UrlServlet* servlet = new UrlServlet(socketName.c_str(), fd[1], seed);
            servlet->start();  
            exit(2); 
        }

        if ((childrenBattle[i] = fork()) < 0) {
            tlog::Log::Instance().logSysError("Could not battle fork child");
        }

        if (!childrenBattle[i]) {
            std::string socketName = "battle.socket" + std::to_string(i); 
            BattleCrawler* servlet = new BattleCrawler(socketName.c_str()); 
            servlet->start(); 
            exit(2);     
        }

        // commands to execute python crawlers
        std::string ssocketname = socketname + std::to_string(i); 
        std::string ssocketnameBattle = socketnameBattle + std::to_string(i); 
        std::string command = pythonCall + " " + ssocketname + " " + ssocketnameBattle + " &";
        std::system(command.c_str());  
    }

    int result; 
   
    char buff[1024];
    int charbuff = 1024; 

    while (true) {
        FD_ZERO(&rset); 
        for(int i = 0; i < CHILDREN; i++) {
            FD_SET(fds[i], &rset); 
        }
 
        if((result = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            tlog::Log::Instance().logSysError("Error is in the logs when trying to select");
            continue; 
        }

        std::vector<std::string> receivedData; 
        for (int i = 0; i < CHILDREN; i++) {
            if (FD_ISSET(fds[i], &rset)) {
                if (read(fds[i], buff, charbuff) < 0) {
                    std::cout << "Fail" << std::endl;
                    tlog::Log::Instance().logSysError("Could not read data from crawler");
                }
                else {
                    std::string buffData (buff, strlen(buff));  
                    // std::cout << "Received: " << buffData << std::endl;
                    // now use distribution algorithm here to disperse to whereever (separate thread)
                    // now send this over to parse as a url :D 
                    // concurrent queue here? 
                    receivedData.push_back(buffData); 
                }
            } 
        }
        // std::cout << receivedData.size() << std::endl;
        // no thread for now, just let it build up in file queue (should consider reducing max buffer size)
        for (auto it = receivedData.begin(); it != receivedData.end(); it++) {
            // for this parcel, determine where we should put this to work
            // TODO: persistent storage
            if (db->insert(*it)) {
                // hash this motherfuckering string and find out which child process to toss back to      
                // note: we start at 1 because the string here will be quoted (unicode characters)
                char firstChar = it->at(1); 
                int hash = firstChar % CHILDREN;
        
                // create new urls here and write to child
                std::string urln = *it; 

                if (write(fds[hash], urln.c_str(), 1024) < 0) {
                    tlog::Log::Instance().logSysError("Could not write a url back to child"); 
                    continue; 
                }
            }  
        }

        // std::cout << db->bucket.size() << std::endl;
        if (db->bucket.size() >= 2500) {
            auto finish = Clock::now(); 

            std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (finish- start).count() << std::endl;
            std::ofstream fstr;
            fstr.open("logged.txt"); 
            for (auto it = db->bucket.begin(); it != db->bucket.end(); it++) {
                fstr << *it << std::endl;                     
            }

            fstr.close(); 
            exit(1); 
        }
    }
    return 0;
 }