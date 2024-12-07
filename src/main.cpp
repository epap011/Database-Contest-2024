#include <CardinalityEstimation.h>
#include <executer/DataExecuterDemo.h>
#include <fstream>

int main(int argc, char *argv[])
{
    int initSize = 20000000; // Initial data size.
    int opSize = 10000; // Number of operations.
    double score = 0;
    int cnt = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    DataExecuterDemo dataExecuter(initSize - 1, opSize);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Time taken by DataExecuterDemo: "
         << duration.count() << " milliseconds" << std::endl;

    auto start1 = std::chrono::high_resolution_clock::now();
    CEEngine ceEngine(initSize, &dataExecuter);

    // dataExecuter.HardcodedDataGenerator("tuples.txt", "queries.txt", "results.txt");
    auto stop1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(stop1 - start1);
    // std::cout << "Time taken by Data Generator: "
        //  << duration1.count() << " milliseconds" << std::endl;
    std::cout << "Time taken by CEengine: "
          << duration1.count() << " milliseconds" << std::endl;


    auto start2 = std::chrono::high_resolution_clock::now();

    std::ifstream queriesFile("queries.txt");
    std::ifstream resultsFile("results.txt");

    Action queryAction;
    queryAction.actionType = QUERY;
    int columnIdx, compareOp, value;
    CompareExpression expr;
    while(queriesFile.peek() != EOF) {
        queryAction.quals.clear();
        queriesFile.read((char*)&columnIdx, sizeof(int));
        queriesFile.read((char*)&compareOp, sizeof(int));
        queriesFile.read((char*)&value, sizeof(int));
        expr = {columnIdx, CompareOp(compareOp), value};
        queryAction.quals.push_back(expr);
        queriesFile.read((char*)&columnIdx, sizeof(int));
        queriesFile.read((char*)&compareOp, sizeof(int));
        queriesFile.read((char*)&value, sizeof(int));
        if(columnIdx != -1) {
            expr = {columnIdx, CompareOp(compareOp), value};
            queryAction.quals.push_back(expr);
        }
        int real,ans;
        resultsFile.read((char*)&real, sizeof(int));
        // if (queryAction.quals[0].compareOp == 0 || queryAction.quals[1].compareOp == 0) continue;
        ans = ceEngine.query(queryAction.quals);
        score += fabs(std::log((ans + 1) * 1.0 / (real + 1)));
        cnt++;
    }
    queriesFile.close();
    resultsFile.close();

    
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(stop2 - start2);

    std::cout << "Time taken by CEengine Estimator: "
          << duration2.count() << " milliseconds" << std::endl;
    std::cout << "Score: " << score / cnt << std::endl;

    // Action action = dataExecuter.getNextAction();

    // auto start2 = std::chrono::high_resolution_clock::now();
    // while (action.actionType != NONE) {
    //     ceEngine.prepare();
    //     if (action.actionType == INSERT) {
    //         ceEngine.insertTuple(action.actionTuple);
    //     //} else if (action.actionType == DELETE) {
    //     //    ceEngine.deleteTuple(action.actionTuple, action.tupleId);
    //     } else if (action.actionType == QUERY) {
    //         int ans = ceEngine.query(action.quals);
    //         score += dataExecuter.answer(ans);
    //         cnt++;
    //     }
    //     action = dataExecuter.getNextAction();
    // }
    // std::cout << score / cnt << std::endl;
    // auto stop2 = std::chrono::high_resolution_clock::now();
    // auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(stop2 - start2);
    // std::cout << "Time taken by while-loop: "
    //      << duration2.count() << " milliseconds" << std::endl;
}