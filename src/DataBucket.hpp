#include <vector> 
#include <unordered_set> 

// NOT thread safe at the moment
// TODO: implement it so that it'll be better for concurrency later on by having actual buckets
class DataBucket {
public: 
    // std::vector<std::unordered_set<std::string>> buckets; 
    std::unordered_set<std::string> bucket; 

    bool insert(std::string value) {
        // generate the urls here
        if (bucket.find(value) == bucket.end()) {
            bucket.insert(value);
            return true;      
        }
        return false; 
    }
};
