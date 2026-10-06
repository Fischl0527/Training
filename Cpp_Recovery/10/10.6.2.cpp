/*
输入 n 个整数
1. 输出最大值
2. 输出最小值
3. 输出平均值
4. 输出所有偶数
5. 把数组逆序输出
*/

//--------------------头文件区别----------------------
// #include<iostream>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;


int main(){

//--------------------变量定义区别----------------------
/*    
int a[100] = {0};
    int b[10] = {0};
    int n = 0;
    int max;
    int min;
    int avarage = 0;

    std::cout << "Please input n" << std::endl; 
    std::cin >> n;
    for(int i = 0; i < n; i++){
        printf("Please inout number%d\n", i);
        scanf("%d", a + i);
    }
    std::cout << std::endl;

    // int a[10] = {5,3,2,7,8,1,4,6,9,0};
    // int b[10] = {0};
    // int n = 10;
    // int max;
    // int min;
    // int avarage = 0;
*/


    int n;
    cout << "Please input n: ";
    cin >> n;
    vector<int> a(n);
    cout << "Please input number: " << endl;
    for (int &x : a){
        cin >> x;
    }
    int max = a[0];
    int min = a[0];
    double sum = 0;

    
/*
//--------------插入排序算法---------------
    for(int i = 0; i < n; i++){
        b[i] = a[i];
    }

    for(int i = 1; i < n; i++){
        int j = i - 1;
        int temp = b[i];
        while(j >= 0 && b[j] > temp){
            b[j + 1] = b[j];
            j--;
        }
        b[j + 1] = temp;
    }
    max = b[n - 1];
    min = b[0];
*/

//--------------------找最大值----------------
    for (int x : a){
        if(x > max)
            max = x;
        if(x < min)
            min = x;
        sum += x;
    }

//---------------------输出简化-------------------------
/*    
    std::cout << "The array is : " << std::endl;
    for(int i = 0; i < n; i++){
        printf("%d ", *(a + i));
    }

    std::cout << std::endl << "The sort array is : " << std::endl;
    for(int i = 0; i < n; i++){
        avarage += b[i];
        printf("%d ", *(b + i));
    }
    
    std::cout<< std::endl;
    std::cout << "Max number is: " << max << std::endl;
    std::cout << "Min number is: " << min << std::endl;
    std::cout << "Avarage is: " << (float)avarage / n << std::endl;
    std::cout << "All even numbers:" << std::endl;;
    for(int i = 0; i < n; i++){
        if(a[i] % 2 == 0)
            printf("%d ", a[i]);
    }
    std::cout<< std::endl;

*/
    cout << "The array is : " << endl;
    for (auto x : a)
        cout << x << " ";
    cout << endl;
    cout << "Max number is: " << max << endl;
    cout << "Min number is: " << min << endl;
    cout << "Average number is: " << sum / n << endl;
    cout << "All even numbers: ";
    for (auto x : a){
        if (x % 2 == 0)
            cout << x << " ";
    }
    cout << endl;

//----------------逆序输出区别-------------
    // std::cout<< "Reverse order is:" << std::endl;
    // for(int i = n - 1; i >= 0; i--){
    //     printf("%d ", a[i]);
    // }
    cout << "Reverse order is: ";
    for (int i = n - 1; i >= 0; i--)
        cout << a[i] << " ";

    return 0;
}
