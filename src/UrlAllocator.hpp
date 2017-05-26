#include <cstdlib> 
#include <cstdio> 
#include <iostream> 
#include <vector> 

// This doesn't serve any purpose now but might in the future

namespace alloc {
    struct batch{
        std::vector<std::string> urls; 
    };  

    void * allocate(void* arg) {
        struct batch* data = (struct batch*) arg; 

        for (auto it = data->urls.begin(); it != data->urls.end(); it++) {
            std::cout << *it << std::endl;
        }
    }
}
