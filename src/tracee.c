#include <stdio.h>

void func1()
{
    printf("function1\n");
}

void func2()
{
    printf("function2\n");
}

void func3()
{
    printf("fucntion3\n");
}

int main()
{
    //printf("===========\n");
    func1();
    func3();
    func2();
    func2();
    func3();
    //printf("===========\n");
    return 0;
}    