#include <stdio.h>


/*
 *  the file size will be 256 * 64 - 256 + 8 = 16136
 */
int main(int argc, char** argv)
{
    char filename[] = "memory00";
    int a[2];
    for(int i = 0;i < 12;i++)
    {
	a[0] = i + 1;
	filename[6] = '0' + (i+1) / 10;
	filename[7] = '0' + (i+1) % 10;
	FILE *fp = fopen(filename,"w");
	for(int j = 0;j < 64;j++)
	{
	    a[1] = j;
	    fseek(fp, 256 * j, SEEK_SET);
	    fwrite(a, sizeof(int), 2, fp);
	}
	fclose(fp);
    }
    return 0;
}
