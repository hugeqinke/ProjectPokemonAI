#ifndef Multiprocess_UrlServlet 
#define Multiprocess_UrlServlet

#include <sys/un.h>
#include <sys/socket.h>

#include "Core/Multiprocess.hpp"
#include "UrlServlet.hpp"

class UrlServletTask : public Task {
public:
    virtual void execute(); 
    virtual void stop();
    UrlServletTask();   
};  

class UrlServletProcess : public Process {
public:
    UrlServletProcess(UrlServletTask* servletTask, std::string seed); 
    virtual void start(); 
    virtual void stop();
 
    int getActiveFd(); 
    std::string getSocket(); 
private: 
    // define some custom process logic here, stuff like ipc, etc
    int _fd[2]; 
    int _activefd; 
    tlog::Log log;
    std::string _seed; 
}; 

#endif 
