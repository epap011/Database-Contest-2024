#ifndef CMS_H
#define CMS_H

#include <vector>
#include <functional> // For std::hash
#include <string>     // For combining keys
#include <climits>    // For INT_MAX

class CountMinSketch {
private:
    int width;                  // Number of columns in the sketch
    int depth;                  // Number of hash functions (rows)
    std::vector<std::vector<int>> table; // 2D table to store counts
    std::vector<std::hash<std::string>> hashFunctions; // Hash functions

public:
    // Constructor: Initialize sketch with given width and depth
    CountMinSketch(int width, int depth);

    // Insert an item (key)
    void insert(int key);

    // Query approximate count of a key
    int query(int key) const;
};

#endif // CMS_H