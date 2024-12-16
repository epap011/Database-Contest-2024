//
// Demo data generator for local debugging. You can implement your own data generator for debugging based on this class.
//
#define MAX_VALUE 20000000

#include <executer/DataExecuterDemo.h>

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
    int x = (rand()) % (end+1);
    while (vis[x]) {
        x = (rand()) % (end+1);
    }
    vis[x] = true;
    return x;
}

void DataExecuterDemo::readTuples(int start, int offset, std::vector<std::vector<int>> &vec)
{
    for (int i = start; i < start + offset; ++i) {
        if (i > end)
            break;
        if (!vis[i]) {
            vec.push_back(set[i]);
        }
    }
    // std::cout << "Tuple["<< start <<"]: " << set[start][0] << " " << set[start][1] << std::endl;
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
        int columnIdx = rand() % 2;
        CompareExpression expr1 = {columnIdx, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
        action.quals.push_back(expr1);
        CompareExpression expr2 = {columnIdx ? 0 : 1, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
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
