#include"iostream"
using namespace std;

int function1(int a){
    cout << a << endl;
}

int function2(int a, int b){
    cout << a + b << endl;
}

int function3(int a, int b){
    cout << a + b << endl;
    cout << a - b << endl;
    cout << a * b << endl;
}

int function4(float length, float width){
    cout << length * width << endl;
}

int function5(float a, float b, float c){
    cout << a + b + c << endl;
}

int main(){
    cout << "fountion1"<<endl;
    function1(1);
    cout << "fountion2"<<endl;
    function2(2, 3);
    cout << "fountion3"<<endl;
    function3(4, 5);
    cout << "fountion4"<<endl;
    function4(6, 7);
    cout << "fountion5"<<endl;
    function5(8, 9, 10);
    return 0;
}


