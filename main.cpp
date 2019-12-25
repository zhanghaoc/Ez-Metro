#include <iostream>
#include <vector>
#include "graph.cpp"
using namespace std;

int main() {
    Singleton* instance = Singleton::getInstance();
    //如果是文件操作，可以正常退出；如果是命令行操作，可以使用CTRL+C退出
    while(1) {
        string start, end;
        cout << "请输入起始站" << endl;
        cin >> start;
        if(cin.eof())
            break;
        cout << "请输入终点站" << endl;
        cin >> end;
        cout << start << ' ' << end << endl;
        cout << "依时间的最短路径是：" << endl;
        

        vector<int> temp = instance->findPath(start, end); 
        if (temp[0] == MAX_TIME) {
            cout << "不存在此站点！" << endl; //若输入有误，会进行特殊处理
            continue;
        }            
        instance->printPath(temp);
    }
}
