#include "ProcessPool_BattleServlet.hpp" 

BattleServletPool::BattleServletPool(int n) {

}

void BattleServletPool::start() {

}

std::vector<BattleCrawlerProcess*>::iterator BattleServletPool::poolbegin() const {
    return _processPool->_pool.begin(); 
}

std::vector<BattleCrawlerProcess*>::iterator BattleServletPool::poolend() const {
    return _processPool->_pool.end(); 
}

