#include "Multiprocess_UrlServlet.hpp" 

void UrlServletTask::execute() {

}

void UrlServletTask::stop() {

} 

UrlServletTask::UrlServletTask() {

} 

UrlServletProcess::UrlServletProcess(UrlServletTask* servletTask, std::string seed)
    : Process(servletTask), _seed(seed)
{ 
    
}

void UrlServletProcess::start() {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, _fd)) {
        perror("Could not create socketpair"); 
        exit(-1); 
    }

    _child = fork(); 
    _pid = getpid(); 

    if (_child < 0) {
        perror("Could not start the webcrawler process"); 
    } 

    if (_child == 0) {
        close(_fd[0]); 
        
        _activefd = _fd[1]; 
        std::string socketname = "sockets/test.socket" + std::to_string(_pid);
        
        UrlServlet* servlet = new UrlServlet(socketname.c_str(), _activefd, _seed);
        servlet->start();  

        exit(0); 
    }
    else {
        close(_fd[1]);
        _activefd = _fd[0]; 
    }
}

void UrlServletProcess::stop() {
    kill(_child, SIGTERM); 
}

int UrlServletProcess::getActiveFd() {
    return _activefd; 
}

// need a getsocket call here because socket's create in child process
std::string UrlServletProcess::getSocket() {
    return "sockets/test.socket" + std::to_string(_child); 
}
