//
// Demo data generator for local debugging. You can implement your own data generator for debugging based on this class.
//
#define MAX_VALUE 20000000

#include <executer/DataExecuterDemo.h>
#include <CardinalityEstimation.h>
#include <fstream>

std::unordered_map<int, bool> vis;
DataExecuterDemo::DataExecuterDemo(int end, int count) : DataExecuter()
{
    this->end = end;
    this->count = count;
    for (int i = 0; i <= end; ++i) {
        std::vector<int> tuple;
        tuple.push_back(rand() % MAX_VALUE + 1);
        tuple.push_back(rand() % MAX_VALUE + 1);
        set.push_back(tuple);
    }
}

std::vector<int> DataExecuterDemo::generateInsert()
{
    std::vector<int> tuple;
    tuple.push_back(rand() % MAX_VALUE + 1);
    tuple.push_back(rand() % MAX_VALUE + 1);
    set.push_back(tuple);
    end++;
    return tuple;
}

int DataExecuterDemo::generateDelete()
{
    int x = (rand()) % end;
    while (vis[x]) {
        x = (rand()) % end;
    }
    vis[x] = true;
    return x;
}

void DataExecuterDemo::readTuples(int start, int offset, std::vector<std::vector<int>> &vec)
{
    for (int i = start; i < start + offset; ++i) {
        if (!vis[i]) {
            vec.push_back(set[i]);
        }
    }
    return;
};

Action DataExecuterDemo::getNextAction()
{
    Action action;
    if (count == 0) {
        action.actionType = NONE;
        return action;
    }
    if (count % 100 == 99) {
        action.actionType = QUERY;
        CompareExpression expr = {rand() % 2, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
        action.quals.push_back(expr);
    } else if (count % 100 == 98){
        action.actionType = QUERY;
        CompareExpression expr1 = {rand() % 2, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
        action.quals.push_back(expr1);
        CompareExpression expr2 = {rand() % 2, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
        action.quals.push_back(expr2);
    } else if (count % 100 < 90) {
        action.actionType = INSERT;
        action.actionTuple = generateInsert();
    } else {
        action.actionType = DELETE;
        action.tupleId = generateDelete();
        action.actionTuple = set[action.tupleId];
    }
    count--;
    curAction = action;
    return action;
};

double DataExecuterDemo::answer(int ans)
{
    int cnt = 0;
    for (int i = 0; i <= end; ++i) {
        if (vis[i])
            continue;
        bool flag = true;
        for (int j = 0; j < curAction.quals.size(); ++j) {
            CompareExpression &expr = curAction.quals[j];
            if (expr.compareOp == GREATER && set[i][expr.columnIdx] <= expr.value) {
                flag = false;
                break;
            }
            if (expr.compareOp == EQUAL && set[i][expr.columnIdx] != expr.value) {
                flag = false;
                break;
            }
        }
        if (flag)
            cnt++;
    }
    double error = fabs(std::log((ans + 1) * 1.0 / (cnt + 1)));
    // if(curAction.quals.size() == 2)
    //     printf("Real: %d\t\tEstimate: %d\n",cnt,ans);
    return error;
};

int DataExecuterDemo::answerCnt(Action queryAction)
{
    int cnt = 0;
    for (int i = 0; i <= end; ++i) {
        bool flag = true;
        for (int j = 0; j < queryAction.quals.size(); ++j) {
            CompareExpression &expr = queryAction.quals[j];
            if (expr.compareOp == GREATER && set[i][expr.columnIdx] <= expr.value) {
                flag = false;
                break;
            }
            if (expr.compareOp == EQUAL && set[i][expr.columnIdx] != expr.value) {
                flag = false;
                break;
            }
        }
        if (flag)
            cnt++;
    }
    return cnt;
};
//print base and results to file. get file arguments
void DataExecuterDemo::HardcodedDataGenerator(std::string tuples, std::string queries, std::string results) {
    std::ofstream tuplesFile(tuples);
    std::ofstream queriesFile(queries);
    std::ofstream resultsFile(results);

    //generate tuples
    for (int i = 0; i <= end; ++i) {
        tuplesFile.write((char*)&set[i][0], sizeof(int));
        tuplesFile.write((char*)&set[i][1], sizeof(int));
    }
    //generate queries, along with results
    Action queryAction;
    int invalid = -1;
    for (int i = 0; i < count; ++i) {
        queryAction.quals.clear();
        //produce queries like getnextaction
        if(i % 2){
            queryAction.actionType = QUERY;
            CompareExpression expr = {rand() % 2, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
            queryAction.quals.push_back(expr);
            queriesFile.write((char*)&queryAction.quals[0].columnIdx, sizeof(int));
            queriesFile.write((char*)&queryAction.quals[0].compareOp, sizeof(int));
            queriesFile.write((char*)&queryAction.quals[0].value, sizeof(int));
            queriesFile.write((char*)&invalid, sizeof(int));
            queriesFile.write((char*)&invalid, sizeof(int));
            queriesFile.write((char*)&invalid, sizeof(int));
        } else {
            queryAction.actionType = QUERY;
            CompareExpression expr1 = {rand() % 2, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
            queryAction.quals.push_back(expr1);
            CompareExpression expr2 = {rand() % 2, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
            queryAction.quals.push_back(expr2);
            queriesFile.write((char*)&queryAction.quals[0].columnIdx, sizeof(int));
            queriesFile.write((char*)&queryAction.quals[0].compareOp, sizeof(int));
            queriesFile.write((char*)&queryAction.quals[0].value, sizeof(int));
            queriesFile.write((char*)&queryAction.quals[1].columnIdx, sizeof(int));
            queriesFile.write((char*)&queryAction.quals[1].compareOp, sizeof(int));
            queriesFile.write((char*)&queryAction.quals[1].value, sizeof(int));
        }
        int result = answerCnt(queryAction);
        //printf("Answer%d: %d\t\-> %d\n",i, result);
        resultsFile.write((char*)&result, sizeof(int));
    }
    tuplesFile.close();
    queriesFile.close();
    resultsFile.close();
};

void DataExecuterDemo::readHardTuples(int start, int offset, std::string tuples, std::vector<std::vector<int>> &vec) {
    std::ifstream tuplesFile(tuples);
    tuplesFile.seekg(start * sizeof(int));
    for (int i = start; i < start + offset; ++i) {
        if (tuplesFile.peek() == EOF) {
            break;
        }
        int x;
        tuplesFile.read((char*)&x, sizeof(int));
        std::vector<int> tuple;
        tuple.push_back(x);
        tuplesFile.read((char*)&x, sizeof(int));
        tuple.push_back(x);
        vec.push_back(tuple);
    }
    tuplesFile.close();
};