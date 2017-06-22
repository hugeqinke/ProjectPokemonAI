#ifndef ProcessPool_PythonCrawler_Hpp
#define ProcessPool_PythonCrawler_Hpp

#include "ProcessPool_BattleServlet.hpp" 
#include "ProcessPool_UrlServlet.hpp" 

#include <string> 
#include <queue> 

class PythonCrawlerPool {
public: 
    PythonCrawlerPool(int n, BattleServletPool* battleServletPool, UrlServletPool* urlServletPool); 
};

#endif 
