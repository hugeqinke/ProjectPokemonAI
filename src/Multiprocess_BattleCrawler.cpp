#include "Multiprocess_Battlecrawler.hpp"

void BattleCrawlerTask::execute() {

}

void BattleCrawlerTask::start() {

}

BattleCrawlerTask::BattleCrawlerTask() {

}

BattleCrawlerProcess::BattleCrawlerProcess(BattleCrawlerTask* task, std::string socketname) 
    : Process(task), _socketname(socketname) 
{

}

void BattleCrawlerProcess::start() {
    _child = fork(); 
    _pid = getpid(); 

    if (_child < 0) {
        perror("Could not start battle crawler process"); 
    } 

    if (_child == 0) {
        BattleCrawler* bc = new BattleCrawler(_socketname.c_str());  
        bc->start(); 

        exit(0); 
    } 
}

void BattleCrawlerProcess::stop() {
    
}

std::string BattleCrawlerProcess::getSocket() {
    return "battle.socket" + std::to_string(_child); 
}
