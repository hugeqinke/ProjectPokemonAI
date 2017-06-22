#ifndef ProcessPool_BattleServlet_Hpp 
#define ProcessPool_BattleServlet_Hpp 

#include "Core/ProcessPool.hpp"
#include "Multiprocess_BattleCrawler.hpp" 

class BattleServletPool {
public: 
    BattleServletPool(int n); 
    void start(); 

    std::vector<BattleCrawlerProcess*>::iterator poolbegin() const; 
    std::vector<BattleCrawlerProcess*>::iterator poolend() const; 
private: 
    ProcessPool<BattleCrawlerProcess*>* _processPool;
}; 


#endif // ProcessPool_BattleServlet_Hpp 

