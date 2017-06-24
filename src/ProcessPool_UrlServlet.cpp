#include "ProcessPool_UrlServlet.hpp" 

UrlServletPool::UrlServletPool(int n) {
    _processPool = new ProcessPool<UrlServletProcess*>(); 
    _db = new DataBucket(); 
    
    _maxfd = -1;
    _n = n; 
     
    for (int i = 0; i < n; i++) {
        std::string seed = seeds.at(i); 
        _db->insert(seed); 
       
        UrlServletTask* servletTask = new UrlServletTask(); 
        UrlServletProcess* servletProcess = new UrlServletProcess(servletTask, seed);  
        servletProcess->start(); 

        int fd = servletProcess->getActiveFd(); 
        if (fd > _maxfd) {
            _maxfd = fd; 
        }         
        _processPool->insertProcess(servletProcess); 
    }
}

// TODO: Change urlservlet implementation to allow for this
// (i.e. intialize fd in constructor instead of start call)
void UrlServletPool::start() {
    for (auto it = poolbegin(); it != poolend(); it++) {
        (*it)->start(); 
    }    
} 

void UrlServletPool::stop() {
    for (auto it = poolbegin(); it != poolend(); it++) {
        (*it)->stop(); 
    }
} 

void UrlServletPool::posixSelect() {
    std::vector<std::string> res; // store result from crawlers here

    FD_ZERO(&_rset);
    for (auto it = poolbegin(); it != poolend(); it++) {
        int fd = (*it)->getActiveFd();
        FD_SET(fd, &_rset); 
    }
    
    if (select(_maxfd + 1, &_rset, NULL, NULL, NULL) < 0) {
        std::cout << "Could not select shizzles" << std::endl;
    }

    for (auto it = poolbegin(); it != poolend(); it++) {
        if (FD_ISSET((*it)->getActiveFd(), &_rset)) {
            char buff[1024]; 
            int charbuff = 1024; 
            if(read((*it)->getActiveFd(), buff, charbuff) < 0) {
                std::cout << "Failed to read" << std::endl;
            }
            else {
                std::string buffstr (buff, strlen(buff)); 
                res.push_back(buffstr); 
            } 
        }  
    } 

    for (auto it = res.begin(); it != res.end(); it++) {
        // for this parcel, determine where we should put this to work
        // TODO: persistent storage
        if (_db->insert(*it)) {
            // hash this motherfuckering string and find out which child process to toss back to      
            // note: we start at 1 because the string here will be quoted (unicode characters)
            if (it->length() == 0) continue;
 
            char firstChar = it->at(0); 
            int hash = firstChar % _n;
    
            // create new urls here and write to child
            std::string urln = *it; 
            // std::cout << "Writing " << urln << std::endl;    
            if (write(_processPool->_pool.at(hash)->getActiveFd(), urln.c_str(), 1024) < 0) {
                tlog::Log::Instance().logSysError("Could not write a url back to child"); 
                continue; 
            }
        } 

        // if (_db->bucket.size() == 20) {
        //     std::cout << "done" << std::endl;
        //     exit(1); 
        // }
         
    }

}

std::vector<UrlServletProcess*>::iterator UrlServletPool::poolbegin() const {
    return _processPool->_pool.begin(); 
}

std::vector<UrlServletProcess*>::iterator UrlServletPool::poolend() const {
    return _processPool->_pool.end(); 
}

