#include <stdio.h>
#include <stdlib.h>

struct Course
{
    int CRN;
};

struct Phone
{
    int region_code;
    int number;
};

struct Student
{
    int grade;
    int netid;

    struct Course c;
    struct Phone p;
};

int main(int argc, char **argv)
{
    struct Student S;
    S.grade = 4;
    S.netid = 123;

    struct Course C;
    C.CRN=526;
    S.c = C;

    struct Phone P;
    P.number = 9799530;
    P.region_code = 217;
    S.p = P;

    printf("test member: %d %d %d %d-%d \n", S.grade, S.netid, S.c.CRN, S.p.number, S.p.region_code);
    return 0;
}
