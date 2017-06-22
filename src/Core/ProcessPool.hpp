#ifndef ProcessPool_hpp
#define ProcessPool_hpp

#include <vector> 
#include <functional> 
#include "Multiprocess.hpp" 

template <class T> 
class ProcessPool {
public: 
    ProcessPool() { }
    void insertProcess(T process) { _pool.push_back(process); }
    std::vector<T> _pool;     
};

#endif
