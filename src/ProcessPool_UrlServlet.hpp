#ifndef ProcessPool_UrlServlet_Hpp
#define ProcessPool_UrlServlet_Hpp

#include <vector>
#include <string> 
#include <fstream> 
#include <sys/select.h> 
#include "Multiprocess_UrlServlet.hpp"
#include "DataBucket.hpp"
#include "Core/ProcessPool.hpp" 
#include "Core/Logger.hpp" 

class UrlServletPool {
public: 
    UrlServletPool(int n);
    // every time insert process is called, we do special stuff
    void posixSelect();
    void start();
    void stop(); 
   
    std::vector<UrlServletProcess*>::iterator poolbegin() const; 
    std::vector<UrlServletProcess*>::iterator poolend() const; 
private: 
    ProcessPool<UrlServletProcess*>* _processPool; 
    DataBucket* _db;  // databucket hash table

    int _maxfd; // file descriptor for select
    int _n; // total number of child processes

    fd_set _rset;  // fd_set for select (fd with data)

    // tlog::Log _log;
    // initial seeds for processes...seeds can't have whitespace, so we replace em with percent encoding
    const std::vector<std::string> seeds = {
        "Gym%20Ldr.%20Pulse", "kdarewolf", "Onox", "hard", 
        "Aimvad", "vitico", "wtfmangg", "Nazara", "Guchi",
        "Kevinskie", "lapp94", "Enmx", "darkhuy" 
    }; 
};

#endif

