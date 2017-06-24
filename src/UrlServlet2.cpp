#include "UrlServlet2.hpp"

std::queue<std::string> _wqBattle;

bool battlecrawler::running = true; 

void battlecrawler::halt(int signo) {
    exit(0); 
} 

void * downloadWorker(void * arg) {
    while (battlecrawler::running) {    
        while(_wqBattle.empty()) continue;

        std::string gameName = _wqBattle.front(); _wqBattle.pop();
        std::cout << "THIS IS THE GAME NAME: " <<  gameName << std::endl;
        std::string wg = "wget -O "; 
        std::string dir = "./datalogs/";
       
        std::string execution = ""; 
        execution += wg; 
        execution += dir; 
        execution += gameName; 
        execution += " \"http://replay.pokemonshowdown.com/"; 
        execution += gameName + "\"";
        execution += " 2>> wgetLogs/wget.log"; 

        // tlog::Log::Instance().logInfo("Executing - " + execution); 
        std::cout << "Executing " << execution << std::endl;
        std::system(execution.c_str());
    }

    return (void*) 1;  
}

BattleCrawler::BattleCrawler(const char* sockname) {
    signal(SIGTERM, battlecrawler::halt); 
    // listen on the socket for the shit coming out from the UserCrawler 
    struct sockaddr_un un; 
    
    un.sun_family = AF_UNIX; 
    un.sun_len = sizeof(un); 

    strcpy(un.sun_path, sockname);    
    int size = offsetof(struct sockaddr_un, sun_path) + strlen(sockname);
   
    if ( (_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        tlog::Log::Instance().logSysError("Could not listen on sock stream");
        exit(-1);  
    }
    // use the information take from the crawler to wget some webpages
    unlink(sockname); 

    if(bind(_fd, (struct sockaddr*)&un, size) < 0) {
        tlog::Log::Instance().logSysError("Could not bind the the socket battle crawler thing");
        exit(-1);
    }

    std::cout << "Battle Crawler successfully binded " << sockname <<  std::endl; 

    if (listen(_fd, 5) < 0) {
        tlog::Log::Instance().logSysError("Could not listen on battle crawler socket");
        exit(-1);   
    } 

    // begin wget task
    pthread_create(&_dwtid, 0, downloadWorker, 0); 
}

void BattleCrawler::start() {
    // tlog::Log::Instance().logInfo("Starting battle crawler child"); 
    std::cout << "Starting battle crawler child" << std::endl;
    while (battlecrawler::running) {
        int sock;
        socklen_t size;
        struct sockaddr_un incoming; 
  
        if ((sock = accept(_fd, (struct sockaddr*)&incoming, &size)) < 0) {
            // tlog::Log::Instance().logSysError("Could not accept incoming connection");
            std::cout << "COuld not accept incoming connection" << std::endl;
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
                    tlog::Log::Instance().logSysError("Could not read battle url from the parent");
                    exit(-1); 
                    break;
                }
            }
            ss << buff;  
        } while(receiving); 
  
        // TODO: wget on separate thread...request and forget 
        std::string gameName; 
        while (getline(ss, gameName, '\n')) {
            _wqBattle.push(gameName);
        }
    }    
}
