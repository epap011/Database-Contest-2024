#include <BloomFilter.h>
#include <functional> // For std::hash

// Constructor
BloomFilter::BloomFilter(size_t filter_size, int num_hash_functions)
    : filter_size(filter_size),
      num_hash_functions(num_hash_functions),
      bit_array(filter_size, false) {}

// Combine two integers into a single string key
std::string BloomFilter::combineKeys(int a, int b) const {
    return std::to_string(a) + ":" + std::to_string(b);
}

// Insert a pair (A, B) into the Bloom filter
void BloomFilter::insert(int a, int b) {
    std::string combined = combineKeys(a, b);
    for (int i = 0; i < num_hash_functions; ++i) {
        size_t hash_val = std::hash<std::string>{}(combined + std::to_string(i));
        bit_array[hash_val % filter_size] = true;
    }
}

// Check if (A, B) is in the Bloom filter
bool BloomFilter::query(int a, int b) const {
    std::string combined = combineKeys(a, b);
    for (int i = 0; i < num_hash_functions; ++i) {
        size_t hash_val = std::hash<std::string>{}(combined + std::to_string(i));
        if (!bit_array[hash_val % filter_size]) {
            return false; // If any bit is unset, the pair is not in the filter
        }
    }
    return true; // Pair might be in the filter
}