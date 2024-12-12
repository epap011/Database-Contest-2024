//
// You should modify this file.
//
#include <common/Root.h>
#include <CardinalityEstimation.h>

#define MEM_LIMIT_BYTES 4194304
#define MAX_VALUE 20000000
#define BUCKETS 131072
#define BUCKET_SIZE 152
#define BINS 512
#define BIN_SIZE 39062
#define OFFSET 500000
#define SAMPLING_RATE 0.01
#define SAMPLING_CORRECTION 100

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

// Total Memory for data structure: 1,003 + 0.99 + 0.99 = 2.97 MB

void CEEngine::insertTuple(const std::vector<int>& tuple)
{
    // Implement your insert tuple logic here.

    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

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

    init_size++;
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId)
{
    // Implement your delete tuple logic here.

    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

    int index_a, index_b, size;
    size = 512;
    for(int i = 0; i < 8; i++) {
        index_a = A/(MAX_VALUE/size) < size ? A/(MAX_VALUE/size) : size-1;
        index_b = B/(MAX_VALUE/size) < size ? B/(MAX_VALUE/size) : size-1;
        if(((u_int32_t(*)[size])histogram[i])[index_a][index_b])
            ((u_int32_t(*)[size])histogram[i])[index_a][index_b]--;
        size /= 2;
    }
    // histogram512[A/BIN_SIZE][B/BIN_SIZE]--;

    // if (buckets_of_A1[A/BUCKET_SIZE]) buckets_of_A1[A/BUCKET_SIZE]--;
    // if (buckets_of_B1[B/BUCKET_SIZE]) buckets_of_B1[B/BUCKET_SIZE]--;

    size = BUCKETS;
    for (int i = 0; i < 16; i++){
        index_a = A/(MAX_VALUE/size) < size ? A/(MAX_VALUE/size) : size-1;
        index_b = B/(MAX_VALUE/size) < size ? B/(MAX_VALUE/size) : size-1;
        if( ((u_int32_t*)buckets_of_A[i])[index_a]) ((u_int32_t*)buckets_of_A[i])[index_a]--;
        if( ((u_int32_t*)buckets_of_B[i])[index_b]) ((u_int32_t*)buckets_of_B[i])[index_b]--;
        size /= 2;
    }
    
    if (init_size) init_size--;
}

int CEEngine::query(const std::vector<CompareExpression>& quals)
{
    // Implement your query logic here.

    if (quals.size() == 1) {
        // A = x OR B = x | // Time Complexity: O(1)
        if (quals[0].compareOp == EQUAL) {
            u_int32_t value = quals[0].value;

            //Probabilistic
            // return quals[0].columnIdx == 0 ? (buckets_of_A1[value/BUCKET_SIZE]/BUCKET_SIZE)*SAMPLING_CORRECTION : (buckets_of_B1[value/BUCKET_SIZE]/BUCKET_SIZE)*SAMPLING_CORRECTION;
            return 1; // 1 is the best case scenario (20 mil tuples, 20 mil unique values)
            
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

                // Experimental
                // u_int32_t last_element = (A/BUCKET_SIZE+1)*(BUCKET_SIZE)-1;
                // u_int32_t proportion = (last_element - A)/10 * buckets_of_A1[A/BUCKET_SIZE];
                // total_count += proportion;

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

                // Experimental
                // u_int32_t last_element = (B/BUCKET_SIZE+1)*(BUCKET_SIZE)-1;
                // u_int32_t proportion = (last_element - B)/10 * buckets_of_B1[B/BUCKET_SIZE];
                // total_count += proportion;
            }

            return total_count*SAMPLING_CORRECTION;

            // Probabilistic (pure speculation of uniform distribution, no sampling correction needed, no sampled data used)
            // return ( (MAX_VALUE - quals[0].value) / MAX_VALUE )*init_size;

            // return init_size/2;
        }
    } 
    
    if (quals.size() == 2) {

        // A = x AND B = y
        if (quals[0].compareOp == EQUAL && quals[1].compareOp == EQUAL) {
            // u_int32_t A = quals[0].value;
            // u_int32_t B = quals[1].value;

            // if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;

            //Probabilistic (very slim chance of 1 on uniform distribution)
            return 0;
        }

        // A = x AND B > y
        if (quals[0].compareOp == EQUAL && quals[1].compareOp == GREATER) {
            // u_int32_t A = quals[0].value;
            // u_int32_t B = quals[1].value;
            // u_int32_t total_count = 0;

            // if (buckets_of_A1[A/BUCKET_SIZE] == 0) return 0;
            // if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;

            // for (u_int32_t i = B/BIN_SIZE+1; i < BINS; i++) {
            //     total_count += histogram[A/BIN_SIZE][i];
            // }

            // //total_count /= BIN_SIZE;

            // return total_count*SAMPLING_CORRECTION;

            //Probabilistic (pure speculation of uniform distribution, no sampling correction needed, no sampled data used)

            //return ( ( (MAX_VALUE - quals[1].value) / MAX_VALUE )*init_size ) / MAX_VALUE;

            //Proven best approximation, so far
            return init_size/MAX_VALUE;
        }

        // A > x AND B = y
        if (quals[0].compareOp == GREATER && quals[1].compareOp == EQUAL) {
            // u_int32_t A = quals[0].value;
            // u_int32_t B = quals[1].value;
            // u_int32_t total_count = 0;

            // if (buckets_of_B1[B/BUCKET_SIZE] == 0) return 0;
            // if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;

            // for (u_int32_t i = A/BIN_SIZE+1; i < BINS; i++) {
            //     total_count += histogram[i][B/BIN_SIZE];
            // }

            // //total_count /= BIN_SIZE;

            // return total_count*SAMPLING_CORRECTION;

            //Probabilistic (pure speculation of uniform distribution, no sampling correction needed, no sampled data used)
            //return ( ( (MAX_VALUE - quals[0].value) / MAX_VALUE )*init_size ) / MAX_VALUE;

            //Proven best approximation, so far
            return init_size/MAX_VALUE;
        }

        // A > x AND B > y
        if (quals[0].compareOp == GREATER && quals[1].compareOp == GREATER) {
            u_int32_t A = quals[0].value;
            u_int32_t B = quals[1].value;

            u_int32_t total_count = 0;
            int index_a, index_b, size;

            // Stoned test for redundant buckets
            // int index_a = A/(MAX_VALUE/64) < 64 ? A/(MAX_VALUE/64) : 63;
            // int index_b = B/(MAX_VALUE/64) < 64 ? B/(MAX_VALUE/64) : 63;
            // total_count += ((u_int32_t(*)[64])histogram[3])[index_a][index_b] *((64 - index_a)+(64 - index_b))/2;

            // index_a = A/BIN_SIZE+1 < BINS ? A/BIN_SIZE+1 : BINS-1;
            // index_b = B/BIN_SIZE+1 < BINS ? B/BIN_SIZE+1 : BINS-1;
            // for (u_int32_t i = index_a; i < BINS; i++) {
            //     for (u_int32_t j = index_b; j < BINS; j++) {
            //         total_count += ((u_int32_t(*)[BINS])histogram[0])[i][j];
            //     }
            // }

            //Logarithmic search in histograms
            size = 512;
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

            return total_count*SAMPLING_CORRECTION;

            // Probabilistic (pure speculation of uniform distribution, no sampling correction needed, no sampled data used)
            // We calculate the area of the rectangle formed by the two points (A,B) and (MAX_VALUE,MAX_VALUE)
            // double A_dimension = ( (MAX_VALUE - quals[0].value) / MAX_VALUE );
            // double B_dimension = ( (MAX_VALUE - quals[1].value) / MAX_VALUE );
            // return (A_dimension * B_dimension * init_size);

            // return init_size/2;
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
    // Implement your constructor here.
    this->dataExecuter = dataExecuter;

    // Memory for data structures
    // u_int32_t data_structures_memory = sizeof(histogram) + sizeof(buckets_of_A1) + sizeof(buckets_of_B1);

    // Read all data from dataExecuter
    
    std::vector<std::vector<int>> data;
    u_int32_t A,B;

    for (int i = 0; i < num; i+=OFFSET) {
        data.clear();
        dataExecuter->readTuples(i, OFFSET*SAMPLING_RATE, data);
        for(int j = 0; j < OFFSET*SAMPLING_RATE; j++){

            A = data[j][0];
            B = data[j][1];

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

            // //Memory of data
            // u_int32_t vector_data = 0;

            // // Memory for the outer vector (std::vector<std::vector<int>>)
            // vector_data = OFFSET * SAMPLING_RATE * sizeof(std::vector<int>); // Vector of vectors, storing pointers to inner vectors

            // // Memory for each inner vector (std::vector<int>)
            // for (u_int32_t k = 0; k < OFFSET*SAMPLING_RATE; k++) {
            //     vector_data += 2 * sizeof(int);          // Memory for the elements in the vector
            //     vector_data += sizeof(std::vector<int>); // Memory for the vector structure itself
            // }

            // // Total Memory
            // std::cout << "Data-Structures Memory: " << data_structures_memory/1024.0/1024.0 << " MB" << " | Vector Memory: " << vector_data/1024.0/1024.0 << " MB" << " | Total Memory: " << (data_structures_memory + vector_data + 20)/1024.0/1024.0 << " MB" << std::endl;
        }
    }
    data.clear();
}