#include "Multiprocess_Battlecrawler.hpp"

void BattleCrawlerTask::execute() {

}

void BattleCrawlerTask::start() {

}

BattleCrawlerTask::BattleCrawlerTask() {

}

BattleCrawlerProcess::BattleCrawlerProcess(BattleCrawlerTask* task) 
    : Process(task)
{

}

void BattleCrawlerProcess::start() {
    _child = fork(); 
    _pid = getpid(); 

    if (_child < 0) {
        perror("Could not start battle crawler process"); 
    } 

    if (_child == 0) {
        std::string socketname = "sockets/battle.socket" + std::to_string(_pid); 
        BattleCrawler* bc = new BattleCrawler(socketname.c_str());  
        bc->start(); 
        
        exit(0); 
    } 
}

void BattleCrawlerProcess::stop() {
    kill(_child, SIGTERM); 
}

// WARNING: THIS CAN ONLY BE CALLED FROM PARENT, NEED SEPARATE DISTINGUISHING 
// THING FROM CHILD
std::string BattleCrawlerProcess::getSocket() {
    return "sockets/battle.socket" + std::to_string(_child); 
}
