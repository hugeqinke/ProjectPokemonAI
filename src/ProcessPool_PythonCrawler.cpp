#include "ProcessPool_PythonCrawler.hpp"

PythonCrawlerPool::PythonCrawlerPool(int n, BattleServletPool* battleServletPool, UrlServletPool* urlServletPool) {
    std::queue<std::string> battlesockets; 
    std::queue<std::string> usersockets;  
    for (auto it = battleServletPool->poolbegin(); it < battleServletPool->poolend(); it++) {
         std::string socketname = (*it)->getSocket(); 
         battlesockets.push(socketname); 
    }

    for (auto it = urlServletPool->poolbegin(); it < urlServletPool->poolend(); it++) {
        std::string socketname = (*it)->getSocket();
        usersockets.push(socketname);  
    }

    if (battlesockets.size() != usersockets.size()) {
        std::cout << "battle sockets don't equal number of user sockets" << std::endl;
    }

    for (int i = 0; i < n; i++) {
        std::string battlesocket = battlesockets.front(); battlesockets.pop(); 
        std::string usersocket = usersockets.front(); usersockets.pop(); 

        std::string pythonCall = "python UserCrawler.py ";
        std::string command = pythonCall + " " +  usersocket + " " + battlesocket + " &";       
        // std::string command = pythonCall + " " +  usersocket + " &";       
        std::system(command.c_str()); 
    }
}
