#include <stdio.h>


int main(void)
{
    int c;
    while ((c = getchar()) >= 0)
        printf("\\x%X", c);
}
