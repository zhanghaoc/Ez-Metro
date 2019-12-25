/*  
    关于graph的数据结构
    每个中转站有一个物理地址与虚拟地址
    物理地址是从所有站角度的编号，
    虚拟地址是从仅从中转站的编号，
    它们直接可以通过vector<int>virtualToPhysical来转换
    virtualToPhysical下标是虚拟地址，对应值是物理地址
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#define MAX_TIME 900 //定义最长时间，即任意简单路径的时间都不会超过该时间
#define LINE_NUMBER 15 //总共有十五条线
//由于换乘时间主要与不同线路之间的时间差和乘车时刻的发车频率有关
//故取换乘时间的均值，约为三分钟
#define TRANSFER_TIME 3
using namespace std;

//为了方便起见，本程序做了一些特殊线路的处理
//三北线（三号线北延段）记为十号线
//十四号线（知识城）记为十一号线
//广佛线记为十二号线
//APM线不考虑
//二十一号线记为十五号线

bool find(vector<int>& v, int n){
    for (auto x : v)
        if (n == x) 
            return true;
    return false;
}

typedef int lineIndex; //线编号
typedef int Time; //时间单位：min
typedef vector<int> line; //地铁线
typedef vector<line> Graph; //多条地铁线组成图
typedef int Vertex; //顶点类型
typedef struct station {
    string name; //站名
    int index; //该站的编号，为了方便起见每个站都有自己的编号
    int lineNumber; //穿过该站的线路数，为1时为普通站台，大于1时为中转站
    vector<pair<lineIndex, Time> > lineAndTime; 
    //存储穿过该站的线路和到线路终点站的时间
    station(string s = "", int i = 0, int l = -1)
    : name{s}, index{i}, lineNumber{l} {
        lineAndTime = vector<pair<lineIndex, Time> >{};
    }
} station;

//地铁类采用单例模式的设计思想，故命名为singleton
class Singleton {
public:
    ~Singleton() {delete instance;}
    static Singleton* getInstance(); //获取实例
    vector<int> findPath(string&, string&); //输入地铁站，输出
    void printPath(vector<int>&); //打印线路    

private:
    static Singleton* instance;    
    Singleton();//构造函数，设置为private以保证只实例化一次

    vector<vector<int>> allLines; //按顺序存储每一条线路上的站编号    
    vector<line> graph; //存储中转站构成的图

    vector<station> stations; //存储所有站点    
    vector<int> virtualToPhysical; //将虚拟地址映射到实际地址
    map<string, int> nameToIndex; //站名到站编号的映射
    vector<int> dijkstra(int start, int end); //获取最快路径的函数，采用Dijkstra算法

    bool sameLine(int, int); //判断是否两个站在同一条线上
    bool sameLineThree(int, int, int);//判断三个站是否在同一线上

    //计算编号为num的站的最近的两个中转站编号和到其距离
    void getPreNext(int, int&, int&, int&, int&); 
    
};

Singleton* Singleton::instance = nullptr; //初始化为空指针


//读数据时每遇到一个换乘站，将其插入到vector第一个
//数据记录格式，线编号，站名，到上一站时间，若为0，代表是起始站
//站编号从1开始编号，线从0开始编号
//站编号为0的表示未编号

Singleton::Singleton() {
    ifstream input("data.txt");
    if (!input.is_open())                  
        return; //打开失败则返回                   

    int currentTime, currentNumber = 1;
    vector<int> emptyL;

    stations.push_back(station{}); //填充为0的元素
    
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
        } else {
            //处理换乘站
            //需要在station资料中lineNumber+1,添加相关线与时间信息
            //还需要将换乘站加入到编号转换vector，便于构建图
            int stationIndex = nameToIndex[name];
            stations[stationIndex].lineNumber++;
            stations[stationIndex].lineAndTime.push_back(make_pair(index, currentTime));
            allLines[index].push_back(stationIndex);
            if (!find(virtualToPhysical, stationIndex))
                virtualToPhysical.push_back(stationIndex);
        }

    }
    input.close();

    //构建换乘站构成的图
    int n = virtualToPhysical.size();
    graph = vector<line>(n, line(n, MAX_TIME));
    for (int i = 0; i < n; i++) {
        int realIndex = virtualToPhysical[i];        
        for (int j = 0; j < stations[realIndex].lineNumber; j++) {
            //由类似getprev的函数，计算出距离，若有，在矩阵相应位置赋值
            int lineIndex = stations[realIndex].lineAndTime[j].first;
            int flag = 0;
            int numPre, numNext, dPre, dNext;
            numPre = numNext = 0;
            dPre = dNext = MAX_TIME;
            for (int k = 0; k < allLines[lineIndex].size(); k++) {
                int currentStation = allLines[lineIndex][k];
                if (currentStation == realIndex) {
                    flag = 1;
                    continue;
                }
                if (stations[currentStation].lineNumber != 1) {
                    int currentDistance, realIndexDistance;
                    for (auto x : stations[currentStation].lineAndTime)
                        if (x.first == lineIndex)
                            currentDistance = x.second;
                    for (auto x : stations[realIndex].lineAndTime)
                        if (x.first == lineIndex)
                            realIndexDistance = x.second;
                    if (flag) {
                        numNext = currentStation;
                        dNext = abs(realIndexDistance-currentDistance);
                        break;
                    } else {
                        numPre = currentStation;
                        dPre = abs(realIndexDistance-currentDistance);
                    }
                } 
            }
            int k;
            for (k = 0; k < virtualToPhysical.size(); k++)
                if (virtualToPhysical[k] == numPre)
                    break;
            if (numPre != 0) graph[i][k] = dPre;//虚拟地址
            for (k = 0; k < virtualToPhysical.size(); k++)
                if (virtualToPhysical[k] == numNext)
                    break;
            if (numNext != 0) graph[i][k] = dNext;
        }
    }
    for (int i = 0; i < n; i++)//对角线赋值为0
        graph[i][i] = 0;
}

bool Singleton::sameLine(int num1, int num2) {
    for (int i = 0; i < stations[num1].lineNumber; i++) {
        for (int j = 0; j < stations[num2].lineNumber; j++) {
            if (stations[num1].lineAndTime[i].first 
            == stations[num2].lineAndTime[j].first)
                return true;
        }
    }
    return false;
}

bool Singleton::sameLineThree(int num1, int num2, int num3) {
    for (int i = 0; i < stations[num1].lineNumber; i++) {
        for (int j = 0; j < stations[num2].lineNumber; j++) {
            for (int k = 0; k < stations[num3].lineNumber; k++)
            if (stations[num1].lineAndTime[i].first 
            == stations[num2].lineAndTime[j].first && 
            stations[num1].lineAndTime[i].first 
            == stations[num3].lineAndTime[k].first)
                return true;
        }
    }
    return false;
}

//通过传递引用计算编号为num的站的最近的两个中转站编号和到其距离（不妨一个称为前，一个称为后）
void Singleton::getPreNext(int num, int& numPre, int& numNext, int& dPre, int& dNext) {
    numNext = numPre = -1;
    dPre = dNext = 0;
    //若num本身就是中转站，则返回的两个中转站都是它自己，距离都为0
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
                    numNext = currentStation;
                    dNext = abs(stations[num].lineAndTime[0].second-
                    currentDistance);
                    break;
                } 
                else {
                    numPre = currentStation;
                    dPre = abs(stations[num].lineAndTime[0].second-
                    currentDistance);
                }
            } 
        }
    }
}

vector<int> Singleton::findPath(string& s1, string& s2) {

    int num1 = nameToIndex[s1];
    int num2 = nameToIndex[s2];    

    if (num1*num2 == 0)
        return vector<int>{MAX_TIME};

    if (sameLine(num1, num2)) {
        vector<int> temp = {num1, num2};
        return temp;
    }
    int num1Pre, num2Pre, num1Next, num2Next;
    int num1PreD, num2PreD, num1NextD, num2NextD;

    getPreNext(num1, num1Pre, num1Next, num1PreD, num1NextD);
    getPreNext(num2, num2Pre, num2Next, num2PreD, num2NextD);

    vector<int> l1 = dijkstra(num1Pre, num2Pre);
    vector<int> l2 = dijkstra(num1Pre, num2Next);
    vector<int> l3 = dijkstra(num1Next, num2Pre);
    vector<int> l4 = dijkstra(num1Next, num2Next);

    vector<int> distances(4, 0);
    distances[0] = num1PreD + l1[0] + num2PreD;
    distances[1] = num1PreD + l2[0] + num2NextD;
    distances[2] = num1NextD + l3[0] + num2PreD;
    distances[3] = num1NextD + l4[0] + num2NextD;
    int minPathIndex = 0;
    for (int i = 1; i < 4; i++)
        if (distances[minPathIndex] > distances[i]) 
            minPathIndex = i;

    vector<int> minPath;
    switch(minPathIndex) {
        case 0:
            minPath = l1;
            break;
        case 1:
            minPath = l2;
            break;
        case 2:
            minPath = l3;
            break;
        case 3:
            minPath = l4;
            break;
    }

    auto iter = minPath.begin();
    if (num1Next != num1Pre)
        minPath[0] = num1;
    else
        minPath.erase(iter);
    if (num2Pre != num2Next)
        minPath.push_back(num2);
    return minPath;
}

/*vector<int> dijkstra(int start, int end)
该函数取两个中转站的物理地址
返回一个vector<int>
vector[0]记录start到end的最短时间，包括中转时间
[1]-[back]依次是路径上结点物理编号如start,...,end
特殊情况：
    start或end是-1，此时仅返回含一个元素MAX_TIME的vector即可*/

vector<int> Singleton::dijkstra(int start, int end)
{
    vector<int> result;
    if(start == -1 || end == -1)
    {
        result.push_back(MAX_TIME);
        return result;
    }
    int n = graph.size();
    int source, destination;

    //将实际地址映射到虚拟地址
    for(int i=0;i<n;i++)
    {        
        if(virtualToPhysical[i] == start)
            source = i;
        if(virtualToPhysical[i] == end)
            destination = i;     
    }

    enum {UNCOLLECTED, COLLECTED} state;

    vector<int> path(n, -1); //存放到该点最短路径的上一个顶点编号
    vector<int> dist(n, MAX_TIME); //到已收录顶点的最短距离
    vector<bool> status(n, UNCOLLECTED); //表示每个点的状态：已收集到S中和未收集到S中

    //初始化距离    
    dist[source] = 0;
    //dijkstra算法
    for(int i=0, s=source;i<n-1;i++)
    {        
        //将距离最小的点s收录到集合S中
        status[s] = COLLECTED;

       //通过收录进来的点s更新其它尚未收录的点
        for(int j=0;j<n;j++) 
        if(status[j] == UNCOLLECTED) //如果没有被收进来，则更新最短距离
        {
            if(dist[s] + graph[s][j] < dist[j])
            {
                if(path[s] != -1 && !sameLine(path[s], j)) //如果从s到j需要换乘
                {
                    if(dist[s] + graph[s][j] + TRANSFER_TIME < dist[j]) //则需考虑上换乘时间
                    { 
                        dist[j] = dist[s] + graph[s][j] + TRANSFER_TIME;
                        path[j] = s;
                    } 
                }
                else //不需换乘则按普通情形处理即可
                {
                    dist[j] = dist[s] + graph[s][j];
                    path[j] = s;
                }
            } 
        }                 
        
        //找到下一个距离最小且尚未收录的点
        int minV = 0, minDist = MAX_TIME;
        for(int j=0;j<n;j++)
            if(status[j] == UNCOLLECTED && dist[j] < minDist) {
                minV = j;
                minDist = dist[j];
            }
        s = minV;
    }
    
    int vertex = destination;
    while(path[vertex] != -1) //依次将路径上的点从终点到起点一个个存进来
    {
        result.push_back(virtualToPhysical[vertex]);
        vertex = path[vertex];
    }
    result.push_back(virtualToPhysical[vertex]);

    result.push_back(dist[destination]);
    for(int i=0;i<result.size()/2;i++)
        swap(result[i], result[result.size()-i-1]);
    return result;
}


//输出路径
void Singleton::printPath(vector<int>& v) {
    auto iter = v.begin();
    auto iter1 = iter+1;
    auto iter2 = iter1+1;
    while(iter2 != v.end()) {
        if (sameLineThree(*iter, *iter1, *iter2)) {
            iter1 = v.erase(iter1);
            iter2 = iter1 + 1;
        } else {
            iter++, iter1++, iter2++;
        }
    }
    cout << "-----------------------" << endl;   
    cout << stations[v[0]].name << " -> ";
    for (int i = 1; i < v.size()-1; i++)
        cout << stations[v[i]].name << "\n↑换乘↓\n" 
        << stations[v[i]].name << " -> ";
    cout << stations[v.back()].name << endl;
    cout << "-----------------------" << endl;   
}

Singleton* Singleton::getInstance() {
    if (instance == nullptr)
        instance = new Singleton();
    return instance;
}