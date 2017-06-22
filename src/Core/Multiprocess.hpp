#ifndef Multiprocess_Hpp
#define Multiprocess_Hpp

#include<string> 
#include<iostream> 
#include<vector>
#include<unistd.h> 
#include<stdlib.h> 
#include<signal.h> 

class Task {
public: 
    Task() { } 
    virtual void execute() { }
    virtual void stop() { } 
};

// Prototyping object-oriented processes and threads
class Process {
public: 
    Process(Task* task) { }
    virtual void start() { }
    virtual void stop() { }

    Task* t; 
protected:
    pid_t _child; 
    pid_t _pid;  
}; 

 
#endif
