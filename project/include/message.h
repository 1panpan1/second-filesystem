#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include<iostream>
#include<string>
using namespace std;

class Message
{
public:
    Message(){};
    ~Message(){};
    static void out(string msg)
    {
        cout<<msg<<endl;
    }
};

#endif