/*
使用 vector 保存。
使用范围 for 输入。
输出所有元素。
输出所有大于平均值的元素。
输出所有偶数。
将数组从小到大排序后输出。
将数组从大到小输出。
统计正数、负数、0 分别有多少个。
*/
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional> // 建议加上
using namespace std;

int main(){

    int n = 0;
    int sum = 0;
    cout << "输入：" << endl;
    cin >> n;

// 安全检查
    if (n <= 0) {
        cout << "n 必须大于 0" << endl;
        return 0;
    }

    vector<int> a(n);
    for (auto &x : a) {
        cin >> x;
    }
    
    cout << endl << "原数组：" << endl;
    for (auto x : a) {
        cout << x << " ";
        sum += x;
    }
    double average = static_cast<double>(sum) / n; //建议加上这个来保持
    //cout << endl << endl << "平均值：" << endl << (float)sum / n << endl;
    cout << endl << endl << "平均值：" << endl << average << endl;

    cout << endl << "大于平均值：" << endl;
    for (auto x : a){
        if (x > (float)sum / n)
            cout << x << " ";
    }

    cout << endl << endl << "偶数：" << endl;
    for (auto x : a){
        if (x % 2 == 0)
            cout << x << " ";
    }

    cout << endl << endl << "升序：" << endl;
    sort(a.begin(), a.end());
    for (auto x : a){
        cout << x << " ";
    }

    cout << endl << endl << "降序：" << endl;
    sort(a.begin(), a.end(), greater<int>());
    for (auto x : a){
        cout << x << " ";
    }

//建议一个循环写完
/*
    int count = 0;
    for (auto x : a){
        if (x > 0)
            count++;
    }
    cout << endl << endl << "正数：" << count << endl;


    count = 0;
    for (auto x : a){
        if (x < 0)
            count++;
    }
    cout << "负数：" << count << endl;

    count = 0;
    for (auto x : a){
        if (x == 0)
            count++;
    }
    cout << "零：" << count << endl;
*/

    int positive = 0;
    int negative = 0;
    int zero = 0;
    for(auto x : a){
        if (x > 0)
            positive++;
        else if (x < 0)
            negative++;
        else 
            zero++;
    }
    cout << endl << endl;
    cout << "正数：" << positive << endl;
    cout << "负数：" << negative << endl;
    cout << "零：" << zero << endl;
    return 0;
}