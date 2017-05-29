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

// TODO: need to make some abstract schenanigans here...some crawler interface or whatnot

// use common params with UrlServlet or something
struct params_battle {
    int fd;     
};

std::queue<std::string> _wqBattle;

void* battleUrlLoader(void* args) {
    // lol gotta accept the connection first 
    struct params_battle* p = (struct params_battle*) args;    
    bool receiving = true; 
    std::stringstream ss; 
   
    while (true) {
        int sock;
        socklen_t size;
        struct sockaddr_un incoming; 
   
        if ((sock = accept(p->fd, (struct sockaddr*)&incoming, &size)) < 0) {
            perror("Could not accept incoming connection");   
        } 

        do {
            char buff[1024]; 
            int buffLen = 1024; 
            int ret; 
            if ((ret = recv(sock, buff, buffLen, 0)) <= 0) {
                receiving = false;
                std::cout << "Terminated" << std::endl;
                if (ret < 0) {
                    perror ("Could not read battle url from the parent");
                    break;
                }
            }
            ss << buff;  

            std::cout << "Received: " << buff << std::endl;
        } while(receiving); 
       
        std::string gameName; 
        while (getline(ss, gameName, '\n')) {
            _wqBattle.push(gameName); 
        }
    }
}

// handles receiving requests from Crawlers
// doesn't even need to communicate with main server...we'll have at most 2 duplicates. 
// in fact, i don't think we'll even have duplicates since it'll be handled the user duplicate checker
class BattleCrawler {
public: 
    BattleCrawler(const char* sockname) {
        // listen on the socket for the shit coming out from the UserCrawler 
        struct sockaddr_un un; 
        
        un.sun_family = AF_UNIX; 
        un.sun_len = sizeof(un); 

        strcpy(un.sun_path, sockname);    
        int size = offsetof(struct sockaddr_un, sun_path) + strlen(sockname);
       
        if ( (_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
            perror("Could not listen on sock stream");
            exit(-1);  
        }
        // use the information take from the crawler to wget some webpages
        unlink(sockname); 
 
        if(bind(_fd, (struct sockaddr*)&un, size) < 0) {
            perror("Could not bind the the socket battle  crawler thing"); 
            exit(-1);
        }

        std::cout << "Battle Crawler successfully binded " << sockname <<  std::endl; 

        if (listen(_fd, 5) < 0) {
            perror("Could not listen on battle crawler socket");
            exit(-1);   
        } 

        // handle reading on a separate thread
        params_battle pb; 
        pb.fd = _fd; 
   
        if (pthread_create(&_tid, NULL, battleUrlLoader, (struct params_battle*)&pb) < 0) {
            perror("Could not create a thread"); 
            exit(-1); 
        }
    }

    void start() {
        while (true) {
            while(_wqBattle.empty()) continue;
            std::string url = _wqBattle.front();

            traversed.push_back(url); 
    
            std::cout << traversed.size() << std::endl; 
            _wqBattle.pop(); 
        }
   
        // while (true) {
        //     // get things from the work queue and wget that shit
        //     std::string url = _wq.front(); 
        //     std::string execution = "wget " + url;  
        //     std::system(execution); 
        //     _wqBattle.pop();  

        //     // and that's it!
        // }
        // clear contents of the directory first (or back up) 
        
        // std::string wg = "wget -O "; 
        // std::string dir = " ./datalogs/";
       
        // std::system("exec rm -f ./datalogs/*");
 
        // std::string execution = wg + dir + url + " http://replay.pokemonshowdown.com/" + url; 
        // std::system(execution.c_str());
    }

private: 
    int _fd; 
    pthread_t _tid; 
    std::vector<std::string> traversed;
};
