#include <string>
#include <iostream>
#include "../../includes/webserv.hpp"

using namespace std;


long stringToHexLong(string str)
{
    long res = 0;
    size_t i = 0;
    string hexChars = "0123456789abcdef";
    for (size_t i = 0; i < str.size(); i++)
    {
        size_t pos = hexChars.find(str[i]);
        if (pos == string::npos)
            return -1;
        res *= 16;
        long temp = (long) pos;
        res += temp;
    }
    return res;
}

bool handleChunkedBodyStart(string &body)
{
    // since i know this is chunked i know it must be  :
    //      HEX\r\n

    long hex = -1;
    size_t i = 0;
    string chunk;
    string newBody;
    string hexStr;
    size_t startReadingIdx = 0;
    size_t hexEndPos = body.find("\r\n", 0);
    while (hexEndPos != string::npos)
    {
        hexStr = body.substr(i, hexEndPos - i);
        hex = stringToHexLong(hexStr);
        if (hex == 0)
            break;

        startReadingIdx = hexEndPos += 2; // skip the \r\n
        chunk = body.substr(startReadingIdx, hex);
        i = startReadingIdx + chunk.size();
        if (i > body.size() - 2 || body[i] != '\r' || body[i + 1] != '\n')
        {
            cout << "Error: Expected '\\r\\n' after chunk data, but found: '" << body.substr(i, 2) << "'" << endl;
            return false;
        }
        i += 2;
        cout << "will start reading from idx " << startReadingIdx <<  ", which is -> " << body[startReadingIdx] << ", with chunk size " << hex << endl;
        newBody += chunk;
        hexEndPos = body.find("\r\n", i);
    }
    cout << "body ->  " << endl << endl << "{{"  << endl << newBody << endl  << "}}" << endl << endl;
    return true;
}


void testChunkedParsing() {
    cout << "=== Testing Chunked Parsing ===" << endl;
    
    // // Test case 1: Simple chunked data
    // string testBody1 = "5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n";
    
    // cout << "Test 1 - Input: " << endl << endl << testBody1 << endl << endl;
    // handleChunkedBodyStart(testBody1);
    
    // // Test case 2: Your actual chunked login data
    // string testBody2 = "1f\r\nemail=asma@gmail.com&password=1234\r\n0\r\n\r\n";
    
    // cout << "Test 2 - Input: " << endl << endl << testBody2 << endl << endl;
    // handleChunkedBodyStart(testBody2);
    
    // Test case 3: Multiple chunks
    string testBody3 = "a\r\nfirstName=\r\n4\r\nJohn\r\na\r\n&lastName=\r\n3\r\nDoe\r\n0\r\n\r\n";
    
    cout << "Test 3 - Input: " << endl << endl << testBody3 << endl << endl;
    handleChunkedBodyStart(testBody3);
    
    cout << "=== End Testing ===" << endl;
}


int main()
{
    testChunkedParsing();
}