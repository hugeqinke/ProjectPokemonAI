#include "UrlServlet2.hpp"

std::queue<std::string> _wqBattle;

bool battlecrawler::running = true; 

void battlecrawler::halt(int signo) {
    std::cout << "BATTLE CRAWLER STOPPING!!!!" << std::endl;
    battlecrawler::running = false; 
} 

void * downloadWorker(void * arg) {
    // while (battlecrawler::running || !_wqBattle.empty()) {    
    while (battlecrawler::running) {    
        while(_wqBattle.empty()) continue;

        std::cout << _wqBattle.size() << std::endl;

        if (!battlecrawler::running) {
            std::cout << "Stopping..." << std::endl;
        }
        std::string gameName = _wqBattle.front(); _wqBattle.pop();
        std::string wg = "wget -O "; 
        std::string dir = "./datalogs/";
       
        std::string execution = ""; 
        execution += wg; 
        execution += dir; 
        execution += gameName; 
        execution += " \"http://replay.pokemonshowdown.com/"; 
        execution += gameName + "\"";
        execution += " 2>> wgetLogs/wget.log"; 

        std::system(execution.c_str());
    }

    return (void*) 1;  
}

BattleCrawler::BattleCrawler(const char* sockname) {
    signal(SIGTERM, battlecrawler::halt); 
    // _log.activate("BattleCrawler"); 

    // listen on the socket for the shit coming out from the UserCrawler 
    struct sockaddr_un un; 
    
    un.sun_family = AF_UNIX; 
    un.sun_len = sizeof(un); 

    strcpy(un.sun_path, sockname);    
    int size = offsetof(struct sockaddr_un, sun_path) + strlen(sockname);
   
    if ( (_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        // _log.logSysError("Could not listen on sock stream");
        exit(-1);  
    }
    // use the information take from the crawler to wget some webpages
    unlink(sockname); 

    if(bind(_fd, (struct sockaddr*)&un, size) < 0) {
        // _log.logSysError("Could not bind the the socket battle crawler thing");
        exit(-1);
    }

    // _log.logInfo("Battle Crawler successfully binded");  
    if (listen(_fd, 5) < 0) {
        // _log.logSysError("Could not listen on battle crawler socket");
        exit(-1);   
    } 

    // begin wget task
    pthread_create(&_dwtid, 0, downloadWorker, 0); 
}

void BattleCrawler::start() {
    // _log.logInfo("Starting battle crawler child"); 
    while (battlecrawler::running) {
        int sock;
        socklen_t size;
        struct sockaddr_un incoming; 
  
        if ((sock = accept(_fd, (struct sockaddr*)&incoming, &size)) < 0) {
            // _log.logSysError("start - Could not accept incoming connection");
            continue; 
        } 
        
        bool receiving = true;
        std::stringstream ss; 
        
        do {
            char buff[1024];
            memset(buff, '\0', 1024);  
            int buffLen = 1024; 
            int ret; 
            if ((ret = recv(sock, buff, buffLen, 0)) <= 0) {
                receiving = false;
                if (ret < 0) {
                    // _log.logSysError("Could not read battle url from the parent");
                    exit(-1); 
                    break;
                }
            }
            ss << buff;  
        } while(receiving); 
  
        // TODO: wget on separate thread...request and forget 
        std::string gameName; 
        while (getline(ss, gameName, '\n')) {
            // _log.logInfo("start - pushing game to queue: " + gameName); 
            _wqBattle.push(gameName);
        }
    }    
}
