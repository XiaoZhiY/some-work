#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
void fun(int *a)
{
    for(int i = 0; i < 10;i++)
	a[i] = i;
    for(int i = 0; i < 10;i++)
	printf("%d ", a[i]);
    printf("\n");
}

int main()
{
    
    srand(time(NULL));

    for(int i = 0 ; i < 40;i++)
    {    double x = (double)rand() / (RAND_MAX);
    printf("%lf\n", x);
    }
    return 0;

}
