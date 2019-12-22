#include <iostream>
#include <vector>
#include "graph.cpp"
using namespace std;
//void findPath(string& , string&)

int main() {
    Singleton* instance = Singleton::getInstance();
    while(1) {
        string start, end;
        instance->test();
        cout << "请输入起始站" << endl;
        cin >> start;
        cout << "请输入终点站" << endl;
        cin >> end;
        cout << start << ' ' << end << endl;
        cout << "依时间的最短路径是：";
        
        // vector<int> temp = instance->findPath(start, end);
        // instance->printPath(temp);
        return 1;
    }
}
