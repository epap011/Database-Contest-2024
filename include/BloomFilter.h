#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include <vector>
#include <string>

class BloomFilter {
private:
    size_t filter_size;                     // Size of the Bloom filter in bits
    int num_hash_functions;                 // Number of hash functions
    std::vector<bool> bit_array;            // Bit array for the filter

    // Combine two integers into a single string key
    std::string combineKeys(int a, int b) const;

public:
    // Constructor
    BloomFilter(size_t filter_size = 4 * 1024 * 1024 * 8, int num_hash_functions = 5);

    // Insert a pair (A, B) into the Bloom filter
    void insert(int a, int b);

    // Check if (A, B) is in the Bloom filter
    bool query(int a, int b) const;
};

#endif // BLOOMFILTER_H
