#include <common/Root.h>
#include <CardinalityEstimation.h>

#include <vector>
#include <functional> // For std::hash
#include <string>     // For combining keys
#include <climits>    // For INT_MAX

#define MEM_LIMIT_BYTES 4194304
#define MAX_VALUE 20000000
#define BUCKETS 262144
#define BUCKET_SIZE (MAX_VALUE/BUCKETS)
#define BUCKET_LAYERS 16
#define BINS 512
#define BIN_SIZE (MAX_VALUE/BINS)
#define OFFSET 250000
#define SAMPLING_RATE 0.05
#define SAMPLING_CORRECTION (1/SAMPLING_RATE)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Memory for table = 4,000(WIDTH) * 32(DEPTH) * 4(BYTES)= 512,000 Bytes = 0.48828125 MB
// Memory for hash functions = 32(DEPTH) * 8(BYTES) = 256 Bytes = 0.000244141MB
// Total Memory for CountMinSketch: 0.488525391MB
class CountMinSketchAB {
private:
    static constexpr u_int32_t WIDTH = 4000; // Number of columns in the sketch
    static constexpr u_int32_t DEPTH = 32;;  // Number of hash functions (rows)
    u_int32_t table[DEPTH][WIDTH]    = {0};  // 2D table to store counts
    std::array<std::function<size_t(u_int32_t, u_int32_t)>, DEPTH> hashFunctions;// Hash functions

public:
    CountMinSketchAB() {
        // Initialize hash functions (std::hash<string> acts as hash functions)
        for (u_int32_t i = 0; i < DEPTH; ++i) {
            hashFunctions[i] = [seed = i](u_int32_t keyA, u_int32_t keyB) {
                return std::hash<u_int32_t>()(keyA) ^ (std::hash<u_int32_t>()(keyB) + seed * 0x9e3779b9);
            };
        }
    }

    // Insert an item into the sketch
    void insert(u_int32_t keyA, u_int32_t keyB) {
        for (u_int32_t i = 0; i < DEPTH; ++i) {
            size_t hashVal = hashFunctions[i](keyA, keyB);
            table[i][hashVal % WIDTH] += 1;
        }
    }

    // Query the approximate count of a key
    u_int32_t query(u_int32_t keyA, u_int32_t keyB) const {
        u_int32_t minCount = std::numeric_limits<u_int32_t>::max();
        for (u_int32_t i = 0; i < DEPTH; ++i) {
            size_t hashVal = hashFunctions[i](keyA, keyB);
            minCount = std::min(minCount, table[i][hashVal % WIDTH]);
        }
        return minCount;
    }

    // Print the sketch table
    void printTable() {
        for (u_int32_t i = 0; i < DEPTH; ++i) {
            for (u_int32_t j = 0; j < 20; ++j) {
                std::cout << table[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl << std::endl;
    }
};

class CountMinSketch {
private:
    static constexpr u_int32_t WIDTH = 4000; // Number of columns in the sketch
    static constexpr u_int32_t DEPTH = 32;;  // Number of hash functions (rows)
    u_int32_t table[DEPTH][WIDTH]    = {0};  // 2D table to store counts
    std::array<std::function<size_t(u_int32_t, u_int32_t)>, DEPTH> hashFunctions;// Hash functions

public:
    CountMinSketch() {
        // Initialize hash functions (std::hash<string> acts as hash functions)
        for (u_int32_t i = 0; i < DEPTH; ++i) {
            hashFunctions[i] = [seed = i](u_int32_t key, u_int32_t) {
                return std::hash<u_int32_t>()(key) + seed * 0x9e3779b9;
            };
        }
    }

    // Insert an item into the sketch
    void insert(u_int32_t key) {
        for (u_int32_t i = 0; i < DEPTH; ++i) {
            size_t hashVal = hashFunctions[i](key, 0);
            table[i][hashVal % WIDTH] += 1;
        }
    }

    // Query the approximate count of a key
    u_int32_t query(u_int32_t key) const {
        u_int32_t minCount = std::numeric_limits<u_int32_t>::max();
        for (u_int32_t i = 0; i < DEPTH; ++i) {
            size_t hashVal = hashFunctions[i](key, 0);
            minCount = std::min(minCount, table[i][hashVal % WIDTH]);
        }
        return minCount;
    }

    // Print the sketch table
    void printTable() {
        for (u_int32_t i = 0; i < DEPTH; ++i) {
            for (u_int32_t j = 0; j < 20; ++j) {
                std::cout << table[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl << std::endl;
    }
};

CountMinSketchAB CMS_AB;
CountMinSketch CMS_A;
CountMinSketch CMS_B;
int cmsAB_noise = 0;
int cmsA_noise = 0;
int cmsB_noise = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//Total Memory for histograms: 1.083251953MB
u_int32_t histogram512[BINS][BINS]       = {0};     // 512 * 512 * 4 = 1048576 Bytes = 1 MB
u_int32_t histogram256[BINS/2][BINS/2]   = {0};     // 256 * 256 * 4 = 65536 Bytes = 0.0625 MB
u_int32_t histogram128[BINS/4][BINS/4]   = {0};     // 128 * 128 * 4 = 16384 Bytes = 0.015625 MB
u_int32_t histogram64[BINS/8][BINS/8]    = {0};     // 64  * 64  * 4 = 4096 Bytes  = 0.00390625 MB
u_int32_t histogram32[BINS/16][BINS/16]  = {0};     // 32  * 32  * 4 = 1024 Bytes  = 0.0009765625 MB
u_int32_t histogram16[BINS/32][BINS/32]  = {0};     // 16  * 16  * 4 = 256 Bytes   = 0.000244140625 MB
u_int32_t histogram8[BINS/64][BINS/64]   = {0};     // 8   * 8   * 4 = 128 Bytes   = 0.0001220703125 MB
u_int32_t histogram4[BINS/128][BINS/128] = {0};     // 4  * 4   * 4 = 64 Bytes    = 0.00006103515625 MB
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
//Total Memory for pointers of histograms: 8pointers x 8Bytes = 0.000061035MB
void* histogram[8] = {histogram512, histogram256, histogram128, histogram64, histogram32, histogram16, histogram8, histogram4};
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
//Total Memory for buckets_of_A: 0.625457764 MB
u_int8_t buckets_of_A1[BUCKETS]         = {0};      // 262,144 * 1 = 262,144 Bytes = 0.25 MB
u_int8_t buckets_of_A2[BUCKETS/2]       = {0};      // 131,072 * 1 = 131,072 Bytes = 0.125 MB
u_int8_t buckets_of_A3[BUCKETS/4]       = {0};      // 65,536  * 1 = 65,536 Bytes  = 0.0625 MB
u_int16_t buckets_of_A4[BUCKETS/8]      = {0};      // 32,768  * 2 = 65,536 Bytes  = 0.0625 MB
u_int16_t buckets_of_A5[BUCKETS/16]     = {0};      // 16,384  * 2 = 32,768 Bytes  = 0.03125 MB
u_int16_t buckets_of_A6[BUCKETS/32]     = {0};      // 8,192   * 2 = 16,384 Bytes  = 0.015625 MB
u_int16_t buckets_of_A7[BUCKETS/64]     = {0};      // 4,096   * 2 = 8,192 Bytes   = 0.0078125 MB
u_int16_t buckets_of_A8[BUCKETS/128]    = {0};      // 2,048   * 2 = 4,096 Bytes   = 0.00390625 MB
u_int16_t buckets_of_A9[BUCKETS/256]    = {0};      // 1,024   * 2 = 2,048 Bytes   = 0.001953125 MB
u_int16_t buckets_of_A10[BUCKETS/512]   = {0};      // 512     * 2 = 1,024 Bytes   = 0.0009765625 MB
u_int16_t buckets_of_A11[BUCKETS/1024]  = {0};      // 256     * 2 = 512 Bytes     = 0.00048828125 MB
u_int16_t buckets_of_A12[BUCKETS/2048]  = {0};      // 128     * 2 = 256 Bytes     = 0.000244140625 MB
u_int32_t buckets_of_A13[BUCKETS/4096]  = {0};      // 64      * 4 = 256 Bytes     = 0.000244140625 MB
u_int32_t buckets_of_A14[BUCKETS/8192]  = {0};      // 32      * 4 = 128 Bytes     = 0.0001220703125 MB
u_int32_t buckets_of_A15[BUCKETS/16384] = {0};      // 16      * 4 = 64 Bytes      = 0.00006103515625 MB
u_int32_t buckets_of_A16[BUCKETS/32768] = {0};      // 8       * 4 = 32 Bytes      = 0.000030517578125 MB
//u_int32_t buckets_of_A17[BUCKETS/65536] = {0};    30517578125 MB
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
// Total Memory for pointers of buckets of A = 17pointers x 8Bytes = 0.0001297MB
void *buckets_of_A[BUCKET_LAYERS] = {buckets_of_A1, buckets_of_A2, buckets_of_A3, buckets_of_A4, buckets_of_A5, buckets_of_A6, buckets_of_A7, buckets_of_A8, buckets_of_A9, buckets_of_A10, buckets_of_A11, buckets_of_A12, buckets_of_A13, buckets_of_A14, buckets_of_A15, buckets_of_A16};
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
//Total Memory for buckets_of_B: 0.625457764 MB
u_int8_t buckets_of_B1[BUCKETS]         = {0};      //262,144 * 1 = 262,144 Bytes = 0.25 MB
u_int8_t buckets_of_B2[BUCKETS/2]       = {0};      //131,072 * 1 = 131,072 Bytes = 0.125 MB
u_int8_t buckets_of_B3[BUCKETS/4]       = {0};      //65,536  * 2 = 131,072 Bytes = 0.125 MB
u_int16_t buckets_of_B4[BUCKETS/8]      = {0};      //32,768  * 2 = 65,536 Bytes  = 0.0625 MB
u_int16_t buckets_of_B5[BUCKETS/16]     = {0};      //16,384  * 2 = 32,768 Bytes  = 0.03125 MB
u_int16_t buckets_of_B6[BUCKETS/32]     = {0};      //8,192   * 2 = 16,384 Bytes  = 0.015625 MB
u_int16_t buckets_of_B7[BUCKETS/64]     = {0};      //4,096   * 2 = 8,192 Bytes   = 0.0078125 MB
u_int16_t buckets_of_B8[BUCKETS/128]    = {0};      //2,048   * 2 = 4,096 Bytes   = 0.00390625 MB
u_int16_t buckets_of_B9[BUCKETS/256]    = {0};      //1,024   * 2 = 2,048 Bytes   = 0.001953125 MB
u_int16_t buckets_of_B10[BUCKETS/512]   = {0};      //512     * 2 = 1,024 Bytes   = 0.0009765625 MB
u_int16_t buckets_of_B11[BUCKETS/1024]  = {0};      //256     * 2 = 512 Bytes     = 0.00048828125 MB  
u_int16_t buckets_of_B12[BUCKETS/2048]  = {0};      //128     * 4 = 512 Bytes     = 0.00048828125 MB
u_int32_t buckets_of_B13[BUCKETS/4096]  = {0};      //64      * 4 = 256 Bytes     = 0.000244140625 MB
u_int32_t buckets_of_B14[BUCKETS/8192]  = {0};      //32      * 4 = 128 Bytes     = 0.0001220703125 MB
u_int32_t buckets_of_B15[BUCKETS/16384] = {0};      //16      * 4 = 64 Bytes      = 0.00006103515625 MB
u_int32_t buckets_of_B16[BUCKETS/32768] = {0};      //8       * 4 = 32 Bytes      = 0.000030517578125 MB
//u_int32_t buckets_of_B17[BUCKETS/65536] = {0};
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
// Total Memory for pointers of buckets of B = 17pointers x 8Bytes = 0.0001297MB
void *buckets_of_B[BUCKET_LAYERS] = {buckets_of_B1, buckets_of_B2, buckets_of_B3, buckets_of_B4, buckets_of_B5, buckets_of_B6, buckets_of_B7, buckets_of_B8, buckets_of_B9, buckets_of_B10, buckets_of_B11, buckets_of_B12, buckets_of_B13, buckets_of_B14, buckets_of_B15, buckets_of_B16};
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
//Total Memory for variables: 4Bytes + 4Bytes + 4Bytes + 8Bytes + 4Bytes + 4Bytes = 24Bytes
u_int32_t init_size = 0;
u_int32_t curr_size = 0;
double multiplier   = SAMPLING_CORRECTION;
int max_value       = MAX_VALUE;
int bin_size        = BIN_SIZE;
int bucket_size     = BUCKET_SIZE;
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
//Total Memory for data structure: CMS_AB + histogram + buckets_of_A + buckets_of_B + variables = 0.48828125 MB + 1.083251953MB + 1.062713623046875 MB + 1.062713623046875 MB + 0.0001297MB = 3.69709079909375 MB
//----------------------------------------------------------------------------------------------------

void CEEngine::insertTuple(const std::vector<int>& tuple) {
    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

    CMS_AB.insert(A,B);
    CMS_A.insert(A);
    CMS_B.insert(B);

    int index_a, index_b, size;
    
    size = BINS;
    for(int i = 0; i < 8; i++) {
        index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
        index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
        ((u_int32_t(*)[size])histogram[i])[index_a][index_b]++;
        size /= 2;
    }

    size = BUCKETS;
    for (int i = 0; i < BUCKET_LAYERS; i++) {
        index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
        index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
        if(i<3){
            ((u_int8_t*)buckets_of_A[i])[index_a]++;
            ((u_int8_t*)buckets_of_B[i])[index_b]++;
        }
        else if(i<12){
            ((u_int16_t*)buckets_of_A[i])[index_a]++;
            ((u_int16_t*)buckets_of_B[i])[index_b]++;
        }
        else{
            ((u_int32_t*)buckets_of_A[i])[index_a]++;
            ((u_int32_t*)buckets_of_B[i])[index_b]++;
        }
        size /= 2;
    }

    curr_size++;
    multiplier = (double) curr_size / (init_size*SAMPLING_RATE + (curr_size - init_size));
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId) {
    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

    int index_a, index_b, size;
    size = BINS;
    for(int i = 0; i < 8; i++) {
        index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
        index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
        if(((u_int32_t(*)[size])histogram[i])[index_a][index_b])
            ((u_int32_t(*)[size])histogram[i])[index_a][index_b]--;
        size /= 2;
    }

    size = BUCKETS;
    for (int i = 0; i < BUCKET_LAYERS; i++){
        index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
        index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
        if(i<3){
            if(((u_int8_t*)buckets_of_A[i])[index_a]) ((u_int8_t*)buckets_of_A[i])[index_a]--;
            if(((u_int8_t*)buckets_of_B[i])[index_b]) ((u_int8_t*)buckets_of_B[i])[index_b]--;
        }
        else if(i<12){
            if(((u_int16_t*)buckets_of_A[i])[index_a]) ((u_int16_t*)buckets_of_A[i])[index_a]--;
            if(((u_int16_t*)buckets_of_B[i])[index_b]) ((u_int16_t*)buckets_of_B[i])[index_b]--;
        }
        else{
            if(((u_int32_t*)buckets_of_A[i])[index_a]) ((u_int32_t*)buckets_of_A[i])[index_a]--;
            if(((u_int32_t*)buckets_of_B[i])[index_b]) ((u_int32_t*)buckets_of_B[i])[index_b]--;
        }
        size /= 2;
    }
    
    if(curr_size){
        curr_size--;
        multiplier = (double) curr_size / (init_size*SAMPLING_RATE + (curr_size - init_size));
    }
}

int CEEngine::query(const std::vector<CompareExpression>& quals) {
    //CMS_AB.printTable();
    if (quals.size() == 1) {
        // A = x OR B = x | // Time Complexity: O(1)
        if (quals[0].compareOp == EQUAL) {
            //CMS/Histogram hybrid
            int estimation = (quals[0].columnIdx == 0) ? CMS_A.query(quals[0].value) : CMS_B.query(quals[0].value);
            estimation -= (quals[0].columnIdx == 0 ? cmsA_noise : cmsB_noise);
            //Histogram fallback
            if (estimation < 0){
                estimation = quals[0].columnIdx == 0 ? ((double)(buckets_of_A1[quals[0].value/bucket_size])/bucket_size)*multiplier : ((double)(buckets_of_B1[quals[0].value/bucket_size])/bucket_size)*multiplier;
            }
            return estimation;
            //return 0;

            // return quals[0].columnIdx == 0 ? CMS_A.query(quals[0].value) : CMS_B.query(quals[0].value);
        }

        // A > x OR B > x | // Time Complexity: O(|Buckets|)
        if (quals[0].compareOp == GREATER) {
            u_int32_t value = quals[0].value;
            u_int32_t total_count = 0;
            int size = BUCKETS;
            int index = value/bucket_size+1 < size ? value/bucket_size+1 : size-1;

            // A > x
            if (quals[0].columnIdx == 0) {
                for(int i=0; i < BUCKET_LAYERS-1; i++) {
                    if(index % 2 == 1){
                        if(i<3)
                            total_count += ((u_int8_t*)buckets_of_A[i])[index++];
                        else if(i<12)
                            total_count += ((u_int16_t*)buckets_of_A[i])[index++];
                        else
                            total_count += ((u_int32_t*)buckets_of_A[i])[index++];
                    }
                    index /= 2;
                }
                for(int i = index;i<8;i++)
                    total_count += ((u_int32_t*)buckets_of_A[BUCKET_LAYERS-1])[i];

            // B > X
            } else {
                for(int i=0; i < BUCKET_LAYERS-1; i++) {
                    if(index % 2 == 1){
                        if(i<3)
                            total_count += ((u_int8_t*)buckets_of_B[i])[index++];
                        else if(i<12)
                            total_count += ((u_int16_t*)buckets_of_B[i])[index++];
                        else
                            total_count += ((u_int32_t*)buckets_of_B[i])[index++];
                    }
                    index /= 2;
                }
                for(int i = index;i<8;i++)
                    total_count += ((u_int32_t*)buckets_of_B[BUCKET_LAYERS-1])[i];
            }

            return total_count*multiplier;

            // Approximation, best so far
            // return curr_size/2;
        }
    } 
    
    if (quals.size() == 2) {

        // A = x AND B = y
        if (quals[0].compareOp == EQUAL && quals[1].compareOp == EQUAL) {
            //Proven best approximation, so far
            // return 0;
            int A,B;
            if (quals[0].columnIdx == 0) {
                A=quals[0].value;
                B=quals[1].value;
            } else {
                A=quals[1].value;
                B=quals[0].value;
            }
            int cms_count = CMS_AB.query(A,B);
            return (cms_count - cmsAB_noise) >= 0 ? cms_count - cmsAB_noise : 0;
        }

        // A = x AND B > y
        if (quals[0].compareOp == EQUAL && quals[1].compareOp == GREATER) {
            int total_count = 0;
            //Equal
            // double eqEstimation = quals[0].columnIdx == 0 ? ((double)(buckets_of_A1[quals[0].value/bucket_size])/bucket_size)*multiplier : ((double)(buckets_of_B1[quals[0].value/bucket_size])/bucket_size)*multiplier;
            //CMS based
            double eqEstimation = quals[0].columnIdx == 0 ? CMS_A.query(quals[0].value) : CMS_B.query(quals[0].value);
            eqEstimation -= (quals[0].columnIdx == 0) ? cmsA_noise : cmsB_noise;
            //probabilistic fallback
            if (eqEstimation < 0) return curr_size/max_value;
            //Greater
            int size = BUCKETS;
            int value = quals[1].value;
            int index = value/bucket_size+1 < size ? value/bucket_size+1 : size-1;
            if (quals[1].columnIdx == 0) {
                for(int i=0; i < BUCKET_LAYERS-1; i++) {
                    if(index % 2 == 1){
                        if(i<3)
                            total_count += ((u_int8_t*)buckets_of_A[i])[index++];
                        else if(i<12)
                            total_count += ((u_int16_t*)buckets_of_A[i])[index++];
                        else
                            total_count += ((u_int32_t*)buckets_of_A[i])[index++];
                    }
                    index /= 2;
                }
                for(int i = index;i<8;i++)
                    total_count += ((u_int32_t*)buckets_of_A[BUCKET_LAYERS-1])[i];
            } else {
                for(int i=0; i < BUCKET_LAYERS-1; i++) {
                    if(index % 2 == 1){
                        if(i<3)
                            total_count += ((u_int8_t*)buckets_of_B[i])[index++];
                        else if(i<12)
                            total_count += ((u_int16_t*)buckets_of_B[i])[index++];
                        else
                            total_count += ((u_int32_t*)buckets_of_B[i])[index++];
                    }
                    index /= 2;
                }
                for(int i = index;i<8;i++)
                    total_count += ((u_int32_t*)buckets_of_B[BUCKET_LAYERS-1])[i];
            }

            //Return the estimation for the first column multiplied by the ratio of the second column
            //CMS / Histogram / Probabilistic tribrid
            return eqEstimation * ((double)(total_count*multiplier)/curr_size);
        }

        // A > x AND B = y
        if (quals[0].compareOp == GREATER && quals[1].compareOp == EQUAL) {
            int total_count = 0;
            //Equal
            //double eqEstimation = quals[1].columnIdx == 0 ? ((double)(buckets_of_A1[quals[1].value/bucket_size])/bucket_size)*multiplier : ((double)(buckets_of_B1[quals[1].value/bucket_size])/bucket_size)*multiplier;
            //CMS based
            double eqEstimation = quals[1].columnIdx == 0 ? CMS_A.query(quals[1].value) : CMS_B.query(quals[1].value);
            eqEstimation -= (quals[1].columnIdx == 0) ? cmsA_noise : cmsB_noise;
            //probabilistic fallback
            if (eqEstimation < 0) return curr_size/max_value;
            //Greater
            int size = BUCKETS;
            int value = quals[0].value;
            int index = value/bucket_size+1 < size ? value/bucket_size+1 : size-1;
            if (quals[0].columnIdx == 0) {
                for(int i=0; i < BUCKET_LAYERS; i++) {
                    if(index % 2 == 1){
                        if(i<3)
                            total_count += ((u_int8_t*)buckets_of_A[i])[index++];
                        else if(i<12)
                            total_count += ((u_int16_t*)buckets_of_A[i])[index++];
                        else
                            total_count += ((u_int32_t*)buckets_of_A[i])[index++];
                    }
                    index /= 2;
                }
                for(int i = index;i<8;i++)
                    total_count += ((u_int32_t*)buckets_of_A[BUCKET_LAYERS-1])[i];
            } else {
                for(int i=0; i < BUCKET_LAYERS-1; i++) {
                    if(index % 2 == 1){
                        if(i<3)
                            total_count += ((u_int8_t*)buckets_of_B[i])[index++];
                        else if(i<12)
                            total_count += ((u_int16_t*)buckets_of_B[i])[index++];
                        else
                            total_count += ((u_int32_t*)buckets_of_B[i])[index++];
                    }
                    index /= 2;
                }
                for(int i = index;i<8;i++)
                    total_count += ((u_int32_t*)buckets_of_B[BUCKET_LAYERS-1])[i];
            }

            //Return the estimation for the first column multiplied by the ratio of the second column
            //CMS / Histogram / Probabilistic tribrid
            return eqEstimation * ((double)(total_count*multiplier)/curr_size);
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

            //Case 2: (A > x AND B > y)

            //Logarithmic search in histograms
            size = BINS;
            
            //Estimation for first row/column
            // index_a = A/bin_size < BINS ? A/bin_size : BINS-1;
            // index_b = B/bin_size < BINS ? B/bin_size : BINS-1;
            // for (int i=index_b; i<size; i++) {
            //     total_count += ((u_int32_t(*)[size])histogram[0])[index_a][i] / 2;
            // }
            // for (int i=index_a; i<size; i++) {
            //     total_count += ((u_int32_t(*)[size])histogram[0])[i][index_b] / 2;
            // }

            // total_count -= ((u_int32_t(*)[size])histogram[0])[index_a][index_b] / 2;

            // //Invalidate further calculations if the search is already at the last row/column
            // if(index_a != BINS-1 && index_b != BINS-1) {
            //     index_a = A/bin_size+1 < size ? A/bin_size+1  : size-1;
            //     index_b = B/bin_size+1 < size ? B/bin_size+1  : size-1;
            // }
            // else{
            //     index_a = size;
            //     index_b = size;
            // }

            index_a = A/bin_size+1 < size ? A/bin_size+1  : size-1;
            index_b = B/bin_size+1 < size ? B/bin_size+1  : size-1;

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

CEEngine::CEEngine(int num, DataExecuter *dataExecuter) {
    init_size = num;
    curr_size = num;
    this->dataExecuter = dataExecuter;

    std::vector<std::vector<int>> data;
    u_int32_t A,B;

    // Sample table for max value, to assign appropriate bin and bucket sizes

    // max_value = 0;
    // for (int k = 0; k < 4; k++) {
    //     data.clear();
    //     dataExecuter->readTuples(k*(num/4), 5000, data);
    //     for (int i = 0; i < 5000; i++) {
    //         A = data[i][0];
    //         B = data[i][1];
    //         if(A > max_value) max_value = A;
    //         if(B > max_value) max_value = B;
    //     }
    // }
    // data.clear();

    // Round up max value to the nearest 100,000 (seems not necessary)
    //max_value = ((max_value + 99999) / 100000) * 100000;
    //std::cout << "Max Value: " << max_value << std::endl;

    //Adjust bin and bucket sizes dynamically, based on max value
    bin_size    = max_value/BINS;
    bucket_size = max_value/BUCKETS;

    for (int i = 0; i < num; i+=OFFSET) {
        data.clear();
        dataExecuter->readTuples(i, OFFSET*SAMPLING_RATE, data);
        for(int j = 0; j < OFFSET*SAMPLING_RATE; j++){

            A = data[j][0];
            B = data[j][1];

            CMS_AB.insert(A,B);
            CMS_A.insert(A);    //boba is life in the summer
            CMS_B.insert(B);    //perfect bubble tea is a must have in the summer time

            int index_a, index_b, size;

            size = BINS;
            for(int i = 0; i < 8; i++) {    
            index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
            index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
                ((u_int32_t(*)[size])histogram[i])[index_a][index_b]++;
                size /= 2;
            }

            size = BUCKETS;
            for (int i = 0; i < BUCKET_LAYERS; i++) {
                index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
                index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
                if(i<3){
                    ((u_int8_t*)buckets_of_A[i])[index_a]++;
                    ((u_int8_t*)buckets_of_B[i])[index_b]++;
                }
                else if(i<12){
                    ((u_int16_t*)buckets_of_A[i])[index_a]++;
                    ((u_int16_t*)buckets_of_B[i])[index_b]++;
                }
                else{
                    ((u_int32_t*)buckets_of_A[i])[index_a]++;
                    ((u_int32_t*)buckets_of_B[i])[index_b]++;
                }
                size /= 2;
            }
        }
    }    
    data.clear();
    int cms_step = MAX_VALUE/200;
    for (int i=0;i<200;i++){
        cmsAB_noise += CMS_AB.query(i*cms_step,(i+1)*cms_step);
        cmsA_noise += CMS_A.query(i*cms_step);
        cmsB_noise += CMS_B.query(i*cms_step);
    }
    cmsAB_noise /= 200;
    cmsA_noise /= 200;
    cmsB_noise /= 200;
}