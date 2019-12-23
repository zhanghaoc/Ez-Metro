#include <iostream>
#include <vector>
#include "graph.cpp"
using namespace std;

int main() {
    Singleton* instance = Singleton::getInstance();
    //程序没有设置正常退出，ctrl+c关掉即可
    while(1) {
        string start, end;
        //instance->test();
        cout << "请输入起始站" << endl;
        cin >> start;
        cout << "请输入终点站" << endl;
        cin >> end;
        cout << start << ' ' << end << endl;
        cout << "依时间的最短路径是：";
        //现在还缺dijkstra,补上后就可以试试下面两行代码了
        vector<int> temp = instance->findPath(start, end);        
        instance->printPath(temp);
    }
}
