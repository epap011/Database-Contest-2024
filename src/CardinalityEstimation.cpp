//
// You should modify this file.
//
#include <common/Root.h>
#include <CardinalityEstimation.h>

#include <vector>
#include <functional> // For std::hash
#include <string>     // For combining keys
#include <climits>    // For INT_MAX

class CountMinSketch {          // Example emory per CMS = 10,000(WIDTH)×32(DEPTH)×4(BYTES)=1,280,000bytes (1.28 MB per CMS).
private:
    int width;                  // Number of columns in the sketch
    int depth;                  // Number of hash functions (rows)
    std::vector<std::vector<int>> table; // 2D table to store counts
    std::vector<std::hash<std::string>> hashFunctions; // Hash functions

public:
    // Constructor
    CountMinSketch(int width, int depth)
        : width(width), depth(depth), table(depth, std::vector<int>(width, 0)) {
        // Initialize hash functions (std::hash<string> acts as hash functions)
        for (int i = 0; i < depth; ++i) {
            hashFunctions.emplace_back(std::hash<std::string>());
        }
    }

    // Insert an item into the sketch
    void insert(int key) {
        std::string keyStr = std::to_string(key);
        for (int i = 0; i < depth; ++i) {
            size_t hashVal = hashFunctions[i](keyStr);
            table[i][hashVal % width] += 1;
        }
    }

    // Query the approximate count of a key
    int query(int key) const {
        std::string keyStr = std::to_string(key);
        int minCount = INT_MAX; // Start with a large value
        for (int i = 0; i < depth; ++i) {
            size_t hashVal = hashFunctions[i](keyStr);
            minCount = std::min(minCount, table[i][hashVal % width]);
        }
        return minCount;
    }
};

class BloomFilter {                         // Example memory per Bloom filter = (1024*1024)(FILTER)*8(ARRAY BITS) = 1 MB
private:
    size_t filter_size;                     // Size of the Bloom filter in bits
    int num_hash_functions;                 // Number of hash functions
    std::vector<bool> bit_array;            // Bit array for the filter

    // Combine two integers into a single string key
    std::string combineKeys(int a, int b) const {
        return std::to_string(a) + ":" + std::to_string(b);
    }

public:
    // Constructor
    BloomFilter(size_t filter_size, int num_hash_functions)
        : filter_size(filter_size),
          num_hash_functions(num_hash_functions),
          bit_array(filter_size, false) {}

    // Insert a pair (A, B) into the Bloom filter
    void insert(int a, int b) {
        std::string combined = combineKeys(a, b);
        for (int i = 0; i < num_hash_functions; ++i) {
            size_t hash_val = std::hash<std::string>{}(combined + std::to_string(i));
            bit_array[hash_val % filter_size] = true;
        }
    }

    // Check if (A, B) is in the Bloom filter
    bool query(int a, int b) const {
        std::string combined = combineKeys(a, b);
        for (int i = 0; i < num_hash_functions; ++i) {
            size_t hash_val = std::hash<std::string>{}(combined + std::to_string(i));
            if (!bit_array[hash_val % filter_size]) {
                return false; // If any bit is unset, the pair is not in the filter
            }
        }
        return true; // Pair might be in the filter
    }
};

#define MEM_LIMIT_BYTES 4194304
#define MAX_VALUE 20000000
#define BUCKETS 131072
#define BUCKET_SIZE 152
#define BINS 512
#define BIN_SIZE 39062
#define OFFSET 500000
#define SAMPLING_RATE 0.01
#define SAMPLING_CORRECTION 100

BloomFilter bloomFilter(256 * 1024 * 8, 5);      // .25 MB Bloom filter with 5 hash functions
CountMinSketch CMS_A(2000, 32);                   // 2000 * 32 * 4 = 256000 Bytes = 0.25 MB
CountMinSketch CMS_B(2000, 32);                   // 2000 * 32 * 4 = 256000 Bytes = 0.25 MB

//259740.25974026
//38986.354775828

u_int32_t histogram512[BINS][BINS] = {0}; // 512 * 512 * 4 = 1,048,576 Bytes = 1 MB
u_int32_t histogram256[BINS/2][BINS/2] = {0}; // 256 * 256 * 4 = 262,144 Bytes = 0.25 MB
u_int32_t histogram128[BINS/4][BINS/4] = {0}; // 128 * 128 * 4 = 131,072 Bytes = 0.125 MB
u_int32_t histogram64[BINS/8][BINS/8] = {0}; // 64 * 64 * 4 = 65,536 Bytes = 0.0625 MB
u_int32_t histogram32[BINS/16][BINS/16] = {0}; // 32 * 32 * 4 = 32,768 Bytes = 0.03125 MB
u_int32_t histogram16[BINS/32][BINS/32] = {0}; // 16 * 16 * 4 = 16,384 Bytes = 0.015625 MB
u_int32_t histogram8[BINS/64][BINS/64] = {0}; // 8 * 8 * 4 = 8,192 Bytes = 0.0078125 MB
u_int32_t histogram4[BINS/128][BINS/128] = {0}; // 4 * 4 * 4 = 4,096 Bytes = 0.00390625 MB
//Total Memory for histograms: ~1 MB

void* histogram[8] = {histogram512, histogram256, histogram128, histogram64, histogram32, histogram16, histogram8, histogram4};

u_int32_t buckets_of_A1[BUCKETS] = {0}; // 131072 * 4 = 524288 Bytes = 0.5 MB
u_int32_t buckets_of_A2[BUCKETS/2] = {0}; // 65536 * 4 = 262144 Bytes = 0.25 MB
u_int32_t buckets_of_A3[BUCKETS/4] = {0}; // 32768 * 4 = 131072 Bytes = 0.125 MB
u_int32_t buckets_of_A4[BUCKETS/8] = {0}; // 16384 * 4 = 65536 Bytes = 0.0625 MB
u_int32_t buckets_of_A5[BUCKETS/16] = {0}; // 8192 * 4 = 32768 Bytes = 0.03125 MB
u_int32_t buckets_of_A6[BUCKETS/32] = {0}; // 4096 * 4 = 16384 Bytes = 0.015625 MB
u_int32_t buckets_of_A7[BUCKETS/64] = {0}; // 2048 * 4 = 8192 Bytes = 0.0078125 MB
u_int32_t buckets_of_A8[BUCKETS/128] = {0}; // 1024 * 4 = 4096 Bytes = 0.00390625 MB
u_int32_t buckets_of_A9[BUCKETS/256] = {0}; // 512 * 4 = 2048 Bytes = 0.001953125 MB
u_int32_t buckets_of_A10[BUCKETS/512] = {0}; // 256 * 4 = 1024 Bytes = 0.0009765625 MB
u_int32_t buckets_of_A11[BUCKETS/1024] = {0}; // 128 * 4 = 512 Bytes = 0.00048828125 MB
u_int32_t buckets_of_A12[BUCKETS/2048] = {0}; // 64 * 4 = 256 Bytes = 0.000244140625 MB
u_int32_t buckets_of_A13[BUCKETS/4096] = {0}; // 32 * 4 = 128 Bytes = 0.0001220703125 MB
u_int32_t buckets_of_A14[BUCKETS/8192] = {0}; // 16 * 4 = 64 Bytes = 0.00006103515625 MB
u_int32_t buckets_of_A15[BUCKETS/16384] = {0}; // 8 * 4 = 32 Bytes = 0.000030517578125 MB
u_int32_t buckets_of_A16[BUCKETS/32768] = {0}; // 4 * 4 = 16 Bytes = 0.0000152587890625 MB
//Total Memory for buckets_of_A: ~1 MB

void *buckets_of_A[16] = {buckets_of_A1, buckets_of_A2, buckets_of_A3, buckets_of_A4, buckets_of_A5, buckets_of_A6, buckets_of_A7, buckets_of_A8, buckets_of_A9, buckets_of_A10, buckets_of_A11, buckets_of_A12, buckets_of_A13, buckets_of_A14, buckets_of_A15, buckets_of_A16};

u_int32_t buckets_of_B1[BUCKETS] = {0}; // 131072 * 4 = 524288 Bytes = 0.5 MB
u_int32_t buckets_of_B2[BUCKETS/2] = {0}; // 65536 * 4 = 262144 Bytes = 0.25 MB
u_int32_t buckets_of_B3[BUCKETS/4] = {0}; // 32768 * 4 = 131072 Bytes = 0.125 MB
u_int32_t buckets_of_B4[BUCKETS/8] = {0}; // 16384 * 4 = 65536 Bytes = 0.0625 MB
u_int32_t buckets_of_B5[BUCKETS/16] = {0}; // 8192 * 4 = 32768 Bytes = 0.03125 MB
u_int32_t buckets_of_B6[BUCKETS/32] = {0}; // 4096 * 4 = 16384 Bytes = 0.015625 MB
u_int32_t buckets_of_B7[BUCKETS/64] = {0}; // 2048 * 4 = 8192 Bytes = 0.0078125 MB
u_int32_t buckets_of_B8[BUCKETS/128] = {0}; // 1024 * 4 = 4096 Bytes = 0.00390625 MB
u_int32_t buckets_of_B9[BUCKETS/256] = {0}; // 512 * 4 = 2048 Bytes = 0.001953125 MB
u_int32_t buckets_of_B10[BUCKETS/512] = {0}; // 256 * 4 = 1024 Bytes = 0.0009765625 MB
u_int32_t buckets_of_B11[BUCKETS/1024] = {0}; // 128 * 4 = 512 Bytes = 0.00048828125 MB
u_int32_t buckets_of_B12[BUCKETS/2048] = {0}; // 64 * 4 = 256 Bytes = 0.000244140625 MB
u_int32_t buckets_of_B13[BUCKETS/4096] = {0}; // 32 * 4 = 128 Bytes = 0.0001220703125 MB
u_int32_t buckets_of_B14[BUCKETS/8192] = {0}; // 16 * 4 = 64 Bytes = 0.00006103515625 MB
u_int32_t buckets_of_B15[BUCKETS/16384] = {0}; // 8 * 4 = 32 Bytes = 0.000030517578125 MB
u_int32_t buckets_of_B16[BUCKETS/32768] = {0}; // 4 * 4 = 16 Bytes = 0.0000152587890625 MB
//Total Memory for buckets_of_B: ~1 MB

void *buckets_of_B[16] = {buckets_of_B1, buckets_of_B2, buckets_of_B3, buckets_of_B4, buckets_of_B5, buckets_of_B6, buckets_of_B7, buckets_of_B8, buckets_of_B9, buckets_of_B10, buckets_of_B11, buckets_of_B12, buckets_of_B13, buckets_of_B14, buckets_of_B15, buckets_of_B16};

u_int32_t init_size = 0;
u_int32_t curr_size = 0;
u_int32_t multiplier = SAMPLING_CORRECTION;

// Total Memory for data structure: 1,003 + 0.99 + 0.99 = 2.97 MB

void CEEngine::insertTuple(const std::vector<int>& tuple)
{
    // Implement your insert tuple logic here.

    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

    bloomFilter.insert(A, B);
    CMS_A.insert(A);
    CMS_B.insert(B);

    int index_a, index_b, size;
    
    size = 512;
    for(int i = 0; i < 8; i++) {
        index_a = A/(MAX_VALUE/size) < size ? A/(MAX_VALUE/size) : size-1;
        index_b = B/(MAX_VALUE/size) < size ? B/(MAX_VALUE/size) : size-1;
        ((u_int32_t(*)[size])histogram[i])[index_a][index_b]++;
        size /= 2;
    }
    // histogram512[A/BIN_SIZE][B/BIN_SIZE]++;

    // buckets_of_A1[A/BUCKET_SIZE]++;
    // buckets_of_B1[B/BUCKET_SIZE]++;

    size = BUCKETS;
    for (int i = 0; i < 16; i++) {
        index_a = A/(MAX_VALUE/size) < size ? A/(MAX_VALUE/size) : size-1;
        index_b = B/(MAX_VALUE/size) < size ? B/(MAX_VALUE/size) : size-1;
        ((u_int32_t*)buckets_of_A[i])[index_a]++;
        ((u_int32_t*)buckets_of_B[i])[index_b]++;
        size /= 2;
    }

    curr_size++;
    multiplier = SAMPLING_CORRECTION * (double)(curr_size)/init_size;
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId)
{
    // Implement your delete tuple logic here.

    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

    int index_a, index_b, size;
    size = BINS;
    for(int i = 0; i < 8; i++) {
        index_a = A/(MAX_VALUE/size) < size ? A/(MAX_VALUE/size) : size-1;
        index_b = B/(MAX_VALUE/size) < size ? B/(MAX_VALUE/size) : size-1;
        if(((u_int32_t(*)[size])histogram[i])[index_a][index_b])
            ((u_int32_t(*)[size])histogram[i])[index_a][index_b]--;
        size /= 2;
    }

    size = BUCKETS;
    for (int i = 0; i < 16; i++){
        index_a = A/(MAX_VALUE/size) < size ? A/(MAX_VALUE/size) : size-1;
        index_b = B/(MAX_VALUE/size) < size ? B/(MAX_VALUE/size) : size-1;
        if( ((u_int32_t*)buckets_of_A[i])[index_a]) ((u_int32_t*)buckets_of_A[i])[index_a]--;
        if( ((u_int32_t*)buckets_of_B[i])[index_b]) ((u_int32_t*)buckets_of_B[i])[index_b]--;
        size /= 2;
    }
    
    if(curr_size){
        curr_size--;
        multiplier = SAMPLING_CORRECTION * (double)(curr_size)/init_size;
    }
        
}

int CEEngine::query(const std::vector<CompareExpression>& quals)
{
    // Implement your query logic here.

    if (quals.size() == 1) {
        // A = x OR B = x | // Time Complexity: O(1)
        if (quals[0].compareOp == EQUAL) {
            
            if (quals[0].columnIdx == 0) {
                return CMS_A.query(quals[0].value)*multiplier;
            } else {
                return CMS_B.query(quals[0].value)*multiplier;
            }

            //Probabilistic
            // return 1; // 1 is the best case scenario (20 mil tuples, 20 mil unique values)
            
        }

        // A > x OR B > x | // Time Complexity: O(|Buckets|)
        
        if (quals[0].compareOp == GREATER) {
            u_int32_t A = quals[0].value;
            u_int32_t B = quals[0].value;
            
            u_int32_t total_count = 0;

            // A > x
            u_int32_t i = A/BUCKET_SIZE+1 < BUCKETS ? A/BUCKET_SIZE+1 : BUCKETS-1;
            if (quals[0].columnIdx == 0) {

                // for (i; i < BUCKETS; i++) {
                //     total_count += buckets_of_A1[i];
                // }

                int size = BUCKETS;
                int index = A/BUCKET_SIZE+1 < size ? A/BUCKET_SIZE+1 : size-1;
                for(int i=0; i < 15; i++) {
                    if(index % 2 == 1)
                        total_count += ((u_int32_t*)buckets_of_A[i])[index++];
                    index /= 2;
                }
                for(int i = index;i<4;i++)
                    total_count += ((u_int32_t*)buckets_of_A[15])[i];

            // B > X
            } else {

                // u_int32_t i = B/BUCKET_SIZE+1 < BUCKETS ? B/BUCKET_SIZE+1 : BUCKETS-1;
                // for (i; i < BUCKETS; i++) {
                //     total_count += buckets_of_B1[i];
                // }

                int size = BUCKETS;
                int index = B/BUCKET_SIZE+1 < size ? B/BUCKET_SIZE+1 : size-1;
                for(int i=0; i < 15; i++) {
                    if(index % 2 == 1)
                        total_count += ((u_int32_t*)buckets_of_B[i])[index++];
                    index /= 2;
                }
                for(int i = index;i<4;i++)
                    total_count += ((u_int32_t*)buckets_of_B[15])[i];
            }

            return total_count*multiplier;

            // Approximation, best so far
            // return curr_size/2;
        }
    } 
    
    if (quals.size() == 2) {

        // A = x AND B = y
        if (quals[0].compareOp == EQUAL && quals[1].compareOp == EQUAL) {

            if (quals[0].columnIdx == 0) {
                return bloomFilter.query(quals[0].value, quals[1].value);
            } else {
                return bloomFilter.query(quals[1].value, quals[0].value);
            }

            //Proven best approximation, so far
            // return 0;
        }

        // A = x AND B > y
        if (quals[0].compareOp == EQUAL && quals[1].compareOp == GREATER) {
            int count;
            if (quals[0].columnIdx == 0) {
                count = CMS_A.query(quals[0].value)*multiplier;
            } else {
                count = CMS_B.query(quals[0].value)*multiplier;
            }
            return count*((double)(MAX_VALUE-quals[1].value)/MAX_VALUE);
            //Proven best approximation, so far
            return 0;
        }

        // A > x AND B = y
        if (quals[0].compareOp == GREATER && quals[1].compareOp == EQUAL) {
            int count;
            if (quals[1].columnIdx == 0) {
                count = CMS_A.query(quals[1].value)*multiplier;
            } else {
                count = CMS_B.query(quals[1].value)*multiplier;
            }
            return count*((double)(MAX_VALUE-quals[0].value)/MAX_VALUE);
            //Proven best approximation, so far
            return 0;
        }

        // A > x AND B > y
        if (quals[0].compareOp == GREATER && quals[1].compareOp == GREATER) {

            u_int32_t A;
            u_int32_t B;

            if(quals[0].columnIdx == 0) {
                A = quals[0].value;
                B = quals[1].value;
            } else {
                A = quals[1].value;
                B = quals[0].value;
            }

            u_int32_t total_count = 0;
            int index_a, index_b, size;

            //Logarithmic search in histograms
            index_a = A/BIN_SIZE < BINS ? A/BIN_SIZE : BINS-1;
            index_b = B/BIN_SIZE < BINS ? B/BIN_SIZE : BINS-1;
            size = 512;
            
            //Estimation for first row/column
            for (int i=index_b; i<size; i++) {
                total_count += ((u_int32_t(*)[size])histogram[0])[index_a][i] / 2;
            }
            for (int i=index_a; i<size; i++) {
                total_count += ((u_int32_t(*)[size])histogram[0])[i][index_b] / 2;
            }

            total_count -= ((u_int32_t(*)[size])histogram[0])[index_a][index_b] / 2;

            index_a = A/BIN_SIZE+1 < size ? A/BIN_SIZE+1  : size-1;
            index_b = B/BIN_SIZE+1 < size ? B/BIN_SIZE+1  : size-1;
            bool flag_a, flag_b;
            for(int i = 0; i < 7; i++) {
                flag_a = flag_b = false;
                if(index_a % 2){
                    for(int j = index_b; j < size; j++) {
                        total_count += ((u_int32_t(*)[size])histogram[i])[index_a][j];
                    }
                    index_a++;
                    flag_a = true;
                }
                if(index_b % 2){
                    for(int j = index_a; j < size; j++) {
                        total_count += ((u_int32_t(*)[size])histogram[i])[j][index_b];
                    }
                    index_b++;
                    flag_b = true;
                }
                if(flag_a && flag_b)
                    total_count -= ((u_int32_t(*)[size])histogram[i])[index_a-1][index_b-1];

                size /= 2;
                index_a /= 2;
                index_b /= 2;
            }
            for(int i = index_a; i < size; i++) {
                for(int j = index_b; j < size; j++) {
                    total_count += ((u_int32_t(*)[size])histogram[7])[i][j];
                }
            }

            return total_count*multiplier;

            // Approximation, best so far
            // return curr_size/2;
        }
    }
}

void CEEngine::prepare()
{
    // Implement your prepare logic here.
}

CEEngine::CEEngine(int num, DataExecuter *dataExecuter)
{
    init_size = num;
    curr_size = num;
    this->dataExecuter = dataExecuter;

    // Read all data from dataExecuter
    
    std::vector<std::vector<int>> data;
    u_int32_t A,B;

    for (int i = 0; i < num; i+=OFFSET) {
        data.clear();
        dataExecuter->readTuples(i, OFFSET*SAMPLING_RATE, data);
        for(int j = 0; j < OFFSET*SAMPLING_RATE; j++){

            A = data[j][0];
            B = data[j][1];

            bloomFilter.insert(A, B);
            CMS_A.insert(A);
            CMS_B.insert(B);


            int index_a, index_b, size;

            size = 512;
            for(int i = 0; i < 8; i++) {    
            index_a = A/(MAX_VALUE/size) < size ? A/(MAX_VALUE/size) : size-1;
            index_b = B/(MAX_VALUE/size) < size ? B/(MAX_VALUE/size) : size-1;
                ((u_int32_t(*)[size])histogram[i])[index_a][index_b]++;
                size /= 2;
            }

            size = BUCKETS;
            for (int i = 0; i < 16; i++) {
                index_a = A/(MAX_VALUE/size) < size ? A/(MAX_VALUE/size) : size-1;
                index_b = B/(MAX_VALUE/size) < size ? B/(MAX_VALUE/size) : size-1;
                ((u_int32_t*)buckets_of_A[i])[index_a]++;
                ((u_int32_t*)buckets_of_B[i])[index_b]++;
                size /= 2;
            }
        }
    }
    data.clear();
}