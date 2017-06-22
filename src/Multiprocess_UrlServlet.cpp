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
        std::string socketname = "test.socket" + std::to_string(_pid);
        // start up python crawlers
        std::string pythonCall = "python UserCrawler.py ";
        std::string command = pythonCall + " " + socketname + " &";       
        std::system(command.c_str()); 

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

}

int UrlServletProcess::getActiveFd() {
    return _activefd; 
}
