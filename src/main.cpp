#include <CardinalityEstimation.h>
#include <executer/DataExecuterDemo.h>

int main(int argc, char *argv[])
{
    int initSize = 1000000; // Initial data size.
    int opSize = 20000;    // Number of operations.
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
    auto stop1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(stop1 - start1);
    std::cout << "Time taken by CEEngine: "
         << duration1.count() << " milliseconds" << std::endl;

    Action action = dataExecuter.getNextAction();

    auto start2 = std::chrono::high_resolution_clock::now();
    while (action.actionType != NONE) {
        ceEngine.prepare();
        if (action.actionType == INSERT) {
            ceEngine.insertTuple(action.actionTuple);
        } else if (action.actionType == DELETE) {
           ceEngine.deleteTuple(action.actionTuple, action.tupleId);
        } else if (action.actionType == QUERY) {
            int ans = ceEngine.query(action.quals);
            double realAns = dataExecuter.answer(ans);
            score += realAns;
            cnt++;
        }
        action = dataExecuter.getNextAction();
    }
    std::cout << score / cnt << std::endl;
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(stop2 - start2);
    std::cout << "Time taken by while-loop: "
         << duration2.count() << " milliseconds" << std::endl;
}