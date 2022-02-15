#include <stdio.h>
#include <stdlib.h>


struct OnionCore
{
    int x;
};

struct Onion5
{
    struct OnionCore o;
    int x;
};

struct Onion4
{
    struct Onion5 o;
    int x;
};

struct Onion3
{
    struct Onion4 o;
    int x;
};

struct Onion2
{
    struct Onion3 o;
    int x;
};

struct Onion1
{
    struct Onion2 o;
    int x;
};

struct Onion
{
    struct Onion1 o;
    int x;
};


int main(int argc, char **argv)
{
    struct Onion oninon;
    oninon.o.o.o.o.o.o.x = 10;

    printf("testNested: %d  \n", oninon.o.o.o.o.o.o.x);
    return 0;
}
