//
// You should modify this file.
//
#include <common/Root.h>
#include <CardinalityEstimation.h>

#define MEM_LIMIT_BYTES 4194304
#define MAX_VALUE 20000000
#define BUCKETS 259741
#define BINS 512
#define BIN_SIZE 39062
#define BUCKET_SIZE 77
#define OFFSET 500000
#define SAMPLING_RATE 0.01
#define SAMPLING_CORRECTION 100

//259740.25974026
//38986.354775828

u_int32_t histogram512[BINS][BINS] = {0}; // 513 * 513 * 4 = 1,052,676 Bytes = 1,003 MB
u_int32_t histogram256[BINS/2][BINS/2] = {0}; // 257 * 257 * 4 = 264,196 Bytes = 0.25 MB
u_int32_t histogram128[BINS/4][BINS/4] = {0}; // 129 * 129 * 4 = 66,516 Bytes = 0.06 MB
u_int32_t histogram64[BINS/8][BINS/8] = {0}; // 65 * 65 * 4 = 16,900 Bytes = 0.02 MB
u_int32_t histogram32[BINS/16][BINS/16] = {0}; // 33 * 33 * 4 = 4,356 Bytes = 0.004 MB
u_int32_t histogram16[BINS/32][BINS/32] = {0}; // 17 * 17 * 4 = 1,108 Bytes = 0.001 MB
u_int32_t histogram8[BINS/64][BINS/64] = {0}; // 9 * 9 * 4 = 324 Bytes = 0.0003 MB
u_int32_t histogram4[BINS/128][BINS/128] = {0}; // 5 * 5 * 4 = 100 Bytes = 0.0001 MB

void* histogram[8] = {histogram512, histogram256, histogram128, histogram64, histogram32, histogram16, histogram8, histogram4};

u_int32_t buckets_of_A[BUCKETS] = {0}; // 259741 * 4 = 1,038,964 Bytes = 0.99 MB  | bins per bucket = 77
u_int32_t buckets_of_B[BUCKETS] = {0}; // 259741 * 4 = 1,038,964 Bytes = 0.99 MB  | bins per bucket = 77

u_int32_t init_size = 0;

// Total Memory for data structure: 1,003 + 0.99 + 0.99 = 2.97 MB

void CEEngine::insertTuple(const std::vector<int>& tuple)
{
    // Implement your insert tuple logic here.

    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];
    int size = 512;
    for(int i = 0; i < 8; i++) {
        ((u_int32_t(*)[size])histogram[i])[A/(MAX_VALUE/size)][B/(MAX_VALUE/size)]++;
        size /= 2;
    }
    // histogram512[A/BIN_SIZE][B/BIN_SIZE]++;

    buckets_of_A[A/BUCKET_SIZE]++;
    buckets_of_B[B/BUCKET_SIZE]++;

    init_size++;
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId)
{
    // Implement your delete tuple logic here.

    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

    int size = 512;
    for(int i = 0; i < 8; i++) {
        ((u_int32_t(*)[size])histogram[i])[A/(MAX_VALUE/size)][B/(MAX_VALUE/size)]--;
        size /= 2;
    }
    // histogram512[A/BIN_SIZE][B/BIN_SIZE]--;

    buckets_of_A[A/BUCKET_SIZE]--;
    buckets_of_B[B/BUCKET_SIZE]--;

    init_size--;
}

int CEEngine::query(const std::vector<CompareExpression>& quals)
{
    // Implement your query logic here.

    if (quals.size() == 1) {
        // A = x OR B = x | // Time Complexity: O(1)
        if (quals[0].compareOp == EQUAL) {
            u_int32_t value = quals[0].value;

            //Probabilistic
            // return quals[0].columnIdx == 0 ? (buckets_of_A[value/BUCKET_SIZE]/BUCKET_SIZE)*SAMPLING_CORRECTION : (buckets_of_B[value/BUCKET_SIZE]/BUCKET_SIZE)*SAMPLING_CORRECTION;
            return 1; // 1 is the best case scenario (20 mil tuples, 20 mil unique values)
            
        }

        // A > x OR B > x | // Time Complexity: O(|Buckets|)
        if (quals[0].compareOp == GREATER) {
            u_int32_t A = quals[0].value;
            u_int32_t B = quals[0].value;
            
            u_int32_t total_count = 0;

            // A > x
            if (quals[0].columnIdx == 0) {
                for (u_int32_t i = A/BUCKET_SIZE+1; i < BUCKETS; i++) {
                    total_count += buckets_of_A[i];
                }

                // Experimental
                // u_int32_t last_element = (A/BUCKET_SIZE+1)*(BUCKET_SIZE)-1;
                // u_int32_t proportion = (last_element - A)/10 * buckets_of_A[A/BUCKET_SIZE];
                // total_count += proportion;

            // B > X
            } else {
                for (u_int32_t i = B/BUCKET_SIZE+1; i < BUCKETS; i++) {
                    total_count += buckets_of_B[i];
                }
                
                // Experimental
                // u_int32_t last_element = (B/BUCKET_SIZE+1)*(BUCKET_SIZE)-1;
                // u_int32_t proportion = (last_element - B)/10 * buckets_of_B[B/BUCKET_SIZE];
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

            // if (buckets_of_A[A/BUCKET_SIZE] == 0) return 0;
            // if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;

            // for (u_int32_t i = B/BIN_SIZE+1; i < BINS; i++) {
            //     total_count += histogram[A/BIN_SIZE][i];
            // }

            // //total_count /= BIN_SIZE;

            // return total_count*SAMPLING_CORRECTION;

            //Probabilistic (pure speculation of uniform distribution, no sampling correction needed, no sampled data used)

            //return ( ( (MAX_VALUE - quals[1].value) / MAX_VALUE )*init_size ) / MAX_VALUE;

            return 1;
        }

        // A > x AND B = y
        if (quals[0].compareOp == GREATER && quals[1].compareOp == EQUAL) {
            // u_int32_t A = quals[0].value;
            // u_int32_t B = quals[1].value;
            // u_int32_t total_count = 0;

            // if (buckets_of_B[B/BUCKET_SIZE] == 0) return 0;
            // if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;

            // for (u_int32_t i = A/BIN_SIZE+1; i < BINS; i++) {
            //     total_count += histogram[i][B/BIN_SIZE];
            // }

            // //total_count /= BIN_SIZE;

            // return total_count*SAMPLING_CORRECTION;

            //Probabilistic (pure speculation of uniform distribution, no sampling correction needed, no sampled data used)
            //return ( ( (MAX_VALUE - quals[0].value) / MAX_VALUE )*init_size ) / MAX_VALUE;

            return 1;
        }

        // A > x AND B > y
        if (quals[0].compareOp == GREATER && quals[1].compareOp == GREATER) {
            u_int32_t A = quals[0].value;
            u_int32_t B = quals[1].value;

            u_int32_t total_count = 0;
            for (u_int32_t i = A/BIN_SIZE+1; i < BINS; i++) {
                for (u_int32_t j = B/BIN_SIZE+1; j < BINS; j++) {
                    total_count += ((u_int32_t(*)[BINS])histogram[0])[i][j];
                    // total_count += histogram512[i][j];
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
    // u_int32_t data_structures_memory = sizeof(histogram) + sizeof(buckets_of_A) + sizeof(buckets_of_B);

    // Read all data from dataExecuter
    
    std::vector<std::vector<int>> data;
    u_int32_t A,B;

    for (int i = 0; i < num; i+=OFFSET) {
        data.clear();
        dataExecuter->readTuples(i, OFFSET*SAMPLING_RATE, data);
        for(int j = 0; j < OFFSET*SAMPLING_RATE; j++){

            A = data[j][0];
            B = data[j][1];

            int size = 512;
            for(int i = 0; i < 8; i++) {
                ((u_int32_t(*)[size])histogram[i])[A/(MAX_VALUE/size)][B/(MAX_VALUE/size)]++;
                size /= 2;
            }
            // histogram512[A/BIN_SIZE][B/BIN_SIZE]++;
            buckets_of_A[A/BUCKET_SIZE]++;
            buckets_of_B[B/BUCKET_SIZE]++;

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