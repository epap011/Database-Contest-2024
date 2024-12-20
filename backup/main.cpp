#include <CardinalityEstimation.h>
#include <executer/DataExecuterDemo.h>

int main(int argc, char *argv[])
{
    int initSize = 40000000; // Initial data size.
    int opSize = 20000;    // Number of operations.
    double SingleEqualScore = 0;
    double SingleGreaterScore = 0;
    double DoubleEqualScore = 0;
    double DoubleGreaterScore = 0;
    double DoubleGreaterEqualScore = 0;
    double score = 0;
    int SingleEqualCnt = 0;
    int SingleGreaterCnt = 0;
    int DoubleEqualCnt = 0;
    int DoubleGreaterCnt = 0;
    int DoubleGreaterEqualCnt = 0;
    int cnt = 0;
    std::string message = "Equalities CMS tests - [40mil 25k]\nAverage score: ";
    
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
         << duration1.count() << " milliseconds" << std::endl << std::endl;

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
            if (action.quals.size() == 2 && action.quals[0].compareOp == EQUAL && action.quals[1].compareOp == EQUAL)
                std::cout<< "A=x AND B=y: estimate->" << ans << " real->";
            double realAns = dataExecuter.answer(ans);
            score += realAns;
            cnt++;
            if (action.quals.size() == 1 && action.quals[0].compareOp == EQUAL){
                SingleEqualScore += realAns;
                SingleEqualCnt++;
            }
            else if (action.quals.size() == 1 && action.quals[0].compareOp == GREATER){
                SingleGreaterScore += realAns;
                SingleGreaterCnt++;
            }
            else if (action.quals.size() == 2 && action.quals[0].compareOp == EQUAL && action.quals[1].compareOp == EQUAL){
                DoubleEqualScore += realAns;
                DoubleEqualCnt++;
            }
            else if (action.quals.size() == 2 && action.quals[0].compareOp == GREATER && action.quals[1].compareOp == GREATER){
                DoubleGreaterScore += realAns;
                DoubleGreaterCnt++;
            }
            else if (action.quals.size() == 2 && ((action.quals[0].compareOp == GREATER && action.quals[1].compareOp == EQUAL) || (action.quals[0].compareOp == EQUAL && action.quals[1].compareOp == GREATER))){
                DoubleGreaterEqualScore += realAns;
                DoubleGreaterEqualCnt++;
            }
        }
        action = dataExecuter.getNextAction();
    }
    std::cout << message << score / cnt << std::endl << std::endl <<"Score analysis"<< std::endl << std::endl;
    std::cout << "(25%)   {A,B} = X: " << SingleEqualScore / SingleEqualCnt << std::endl;
    std::cout << "(25%)   {A,B} > X: " << SingleGreaterScore / SingleGreaterCnt << std::endl;
    std::cout << "(12.5%) {A,B} = X, {B,A} = Y: " << DoubleEqualScore / DoubleEqualCnt << std::endl;
    std::cout << "(12.5%) {A,B} > X, {B,A} > Y: " << DoubleGreaterScore / DoubleGreaterCnt << std::endl;
    std::cout << "(%25)   {A,B} > X, {B,A} = Y: " << DoubleGreaterEqualScore / DoubleGreaterEqualCnt << std::endl;
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(stop2 - start2);
    std::cout << std::endl <<"Time taken by while-loop: "
         << duration2.count() << " milliseconds" << std::endl;
}
