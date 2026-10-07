/* 
设计一个密码分析器。🔐输入一行密码，例如：Fischl2026@Genshin，要求程序输出：
长度：18
大写字母：2
小写字母：11
数字：4
其他字符：1
是否包含数字：Yes
是否包含大小写字母：Yes
全部转换成小写：fischl2026@genshin
只提取其中的数字：2026

要求必须至少使用今天学的一下内容，并且数字、大写、小写、其他字符尽量一次循环统计完：
string
getline
size
范围 for
isupper
islower
isdigit
tolower
push_back
*/
#include<iostream>
// #include<string> // 多余
#include<vector>
#include<cctype>
using namespace std;

int main(){
    string str;
    
    cout << "请输入密码 : ";

    // 忘看题目要求了，应该用getline()的
    // cin >> str;
    getline(cin, str);

    if (str.empty()){
        cout << "Empty String!";
        return 0;
    }
    cout << "你的密码是 : " << str << endl;

    cout << "长度：" << str.size() << endl;

    int upper = 0, lower = 0, digit = 0, other = 0;
    for(auto c : str) {
        if (isupper(c)){
            upper++;
        }
        else if (islower(c)){
            lower++;
        }
        else if (isdigit(c)){
            digit++;
        }
        else
            other++;
    }

    cout << "大写字母：" << upper << endl;
    cout << "小写字母：" << lower << endl;
    cout << "数字：" << digit << endl;
    cout << "其他字符：" << other << endl;

    cout << "是否包含数字：";
    if (digit > 0) {
        cout << "Yes" << endl;
    }
    else{
        cout << "No" << endl;
    }

    cout << "是否同时包含大写字母和小写字母：";
    if (upper > 0 && lower > 0){
        cout << "Yes" << endl;
    }
    else{
        cout << "No" << endl;
    }

    string temp;
    for (auto &c : str) {
        c = tolower(c);
        if (isdigit(c)){
            temp.push_back(c);
        }
    }
    cout << "全部转换成小写：" << str << endl;
    cout << "只提取其中的数字："<< temp << endl;

    
    return 0;
}