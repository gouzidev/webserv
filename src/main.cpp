
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;

class Debugger
{
    public:
        template <typename Container>
        static void printVec(string varName, vector <Container> &v)
        {
            cout << varName << ": " << endl << "----";
            for (size_t i = 0; i < v.size(); i++)
                cout << v[i] << "  ";
            cout << endl;
        }

        template <typename type1, typename type2>
        static void printMap(string varName, map <type1, type2> &m)
        {
            cout << varName << ": " << endl << "----";
            for (size_t i = 0; i < m.size(); i++)
                cout << m[i].first << " -> " << m[i].second << "  ";
            cout << endl;
    }
};


vector<string> split(string &str, char delim)
{
        size_t start;
        vector<string> v;
        size_t end = 0;
     
        while ((start = str.find_first_not_of(delim, end)) != string::npos) {
            end = str.find(delim, start);
            v.push_back(str.substr(start, end - start));
        }
        return v;
}
int main()
{
    string  test = "test";
    vector <string> res = split(test, ' ');
    Debugger::printVec("res", res);
}