#include <common/Root.h>
#include <CardinalityEstimation.h>

#define MAX_VALUE 20000000
#define BUCKETS 131072 //524288
#define BUCKET_SIZE (MAX_VALUE/BUCKETS)
#define BINS 512
#define BIN_SIZE (MAX_VALUE/BINS)
#define OFFSET 250000
#define SAMPLING_RATE 0.02
#define SAMPLING_CORRECTION (1/SAMPLING_RATE)

#define DUPLICATE_COLUMNS //Uncomment to enable duplicate columns (A [>,=] x AND A [>,=] y) or (B [>,=] x AND B [>,=] y)

u_int32_t histogram512[BINS][BINS]       = {0}; // 1024 * 1024 = 1,048,576 Bytes = 1 MB
u_int32_t histogram256[BINS/2][BINS/2]   = {0}; // 512 * 512 = 262144 Bytes = 0.25 MB
u_int32_t histogram128[BINS/4][BINS/4]   = {0}; // 256 * 256 = 65536 Bytes = 0.0625 MB
u_int32_t histogram64[BINS/8][BINS/8]    = {0}; // 128 * 128 = 16384 Bytes = 0.015625 MB
u_int32_t histogram32[BINS/16][BINS/16]  = {0}; // 64 * 64 = 4096 Bytes = 0.00390625 MB
u_int32_t histogram16[BINS/32][BINS/32]  = {0}; // 32 * 32 = 1024 Bytes = 0.0009765625 MB
u_int32_t histogram8[BINS/64][BINS/64]   = {0}; // 16 * 16 = 256 Bytes = 0.000244140625 MB
u_int32_t histogram4[BINS/128][BINS/128] = {0}; // 8 * 8 = 64 Bytes = 0.00006103515625 MB
//Total Memory for histograms: ~1 MB

void* histogram[8] = {histogram512, histogram256, histogram128, histogram64, histogram32, histogram16, histogram8, histogram4};

u_int32_t buckets_of_A1[BUCKETS]        = {0};
u_int32_t buckets_of_A2[BUCKETS/2]      = {0};
u_int32_t buckets_of_A3[BUCKETS/4]      = {0};
u_int32_t buckets_of_A4[BUCKETS/8]      = {0};
u_int32_t buckets_of_A5[BUCKETS/16]     = {0};
u_int32_t buckets_of_A6[BUCKETS/32]     = {0};
u_int32_t buckets_of_A7[BUCKETS/64]     = {0};
u_int32_t buckets_of_A8[BUCKETS/128]    = {0};
u_int32_t buckets_of_A9[BUCKETS/256]    = {0};
u_int32_t buckets_of_A10[BUCKETS/512]   = {0};
u_int32_t buckets_of_A11[BUCKETS/1024]  = {0};
u_int32_t buckets_of_A12[BUCKETS/2048]  = {0};
u_int32_t buckets_of_A13[BUCKETS/4096]  = {0};
u_int32_t buckets_of_A14[BUCKETS/8192]  = {0};
u_int32_t buckets_of_A15[BUCKETS/16384] = {0};
u_int32_t buckets_of_A16[BUCKETS/32768] = {0};
//Total Memory for buckets_of_A: ~1 MB

void *buckets_of_A[16] = {buckets_of_A1, buckets_of_A2, buckets_of_A3, buckets_of_A4, buckets_of_A5, buckets_of_A6, buckets_of_A7, buckets_of_A8, buckets_of_A9, buckets_of_A10, buckets_of_A11, buckets_of_A12, buckets_of_A13, buckets_of_A14, buckets_of_A15, buckets_of_A16};

u_int32_t buckets_of_B1[BUCKETS]        = {0};
u_int32_t buckets_of_B2[BUCKETS/2]      = {0};
u_int32_t buckets_of_B3[BUCKETS/4]      = {0};
u_int32_t buckets_of_B4[BUCKETS/8]      = {0};
u_int32_t buckets_of_B5[BUCKETS/16]     = {0};
u_int32_t buckets_of_B6[BUCKETS/32]     = {0};
u_int32_t buckets_of_B7[BUCKETS/64]     = {0};
u_int32_t buckets_of_B8[BUCKETS/128]    = {0};
u_int32_t buckets_of_B9[BUCKETS/256]    = {0};
u_int32_t buckets_of_B10[BUCKETS/512]   = {0};
u_int32_t buckets_of_B11[BUCKETS/1024]  = {0};
u_int32_t buckets_of_B12[BUCKETS/2048]  = {0};
u_int32_t buckets_of_B13[BUCKETS/4096]  = {0};
u_int32_t buckets_of_B14[BUCKETS/8192]  = {0};
u_int32_t buckets_of_B15[BUCKETS/16384] = {0};
u_int32_t buckets_of_B16[BUCKETS/32768] = {0};
//Total Memory for buckets_of_B: ~1 MB

void *buckets_of_B[16] = {buckets_of_B1, buckets_of_B2, buckets_of_B3, buckets_of_B4, buckets_of_B5, buckets_of_B6, buckets_of_B7, buckets_of_B8, buckets_of_B9, buckets_of_B10, buckets_of_B11, buckets_of_B12, buckets_of_B13, buckets_of_B14, buckets_of_B15, buckets_of_B16};

u_int32_t init_size = 0;
u_int32_t curr_size = 0;
u_int32_t multiplier = SAMPLING_CORRECTION;
int max_value = MAX_VALUE;
int bin_size = BIN_SIZE;
int bucket_size = BUCKET_SIZE;

// Total Memory for data structure: 1,003 + 0.99 + 0.99 = 2.97 MB

void CEEngine::insertTuple(const std::vector<int>& tuple) {
    u_int32_t A = tuple[0];
    u_int32_t B = tuple[1];

    int index_a, index_b, size;
    
    size = BINS;
    for(int i = 0; i < 8; i++) {
        index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
        index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
        ((u_int32_t(*)[size])histogram[i])[index_a][index_b]++;
        size /= 2;
    }

    size = BUCKETS;
    for (int i = 0; i < 16; i++) {
        index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
        index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
        ((u_int32_t*)buckets_of_A[i])[index_a]++;
        ((u_int32_t*)buckets_of_B[i])[index_b]++;
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
    for (int i = 0; i < 16; i++){
        index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
        index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
        if( ((u_int32_t*)buckets_of_A[i])[index_a]) ((u_int32_t*)buckets_of_A[i])[index_a]--;
        if( ((u_int32_t*)buckets_of_B[i])[index_b]) ((u_int32_t*)buckets_of_B[i])[index_b]--;
        size /= 2;
    }
    
    if(curr_size){
        curr_size--;
        multiplier = (double) curr_size / (init_size*SAMPLING_RATE + (curr_size - init_size));
    }
}

int CEEngine::query(const std::vector<CompareExpression>& quals) {
    if (quals.size() == 1) {
        
        // A,B = x
        if (quals[0].compareOp == EQUAL) {
            //Probabilistic (proven best on tests)
            int estimation = quals[0].columnIdx == 0 ? ((float)(buckets_of_A1[quals[0].value/bucket_size])/bucket_size)*SAMPLING_CORRECTION : ((float)(buckets_of_B1[quals[0].value/bucket_size])/bucket_size)*SAMPLING_CORRECTION;
            return estimation;
        }
        
        // A, B > x
        if (quals[0].compareOp == GREATER) {
            u_int32_t value = quals[0].value;
            u_int32_t total_count = 0;
            int size = BUCKETS;
            int index = value/bucket_size+1 < size ? value/bucket_size+1 : size-1;

            // A > x
            if (quals[0].columnIdx == 0) {
                for(int i=0; i < 15; i++) {
                    if(index % 2 == 1)
                        total_count += ((u_int32_t*)buckets_of_A[i])[index++];
                    index /= 2;
                }
                for(int i = index;i<4;i++)
                    total_count += ((u_int32_t*)buckets_of_A[15])[i];

            // B > X
            } else {
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
            //Case 1: (A = x AND A = y) or (B = x AND B = y)
            #ifdef DUPLICATE_COLUMNS
            if(quals[0].columnIdx == quals[1].columnIdx) {
                int x = quals[0].value;
                int y = quals[1].value;
                if(x == y)
                    return quals[0].columnIdx == 0 ? (buckets_of_A1[x/bucket_size]/bucket_size)*SAMPLING_CORRECTION : (buckets_of_B1[y/bucket_size]/bucket_size)*SAMPLING_CORRECTION;
                return 0;
            }
            #endif

            //Case 2: (A = x AND B = y)
            //Proven best approximation, so far
            return 0;
        }

        // A = x AND B > y
        if (quals[0].compareOp == EQUAL && quals[1].compareOp == GREATER) {
            //Case 1: (A = x AND A > y) or (B = x AND B > y)
            #ifdef DUPLICATE_COLUMNS
            if (quals[0].columnIdx == quals[1].columnIdx) {
                int x = quals[0].value;
                int y = quals[1].value;
                if (y >= x)
                    return 0;
                return quals[0].columnIdx == 0 ? (buckets_of_A1[quals[0].value/bucket_size]/bucket_size)*SAMPLING_CORRECTION : (buckets_of_B1[quals[0].value/bucket_size]/bucket_size)*SAMPLING_CORRECTION;
            }
            #endif

            //Case 2: (A = x AND B > y)
            return curr_size/max_value;
        }

        // A > x AND B = y
        if (quals[0].compareOp == GREATER && quals[1].compareOp == EQUAL) {

            //Case 1: (A > x AND A = y) or (B > x AND B = y)
            #ifdef DUPLICATE_COLUMNS
            if (quals[0].columnIdx == quals[1].columnIdx) {
                int x = quals[0].value;
                int y = quals[1].value;
                if (y <= x)
                    return 0;
                return quals[1].columnIdx == 0 ? (buckets_of_A1[quals[1].value/bucket_size]/bucket_size)*SAMPLING_CORRECTION : (buckets_of_B1[quals[1].value/bucket_size]/bucket_size)*SAMPLING_CORRECTION;
            }
            #endif

            //Case 2: (A > x AND B = y)
            return curr_size/max_value;
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

            //Case 1: (A > x AND A > y) or (B > x AND B > y)
            #ifdef DUPLICATE_COLUMNS
            if(quals[0].columnIdx == quals[1].columnIdx) {

                size = BUCKETS;
                int value = quals[0].value < quals[1].value ? quals[0].value : quals[1].value;
                int index = value/bucket_size+1 < size ? value/bucket_size+1 : size-1;

                if(quals[0].columnIdx == 0){
                    for(int i=0; i < 15; i++) {
                        if(index % 2 == 1)
                            total_count += ((u_int32_t*)buckets_of_A[i])[index++];
                        index /= 2;
                    }
                    for(int i = index;i<4;i++)
                        total_count += ((u_int32_t*)buckets_of_A[15])[i];
                }
                else{
                    for(int i=0; i < 15; i++) {
                        if(index % 2 == 1)
                            total_count += ((u_int32_t*)buckets_of_B[i])[index++];
                        index /= 2;
                    }
                    for(int i = index;i<4;i++)
                        total_count += ((u_int32_t*)buckets_of_B[15])[i];
                }
                return total_count*multiplier;
            }
            #endif

            //Case 2: (A > x AND B > y)

            //Logarithmic search in histograms
            size = BINS;

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

void CEEngine::prepare() {}

CEEngine::CEEngine(int num, DataExecuter *dataExecuter) {
    init_size = num;
    curr_size = num;
    this->dataExecuter = dataExecuter;
    
    std::vector<std::vector<int>> data;
    u_int32_t A,B;

    //Adjust bin and bucket sizes dynamically, based on max value
    bin_size = max_value/BINS;
    bucket_size = max_value/BUCKETS;

    for (int i = 0; i < num; i+=OFFSET) {
        data.clear();
        dataExecuter->readTuples(i, OFFSET*SAMPLING_RATE, data);
        for(int j = 0; j < OFFSET*SAMPLING_RATE; j++) {

            A = data[j][0];
            B = data[j][1];

            int index_a, index_b, size;

            size = BINS;
            for(int i = 0; i < 8; i++) {    
                index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
                index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
                ((u_int32_t(*)[size])histogram[i])[index_a][index_b]++;
                size /= 2;
            }

            size = BUCKETS;
            for (int i = 0; i < 16; i++) {
                index_a = A/(max_value/size) < size ? A/(max_value/size) : size-1;
                index_b = B/(max_value/size) < size ? B/(max_value/size) : size-1;
                ((u_int32_t*)buckets_of_A[i])[index_a]++;
                ((u_int32_t*)buckets_of_B[i])[index_b]++;
                size /= 2;
            }
        }
    }
    data.clear();
}