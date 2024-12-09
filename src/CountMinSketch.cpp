#include <CountMinSketch.h>

// Constructor
CountMinSketch::CountMinSketch(int width, int depth)
    : width(width), depth(depth), table(depth, std::vector<int>(width, 0)) {
    // Initialize hash functions (std::hash<string> acts as hash functions)
    for (int i = 0; i < depth; ++i) {
        hashFunctions.emplace_back(std::hash<std::string>());
    }
}

// Insert an item into the sketch
void CountMinSketch::insert(int key) {
    std::string keyStr = std::to_string(key);
    for (int i = 0; i < depth; ++i) {
        size_t hashVal = hashFunctions[i](keyStr);
        table[i][hashVal % width] += 1;
    }
}

// Query the approximate count of a key
int CountMinSketch::query(int key) const {
    std::string keyStr = std::to_string(key);
    int minCount = INT_MAX; // Start with a large value
    for (int i = 0; i < depth; ++i) {
        size_t hashVal = hashFunctions[i](keyStr);
        minCount = std::min(minCount, table[i][hashVal % width]);
    }
    return minCount;
}
