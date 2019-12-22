/* 注意事项
现在还差dijkstra的算法，函数描述如下
vector<int> dijkstra(int start, int end)
该函数取两个中转站的物理地址
返回一个vector<int>
vector[0]记录start到end的最短时间，包括中转时间
[1]-[back]依次是路径上结点物理编号如start,...,end
特殊情况：
    start或end是-1，此时仅返回含一个元素：1000的vector即可
关于graph的数据结构
    每个中转站有一个物理地址与虚拟地址
    物理地址是从所有站角度的编号，
    虚拟地址是从仅从中转站的编号，
    他们直接可以通过vector<int>transfer来转换
    transfer下标是虚拟地址，对应值是物理地址

*/
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#define MAX_LENGTH 1000
#define LINE_NUMBER 15
using namespace std;
//using std::cout//endl
//using std::vector
//using std::string

//实际上我们不输出线的名字
//data.txt里面
//3.2->10
//14.2->11
//广佛->12
//apm不考虑
//21->15
typedef int lineIndex;
typedef int time;
typedef vector<int> row;
typedef vector<row> Graph;
typedef int Vertex;
typedef struct station {
    string name;
    int index;
    int lineNumber;
    vector<pair<lineIndex, time>> lineAndTime;
    station(string s = "", int i = 0, int l = -1)
    : name{s}, index{i}, lineNumber{l} {
        lineAndTime = vector<pair<lineIndex, time>>{};
    }
} station;
//单例模式，没啥用，只是对全局变量的一个封装
//特点是有一个getInstance()函数
//构造函数是private的
class Singleton {
public:
    ~Singleton() {delete instance;}
    static Singleton* getInstance();
    vector<int> findPath(string&, string&);
    void printPath(vector<int>&);
    void test();//用来我自己瞎测试用,输出一些数据到data.txt
private:
    static Singleton* instance;
    vector<vector<int>> allLines;
    //graph里一定要是0-40代表换乘站
    vector<row> graph; 
    //第0个元素无效
    vector<station> stations;
    vector<int> transfers;
    map<string, int> nameToIndex;
    vector<int> dijkstra(int start, int end) {}
    Singleton();
    bool sameLine(int, int);
    void getPreNext(int, int&, int&, int&, int&);
};
void Singleton::test() {
    ofstream out("out.txt");
    out << "\n-----allLines----" << endl;
    for (auto x : allLines) {
        for (auto y : x) {
            out << y << ' ';
        }
        out << endl;
    }
    out << "\n------nameToIndex-----" << endl;
    for (auto x : nameToIndex) {
        out << x.first << "->" << x.second << endl;
    }
    out << "\n------graph-----" << endl;
    for (auto x : allLines) {
        for (auto y : x) {
            out << y << ' ';
        }
        out << endl;
    }
    out << "\n------transfers-----" << endl;
    for (auto x : transfers) {
        out << stations[x].name << endl;
    }
    out << "\n------stations-----" << endl;
    for (auto x : stations) {
        out << x.name << ":" << x.index << endl;
        for (auto y : x.lineAndTime) {
            out << "line" << y.first+1 << "：" << y.second << endl;
        }
        out << "-------" << endl;
    }
    out.close();
}
bool find(vector<int>& v, int n) {
    for (auto x : v)
        if (n == x) 
            return true;
    return false;
}
//读数据时每遇到一个换乘站，将其插入到vector第一个
//数据记录格式，线编号，站名，到上一站时间，若为0，代表是起始站
//站编号从1开始编号，线从0开始编号
//站编号为0的表示未编号
//data中的数据从1号线顺序排序
//存在中转3条线的站，于是判断条件不要写 ==2，用 !=1
Singleton::Singleton() {
    ifstream input("data.txt");
    if (!input.is_open()) return;
    int currentTime, currentNumber = 1;
    vector<int> emptyL;

    stations.push_back(station{});//无意义
    allLines = vector<vector<int>>{LINE_NUMBER, emptyL};
    //读入所有数据
    while (1) {
        int index, time;
        string name;
        input >> index >> name >> time;
        if (input.eof()) break;
        if (time == 0) currentTime = 0;
        currentTime += time;
        --index;
        //处理第一次出现的站点
        if (nameToIndex[name] == 0) {
            station tempStation(name, currentNumber, 1);
            tempStation.lineAndTime.push_back(make_pair(index, currentTime));
            stations.push_back(tempStation);
            nameToIndex[name] = currentNumber;
            allLines[index].push_back(currentNumber);
            currentNumber++; 
        } else {//处理换乘站
            //需要在station资料中lineNumber+1,添加相关线与时间信息
            //还需要将换乘站加入到编号转换vector，便于构建图
            int stationIndex = nameToIndex[name];
            stations[stationIndex].lineNumber++;
            stations[stationIndex].lineAndTime.push_back(make_pair(index, currentTime));
            allLines[index].push_back(currentNumber);
            if (!find(transfers, stationIndex))
                transfers.push_back(stationIndex);
        }
    }
    input.close();
    //构建换乘站构成的图
    int n = transfers.size();
    graph = vector<row>{n, row{n, MAX_LENGTH}};
    for (int i = 0; i < n; i++) {
        int realIndex = transfers[i];
        //cout << "\n----zhengchang----" << i << endl;
        for (int j = 0; j < stations[realIndex].lineNumber; j++) {
            //由类似getprev的函数，计算出距离，若有，在矩阵相应位置赋值
            int lineIndex = stations[realIndex].lineAndTime[0].first;
            int flag = 0;
            int numPre, numNext, dPre, dNext;
            numPre = numNext = 0;
            dPre = dNext = MAX_LENGTH;
            for (int k = 0; k < allLines[lineIndex].size(); k++) {
                int currentStation = allLines[lineIndex][k];
                if (currentStation == realIndex)
                    flag = 1;
                if (stations[currentStation].lineNumber == 2) {
                    int currentDistance;
                    for (auto x : stations[currentStation].lineAndTime)
                        if (x.first == lineIndex)
                            currentDistance = x.second;
                    if (flag) {
                        numPre = currentStation;
                        dPre = abs(stations[realIndex].lineAndTime[0].second
                        -currentDistance);
                    } 
                    else {
                        numNext = currentStation;
                        dNext = abs(stations[realIndex].lineAndTime[0].second
                        -currentDistance);
                        break;
                    }
                } 
            }
            int k;
            for (k = 0; k < transfers.size(); k++)
                if (transfers[k] == numPre)
                    break;
            if (numPre != 0) graph[i][k] = dPre;//虚拟地址
            for (k = 0; k < transfers.size(); k++)
                if (transfers[k] == numNext)
                    break;
            if (numNext != 0) graph[i][k] = dNext;
        }
    }
}
Singleton* Singleton::instance = nullptr;
bool Singleton::sameLine(int num1, int num2) {
    for (int i = 0; i < stations[num1].lineNumber; i++) {
        for (int j = 0; j < stations[num2].lineNumber; i++) {
            if (stations[num1].lineAndTime[0].first 
            == stations[num2].lineAndTime[0].first)
                return true;
        }
    }
    return false;
}
//返回：通过引用传递，前一个中转站编号和到其距离，后一个中转站编号和到期距离
void Singleton::getPreNext(int num, int& numPre, int& numNext, int& dPre, int& dNext) {
    numNext = numPre = -1;
    //加入num本身就是中转站，前一个和后一个都视为本身
    if (stations[num].lineNumber != 1) {//lineNumber不为1的都是换乘站
        numPre = numNext = num;
        dPre = dNext = 0;
    } else {
        int flag = 0;
        int lineIndex = stations[num].lineAndTime[0].first;
        for (int i = 0; i < allLines[lineIndex].size(); i++) {
            int currentStation = allLines[lineIndex][i];
            if (currentStation == num)
                flag = 1;
            if (stations[currentStation].lineNumber != 1) {
                int currentDistance;
                    for (auto x : stations[currentStation].lineAndTime)
                        if (x.first == lineIndex)
                            currentDistance = x.second;
                    if (flag) {
                        numPre = currentStation;
                        dPre = abs(stations[num].lineAndTime[0].second-
                        currentDistance);
                    } 
                    else {
                        numNext = currentStation;
                        dNext = abs(stations[num].lineAndTime[0].second-
                        currentDistance);
                        break;
                    }
            } 
        }
    }
}
//用来输出，不用管
void Singleton::printPath(vector<int>& v) {
    cout << stations[v[0]].name << " -> ";
    for (int i = 1; i < v.size()-2; i++)
        cout << stations[v[i]].name << "\n↑换乘↓\n" 
        << stations[v[i]].name << " -> ";
    cout << stations[v.back()].name << endl;
}
vector<int> Singleton::findPath(string& s1, string& s2) {
    int num1 = nameToIndex[s1];
    int num2 = nameToIndex[s2];
    if (sameLine(num1, num2)) {
        vector<int> temp = {num1, num2};
        return temp;
    }
    int num1Pre, num2Pre, num1Next, num2Next;
    int num1PreD, num2PreD, num1NextD, num2NextD;
    
    getPreNext(num1, num1Pre, num1Next, num1PreD, num1NextD);
    getPreNext(num2, num2Pre, num2Next, num2PreD, num2NextD);
    //vector第一位存距离
    //dijskstra时最好单独处理好num1Pre为-1的情况！！！
    //重要！！！此时vector请仅返回1000 (MAX_LENGTH)
    //这里的num1Pre-num2Next均是站的物理编号
    //进行dijkstra时要映射成虚拟编号进行操作
    //dijkstra内部要算上换乘时间！
    vector<int> l1 = dijkstra(num1Pre, num2Pre);
    vector<int> l2 = dijkstra(num1Pre, num2Next);
    vector<int> l3 = dijkstra(num1Next, num2Pre);
    vector<int> l4 = dijkstra(num1Next, num2Next);
    vector<int> distances(4, 0);
    distances[0] = num1PreD + l1[0] + num2PreD;
    distances[1] = num1PreD + l2[0] + num2NextD;
    distances[2] = num1NextD + l3[0] + num2PreD;
    distances[3] = num1NextD + l4[0] + num2NextD;
    int min = 0;
    for (int i = 1; i < 4; i++)
        if (distances[min] > distances[i]) 
            min = i;
    vector<int> temp;
    switch(min) {
        case 0:
            temp = l1;
            break;
        case 1:
            temp = l2;
            break;
        case 2:
            temp = l3;
            break;
        case 3:
            temp = l4;
            break;
    }
    temp[0] = num1;
    temp.push_back(num2);
    return temp;
}
//单例模式，没啥用，只是对全局变量的一个封装

Singleton* Singleton::getInstance() {
    if (instance == nullptr)
        instance = new Singleton();
    return instance;
}