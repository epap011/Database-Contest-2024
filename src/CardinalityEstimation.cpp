//
// You should modify this file.
//
#include <common/Root.h>
#include <CardinalityEstimation.h>
#include <BloomFilter.h>
#include <fstream>

#define MEM_LIMIT_BYTES 4194304
#define BUCKETS 259741
#define BINS 513
#define BIN_SIZE 39062
#define BUCKET_SIZE 77
#define OFFSET 500000
#define SAMPLING_RATE 0.01
#define SAMPLING_CORRECTION 100

//259740.25974026
//38986.354775828

u_int32_t histogram[BINS][BINS] = {0}; // 513 * 513 * 4 = 1,052,676 Bytes = 1,003 MB
u_int32_t buckets_of_A[BUCKETS] = {0}; // 259741 * 4 = 1,038,964 Bytes = 0.99 MB  | bins per bucket = 77
u_int32_t buckets_of_B[BUCKETS] = {0}; // 259741 * 4 = 1,038,964 Bytes = 0.99 MB  | bins per bucket = 77
BloomFilter bloomFilter;

// Total Memory for data structure: 1,003 + 0.99 + 0.99 = 2.97 MB

void CEEngine::insertTuple(const std::vector<int>& tuple)
{
    // Implement your insert tuple logic here.

    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

    histogram[A/BIN_SIZE][B/BIN_SIZE]++;
    buckets_of_A[A/BUCKET_SIZE]++;
    buckets_of_B[B/BUCKET_SIZE]++;
    bloomFilter.insert(A, B);
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId)
{
    // Implement your delete tuple logic here.

    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

    histogram[A/BIN_SIZE][B/BIN_SIZE]--;
    buckets_of_A[A/BUCKET_SIZE]--;
    buckets_of_B[B/BUCKET_SIZE]--;
}

int CEEngine::query(const std::vector<CompareExpression>& quals)
{
    // Implement your query logic here.
    u_int32_t A, B;
    u_int32_t total_count = 0;

    if (quals.size() == 1) {
        A = quals[0].value;
        B = quals[0].value;
        // A = x OR B = y | // Time Complexity: O(1)
        if (quals[0].compareOp == 0) {
            return 1;
            return quals[0].columnIdx == 0 ? (buckets_of_A[A/BUCKET_SIZE]/BUCKET_SIZE)*(SAMPLING_CORRECTION) : (buckets_of_B[B/BUCKET_SIZE]/BUCKET_SIZE)*(SAMPLING_CORRECTION);
        }

        // A > x OR B > y | // Time Complexity: O(|Buckets|)
        if (quals[0].compareOp == 1) {
            

            // A > x
            if (quals[0].columnIdx == 0) {
                for (u_int32_t i = A/BUCKET_SIZE+1; i < BUCKETS; i++) {
                    total_count += buckets_of_A[i];
                }

                u_int32_t last_element = (A/BUCKET_SIZE+1)*(BUCKET_SIZE)-1;
                u_int32_t proportion = (last_element - A)/10 * buckets_of_A[A/BUCKET_SIZE];
                total_count += proportion;
            // B > y
            } else {
                for (u_int32_t i = B/BUCKET_SIZE+1; i < BUCKETS; i++) {
                    total_count += buckets_of_B[i];
                }

                u_int32_t last_element = (B/BUCKET_SIZE+1)*(BUCKET_SIZE)-1;
                u_int32_t proportion = (last_element - B)/10 * buckets_of_B[B/BUCKET_SIZE];
                total_count += proportion;
            }

            return total_count*(SAMPLING_CORRECTION);
        }
    } 
    
    else if (quals.size() == 2) {

        A = quals[0].value;
        B = quals[1].value;
        
        u_int32_t A_bin = A/BIN_SIZE;
        double A_multiplier = ((A_bin+1)*(BIN_SIZE) - A)/BIN_SIZE;

        u_int32_t B_bin = B/BIN_SIZE;
        double B_multiplier = ((B_bin+1)*(BIN_SIZE) - B)/BIN_SIZE;

        //A = x AND B = y
        if (quals[0].compareOp == 0 && quals[1].compareOp == 0) {

            if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;
            
            return (bloomFilter.query(A, B) ? 1 : 0);
        }

        // // A = x AND B > y
        if (quals[0].compareOp == 0 && quals[1].compareOp == 1) {
            return 0;

            if (buckets_of_A[A/BUCKET_SIZE] == 0) return 0;
            //The following is wrong, because B>y might still exist in bins after B/BIN_SIZE
            //if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;

            //First bin
            total_count += histogram[A/BIN_SIZE][B/BIN_SIZE]*B_multiplier;

            for (u_int32_t i = B/BIN_SIZE+1; i < BINS; i++) {
                total_count += histogram[A/BIN_SIZE][i];
            }

            return total_count*(SAMPLING_CORRECTION);
        }

        // // A > x AND B = y
        if (quals[0].compareOp == 1 && quals[1].compareOp == 0) {
            return 1;
            if (buckets_of_B[B/BUCKET_SIZE] == 0) return 0;

            //The following is wrong, because A>x might still exist in bins after A/BIN_SIZE
            //if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;

            //First bin
            total_count += histogram[A/BIN_SIZE][B/BIN_SIZE]*A_multiplier;

            for (u_int32_t i = A/BIN_SIZE+1; i < BINS; i++) {
                total_count += histogram[i][B/BIN_SIZE];
            }

            return total_count*(SAMPLING_CORRECTION);
        }

        // // A > x AND B > y
        if (quals[0].compareOp == 1 && quals[1].compareOp == 1) {
            
            // A bin column

            for (u_int32_t i = B/BIN_SIZE; i < BINS; i++) {
                total_count += histogram[A/BIN_SIZE][i]*A_multiplier;
            }

            // B bin line
            // here, i will be A/BIN_SIZE+1 because we already counted the first bin

            for (u_int32_t i = A/BIN_SIZE+1; i < BINS; i++) {
                total_count += histogram[i][B/BIN_SIZE]*B_multiplier;
            }

            // Rectangle after bin corresponding to A and B
            for (u_int32_t i = A/BIN_SIZE+1; i < BINS; i++) {
                for (u_int32_t j = B/BIN_SIZE+1; j < BINS; j++) {
                    total_count += histogram[i][j];
                }
            }
            return total_count*(SAMPLING_CORRECTION);
        }
    }
}

void CEEngine::prepare()
{
    // Implement your prepare logic here.
}

CEEngine::CEEngine(int num, DataExecuter *dataExecuter)
{
    // Implement your constructor here.
    this->dataExecuter = dataExecuter;
    
    // Read all data from dataExecuter
    std::vector<std::vector<int>> data;

    //Bloom Filter
    bloomFilter = BloomFilter(4 * 1024 * 1024 * 8, 5);

    for (int i = 0; i < num; i+=OFFSET*(1-SAMPLING_RATE)) {
        //dataExecuter->readTuples(i, 1, data);
        //debugging
        dataExecuter->readHardTuples(i, OFFSET*SAMPLING_RATE, "../datasets/tuples.txt", data);
        //if (i%10000 == 0) std::cout << i << " tuples read" << std::endl << "tuple: " << data[0][0] << " " << data[0][1] << std::endl;
        for (int j = 0; j < data.size(); j++) {
            u_int32_t A = data[j][0];
            u_int32_t B = data[j][1];

            histogram[A/BIN_SIZE][B/BIN_SIZE]++;
            buckets_of_A[A/BUCKET_SIZE]++;
            buckets_of_B[B/BUCKET_SIZE]++;
            bloomFilter.insert(A, B);
        }

        data.clear();
    }
    //debugging
}