// test_vetcd.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include "../src/ui_event.h"
#include "../src/vetcd3.h"

//---------- Run Debug x64 ------------
// connect etcd server ok!
// key: / 333, value : 33333333333333
// connection close

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    std::string serr;
    vetcd3 cli;
    if (!cli.connect("127.0.0.1:2379", serr))
    {
        printf("connect etcd server failed, msg:%s, try reconnect with user/pwd\n", serr.c_str());

        if (!cli.connect_ex("127.0.0.1:2379", "user", "password", serr))
        {
            printf("connect etcd server failed, msg:%s\n", serr.c_str());
            return 0;
        }
    }
    printf("connect etcd server ok!\n");

    // list all keys with prefix "" if support(not enable auth)
    std::vector<vetcd_keyvalue> arr;
    cli.ls_all(arr, serr);
    for (auto& it : arr)
    {
        printf("key: %s, value: %s\n", it.skey.c_str(), it.sval.c_str());
    }
    cli.close();

    WSACleanup();
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
