#ifndef Multiprocess_Battlecrawler_hpp
#define Multiprocess_Battlecrawler_hpp

#include <sys/un.h> 
#include <sys/socket.h> 
#include "UrlServlet2.hpp" 
#include "Core/Multiprocess.hpp"
 
class BattleCrawlerTask : public Task {
public: 
    virtual void execute(); 
    virtual void start(); 
    BattleCrawlerTask(); 
};

class BattleCrawlerProcess : public Process {
public: 
    BattleCrawlerProcess(BattleCrawlerTask* task); 
    virtual void start(); 
    virtual void stop(); 
    std::string getSocket(); 
private: 
    std::string _socketname; 
}; 


#endif // Multiprocess_Battlecrawler_hpp
