#include "ProcessPool_BattleServlet.hpp" 

BattleServletPool::BattleServletPool(int n) {
    _processPool = new ProcessPool<BattleCrawlerProcess*>(); 
    _n = n; 

    for (int i = 0; i < n; i++) {
        BattleCrawlerTask* task = new BattleCrawlerTask(); 
        BattleCrawlerProcess* process = new BattleCrawlerProcess(task); 
        
        process->start();

        _processPool->insertProcess(process);  
    } 
}

void BattleServletPool::start() {

}

void BattleServletPool::stop() {
    for (auto it = poolbegin(); it != poolend(); it++) {
        (*it)->stop(); 
    }
}

std::vector<BattleCrawlerProcess*>::iterator BattleServletPool::poolbegin() const {
    return _processPool->_pool.begin(); 
}

std::vector<BattleCrawlerProcess*>::iterator BattleServletPool::poolend() const {
    return _processPool->_pool.end(); 
}

