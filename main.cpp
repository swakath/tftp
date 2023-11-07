#include <iostream>
#include <fstream>
#include <string>

using namespace std;
int main(int argc, char* argv[]){
    string file_path = "your_file.txt";
    fstream file(file_path, ios::out);
    cout<<file.is_open()<<file<<endl;
    fstream f2(file_path, ios::out);
    cout<<f2.is_open()<<f2<<endl;
    while(1){}
    return 0;
}