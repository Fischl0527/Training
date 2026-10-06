/*
原数据：5 8 2 10 7 3
5
8
2
10
7
3
-1
元素个数：6
最大值：10
最小值：2
平均值：5.83333
排序后：2 3 5 7 8 10
*/

#include<iostream>
#include<vector>
// #include<functional> // 多余，可以删掉
#include<algorithm>
// #define MAXSIZE 100  // C/C++老传统了, 建议constexpr int MAXSIZE = 100
// constexpr int MAXSIZE = 100;
using namespace std;

int main(){
    //初始定义
    vector<int> a;
    int num = 0;
    int count = 0;

    //-----------不必加MAXSIZE了没必要-----------------
    //-----------如果第一个数是-1，那么可能会出事的------------
    // cout << "Please input your numbers. MAXSIZE is " << MAXSIZE << "! '-1' is end" << endl;
    // while (count <= MAXSIZE) {
    //     count++;
    //     cout << "Number" << count << ": ";
    //     cin >> num;
    //     if (num == -1)
    //         break;
    //     a.push_back(num);
    //     if (count == MAXSIZE) {
    //         cout << "MAXSIZE!" << endl;
    //         break;
    //     }
    // }
    cout << "Please input numbers (-1 to end):" << endl;
    while (true) {
        count++;
        cout << "Number" << count << ": ";
        cin >> num;
        if(num == -1) {
            break;
        }
        a.push_back(num);
    }


    // ------------加入空数组警告-----------
    // 不加的话，后面的a[0]是越界访问，属于未定义行为
    if (a.empty()) {
        cout << "No data!" << endl;
        return 0;
    }


    cout << "Your array is :" << endl;
    for (auto x : a) {
        cout << x << " ";
    }

    cout << endl << "Number of elements: " << a.size() << endl;

    // 由于max，min在cpp中有函数的，可能有关键字，所以尽量起一个别名
    //int max = a[0], min = a[0];
    int maxvalue = a[0], minvalue = a[0];

    // 由于sum已经是double了，所以后面没必要用static_cast<double>(sum)了
    // 想体现的话，这里用int
    //double sum = 0;
    int sum = 0;
    for(auto x : a) {
        if (maxvalue < x) {
            maxvalue = x;
        }
        else if (minvalue > x) {
            minvalue = x;
        }
        sum += x;
    }

    cout << "Max number is : " << maxvalue << endl;
    cout << "Min number is : " << minvalue << endl;

    // 由于前面的sum是int，所以这得转换类型，
    // cout << "The average is : " << static_cast<double>(sum) / a.size() << endl;
    double average = static_cast<double>(sum) / a.size();
    cout << "The average is : " << average << endl;


    sort(a.begin(), a.end());
    cout << "The sort array is : ";
    for (auto x : a){
        cout << x << " ";
    }

    return 0;
}