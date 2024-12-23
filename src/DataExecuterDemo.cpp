//
// Demo data generator for local debugging. You can implement your own data generator for debugging based on this class.
//
#define MAX_VALUE 20000000

#include <executer/DataExecuterDemo.h>

//DEBUG
int check = 0;
int s_check = 0;

std::unordered_map<int, bool> vis;
DataExecuterDemo::DataExecuterDemo(int end, int count) : DataExecuter()
{
    this->end = end;
    this->count = count;
    //Generate first 9mil manually (9000 values 1000 times each)
    for (int i = 0; i < 9000; ++i) {
        for(int j = 0; j < 1000; j++){
            std::vector<int> tuple;
            tuple.push_back(i*100);
            tuple.push_back(i*100);
            set.push_back(tuple);
        }
    }
    for (int i = 9000000; i <= end; ++i) {
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
    else if (count>19975){
        action.actionType = QUERY;
        int columnIdx = rand() % 2;
        if(rand()%2){
            CompareExpression expr1 = {columnIdx, CompareOp(EQUAL), check};
            action.quals.push_back(expr1);
            CompareExpression expr2 = {columnIdx ? 0 : 1, CompareOp(EQUAL), check};
            action.quals.push_back(expr2);
            check+=100;
        }
        else{
            CompareExpression expr1 = {columnIdx, CompareOp(EQUAL), rand() % MAX_VALUE + 1};
            action.quals.push_back(expr1);
            CompareExpression expr2 = {columnIdx ? 0 : 1, CompareOp(EQUAL), rand() % MAX_VALUE + 1};
            action.quals.push_back(expr2);
        }
    }
    else if (count>19925){
        action.actionType = QUERY;
        CompareOp op1 = rand() % 2 ? EQUAL : GREATER;
        CompareOp op2 = op1 == EQUAL ? GREATER : EQUAL;
        int column = rand() % 2;
        CompareExpression expr = {column, op1, (op1 == EQUAL) ? s_check : rand() % MAX_VALUE + 1};
        action.quals.push_back(expr);
        CompareExpression expr2 = {column ? 0 : 1, op2, (op2 == EQUAL) ? s_check : rand() % MAX_VALUE + 1};
        action.quals.push_back(expr2);
        s_check+=100;
    }
    else if (count % 100 == 99) {
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
    // DEBUG PRINTS (also in main.cpp)
    // if (curAction.quals.size() == 2 && curAction.quals[0].compareOp == EQUAL && curAction.quals[1].compareOp == EQUAL)
    //     std::cout << cnt << std::endl;
    // else if (curAction.quals.size() == 2 && ((curAction.quals[0].compareOp == EQUAL && curAction.quals[1].compareOp == GREATER) || (curAction.quals[0].compareOp == GREATER && curAction.quals[1].compareOp == EQUAL)))
    //     std::cout << cnt << std::endl;
    // else if (curAction.quals.size()==1 && curAction.quals[0].compareOp == EQUAL)
    //     std::cout << cnt << std::endl;
    return error;
};
