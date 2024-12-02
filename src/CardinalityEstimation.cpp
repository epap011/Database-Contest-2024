//
// You should modify this file.
//
#include <common/Root.h>
#include <CardinalityEstimation.h>

#define MEM_LIMIT_BYTES 4194304
#define MAX_VALUE 20000000
#define BUCKET_SIZE_BYTES 8
#define BUCKET_DIVISION 78
#define BUCKET_SIZE 20000000 / BUCKET_DIVISION

// 259740: 0 - 259739
// 20000000 / 77 = 259740.25974026
// 20000000 / 78 = 256410.256410256

// Buckets for A: 262144 
// Buckets for B: 262144
// Entries per bucket: 20,000,000 / 262144 = 76.29 ~77
// Total buckets: 20,000,000 / 77 = 259,740
//Buckets of A: _ _ _ _ _ _ _ _ _ _   _
//              [0 1 2 3 4 5...75] [76 77 78 79... 151] [152...20,000,000-1

struct Bucket {
    u_int32_t cnt; // worst case: 20,000,000 + 50,000,000 = 70,000,000
    u_int32_t sum; // worst case: 20,000,000 x 70,000,000 = 1,400,000,000
    
};

Bucket buckets_of_A[259740] = {0};
Bucket buckets_of_B[259740] = {0};

void CEEngine::insertTuple(const std::vector<int>& tuple)
{
    // Implement your insert tuple logic here.
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId)
{
    // Implement your delete tuple logic here.
}

int CEEngine::query(const std::vector<CompareExpression>& quals)
{
    // Implement your query logic here.
    return 0;
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
        buckets_of_A[A / BUCKET_DIVISION].cnt += 1;
        buckets_of_A[A / BUCKET_DIVISION].sum += A;

        u_int32_t B = data[0][1];
        buckets_of_B[B / BUCKET_DIVISION].cnt += 1;
        buckets_of_B[B / BUCKET_DIVISION].sum += B;

        data.clear();
    }
}
