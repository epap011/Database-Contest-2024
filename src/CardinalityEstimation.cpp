//
// You should modify this file.
//
#include <common/Root.h>
#include <CardinalityEstimation.h>

#define MEM_LIMIT_BYTES 4194304
#define MAX_VALUE 20000000
#define BUCKETS 259741
#define BINS 513
#define BIN_SIZE 39062
#define BUCKET_SIZE 77

//259740.25974026
//38986.354775828

u_int32_t histogram[BINS][BINS] = {0}; // 513 * 513 * 4 = 1,052,676 Bytes = 1,003 MB
u_int32_t buckets_of_A[BUCKETS] = {0}; // 259741 * 4 = 1,038,964 Bytes = 0.99 MB  | bins per bucket = 77
u_int32_t buckets_of_B[BUCKETS] = {0}; // 259741 * 4 = 1,038,964 Bytes = 0.99 MB  | bins per bucket = 77

// Total Memory for data structure: 1,003 + 0.99 + 0.99 = 2.97 MB

void CEEngine::insertTuple(const std::vector<int>& tuple)
{
    // Implement your insert tuple logic here.

    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

    histogram[A/BIN_SIZE][B/BIN_SIZE]++;
    buckets_of_A[A/BUCKET_SIZE]++;
    buckets_of_B[B/BUCKET_SIZE]++;
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

    u_int32_t ans = 0;

    if (quals.size() == 1) {
        // A = x OR B = y | // Time Complexity: O(1)
        if (quals[0].compareOp == 0) {
            u_int32_t A = quals[0].value;
            u_int32_t B = quals[0].value;

            return quals[0].columnIdx == 0 ? buckets_of_A[A/BUCKET_SIZE]/BUCKET_SIZE : buckets_of_B[B/BUCKET_SIZE]/BUCKET_SIZE;
        }

        // A > x OR B > y | // Time Complexity: O(|Buckets|)
        if (quals[0].compareOp == 1) {
            u_int32_t A = quals[0].value;
            u_int32_t B = quals[0].value;
            
            u_int32_t total_count = 0;

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

            return total_count;
        }
    } 
    
    if (quals.size() == 2) {

        //A = x AND B = y
        if (quals[0].compareOp == 0 && quals[1].compareOp == 0) {
            u_int32_t A = quals[0].value;
            u_int32_t B = quals[1].value;

            if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;
            return 0;
        }

        // // A = x AND B > y
        if (quals[0].compareOp == 0 && quals[1].compareOp == 1) {
            u_int32_t A = quals[0].value;
            u_int32_t B = quals[1].value;
            u_int32_t total_count = 0;

            if (buckets_of_A[A/BUCKET_SIZE] == 0) return 0;
            if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;

            for (u_int32_t i = B/BIN_SIZE+1; i < BINS; i++) {
                total_count += histogram[A/BIN_SIZE][i];
            }

            return total_count;
        }

        // // A > x AND B = y
        if (quals[0].compareOp == 1 && quals[1].compareOp == 0) {
            u_int32_t A = quals[0].value;
            u_int32_t B = quals[1].value;
            u_int32_t total_count = 0;

            if (buckets_of_B[B/BUCKET_SIZE] == 0) return 0;
            if (histogram[A/BIN_SIZE][B/BIN_SIZE] == 0) return 0;

            for (u_int32_t i = A/BIN_SIZE+1; i < BINS; i++) {
                total_count += histogram[i][B/BIN_SIZE];
            }

            return total_count;
        }

        // // A > x AND B > y
        if (quals[0].compareOp == 1 && quals[1].compareOp == 1) {
            u_int32_t A = quals[0].value;
            u_int32_t B = quals[1].value;

            u_int32_t total_count = 0;
            for (u_int32_t i = A/BIN_SIZE+1; i < BINS; i++) {
                for (u_int32_t j = B/BIN_SIZE+1; j < BINS; j++) {
                    total_count += histogram[i][j];
                }
            }

            return total_count;
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
    for (int i = 0; i < num; i++) {
        dataExecuter->readTuples(i, 1, data);

        u_int32_t A = data[0][0];
        u_int32_t B = data[0][1];

        histogram[A/BIN_SIZE][B/BIN_SIZE]++;

        buckets_of_A[A/BUCKET_SIZE]++;
        buckets_of_B[B/BUCKET_SIZE]++;

        data.clear();
    }
}