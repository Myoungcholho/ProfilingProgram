#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include "QueryP.h"
#include <locale.h>


int main()
{
    //_wsetlocale(LC_ALL, L"Korean");

    for (int i = 0; i < 4000; ++i)
    {
        PRO_BEGIN(L"Func1");

        printf("hell");

        PRO_END(L"Func1");
    }
    return 0;
}