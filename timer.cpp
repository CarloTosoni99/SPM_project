
#include <iostream>
#include <string>
using namespace std;

// A simple name-echoing program.
// Art Lee, January 8, 2001
//
int main () {
    string s;
    cout << "Please enter your name: ";
    getline(cin, s);
    cout << "Hello " << s << endl;
    cout << "Have fun in cs2010...!..." << endl;
    return 0;
}
