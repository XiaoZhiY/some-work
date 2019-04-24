#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include <semaphore.h>
#include <time.h>
#include <memory.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#define IDLE 0 /* init state */
#define WAITING 1 /* waiting state */
#define WORKING 2 /* work state */

#define C_FIFO 1
#define C_LRU 2

// const 
#define PROC_NUM 12
#define PAGE_NUM 64

int state[PROC_NUM];
int mypage[PROC_NUM][10]; /* we set the last page is page table*/

unsigned char memory[1 << 14];
unsigned char page[64];
static volatile int freePage = 64;

double range[PAGE_NUM];

// semaphore
sem_t mutex;
sem_t s[PROC_NUM];

void init()
{
    memset(memory, 0, sizeof(memory));
    memset(page, 0 ,sizeof(page));
    for(int i = 0; i < PROC_NUM; i++)
    {
	state[i] = IDLE;
	sem_init(&s[i], 0, 0);
    }
    sem_init(&mutex, 0, 1);

    double sum = 0;
    for(int i = 0;i < PAGE_NUM; i++)
    {
	range[i] = (double)1 / sqrt(i + 1);
	sum += range[i];
	if(i)
	    range[i] += range[i-1];
    }
    for(int i = 0;i < PAGE_NUM; i++)
	range[i] /= sum;
}

void clear()
{
    memset(memory, 0, sizeof(memory));
    memset(page, 0 ,sizeof(page));
    for(int i = 0; i < PROC_NUM; i++)
    {
	sem_destroy(&s[i]);
    }
    sem_destroy(&mutex);

}

void getPages(int work_id)
{
    int n = 10, k = 0, i = 0;
    while(n)
    {
	if(page[i] == 0)
	{
	    n--;
	    page[i] = work_id;
	    mypage[work_id - 1][k] = i;
	    k++;
	}
	i++;
    }
    
}

void freePages(int work_id)
{
    for(int i = 0; i < 10;i++)
    {
	memset(&memory[mypage[work_id - 1][i] * 256], 0, 256);
    }
    for(int i = 0; i < 10;i++)
    {
	page[mypage[work_id - 1][i]] = 0;
    }
}

void updateTable_hit(int page_table_id, int accessPage, int method_id)
{
    if(method_id == C_FIFO)
    {
	//do nothing 
	;
    }
    else if(method_id == C_LRU)
    {
	for(int i = 0; i < PAGE_NUM; i++)
	{
	    if(memory[page_table_id * 256 + i * 4 + 1] == 1)
		memory[page_table_id * 256 + i * 4 + 2] += 1;
	}
	memory[page_table_id * 256 + accessPage *4 + 2] = 0;
    }
    else
    {	
	printf("error: in func updateTable_hit(int,int,int method_id), method_id invalid.");
	clear();
	exit(-1);
    }
}

void updateTable_miss(int page_table_id, int accessPage, int curpagenum, int method_id)
{
    if(method_id == C_FIFO)
    {
	if(curpagenum != 9)
	{
	    // update all
	    for(int i = 0; i < PAGE_NUM; i++)
		if(memory[page_table_id * 256 + i * 4 + 1] == 1)
		    memory[page_table_id * 256 + i * 4 + 2] += 1;

	    // frame num
	    memory[page_table_id * 256 + accessPage * 4] = curpagenum;
	    // sig bit
	    memory[page_table_id * 256 + accessPage * 4 + 1] = 1;
	    // age
	    memory[page_table_id * 256 + accessPage * 4 + 2] = 0;

	}
	else
	{
	    int outPage, maxtime = -1;
	    for(int i = 0; i < PAGE_NUM;i++)
	    {
		if(memory[page_table_id * 256 + i * 4 + 1] == 1)
	    	{
		    memory[page_table_id * 256 + i * 4 + 2] += 1;
		    if(memory[page_table_id * 256 + i * 4 + 2] > maxtime)
		    {
			maxtime = memory[page_table_id * 256 + i * 4 + 2];
			outPage = i;
		    }
		}
	    }

	    // update
	    memory[page_table_id * 256 + accessPage * 4] = memory[page_table_id * 256 + outPage * 4];
	    memory[page_table_id * 256 + accessPage * 4 + 1] = 1;
	    memory[page_table_id * 256 + accessPage * 4 + 2] = 0;

	    memory[page_table_id * 256 + outPage * 4 + 1] = 0;
	}   
    }
    else if(method_id == C_LRU)
    {
	if(curpagenum != 9)
	{
	     // update all
	    for(int i = 0; i < PAGE_NUM; i++)
		if(memory[page_table_id * 256 + i * 4 + 1] == 1)
		    memory[page_table_id * 256 + i * 4 + 2] += 1;

	    // frame num
	    memory[page_table_id * 256 + accessPage * 4] = curpagenum;
	    // sig bit
	    memory[page_table_id * 256 + accessPage * 4 + 1] = 1;
	    // age
	    memory[page_table_id * 256 + accessPage * 4 + 2] = 0;


	}
	else
	{
	    int outPage, maxtime = -1;
	    for(int i = 0; i < PAGE_NUM;i++)
	    {
		if(memory[page_table_id * 256 + i * 4 + 1] == 1)
	    	{
		    memory[page_table_id * 256 + i * 4 + 2] += 1;
		    if(memory[page_table_id * 256 + i * 4 + 2] > maxtime)
		    {
			maxtime = memory[page_table_id * 256 + i * 4 + 2];
			outPage = i;
		    }
		}
	    }

	    // update
	    memory[page_table_id * 256 + accessPage * 4] = memory[page_table_id * 256 + outPage * 4];
	    memory[page_table_id * 256 + accessPage * 4 + 1] = 1;
	    memory[page_table_id * 256 + accessPage * 4 + 2] = 0;

	    memory[page_table_id * 256 + outPage * 4 + 1] = 0;

	}
    }
    else
    {
	printf("error: in func updateTable_miss(int,int,int method_id), method_id invalid.");
	clear();
	exit(-1);
    }
}


void fun(int work_id)
{
    // print log
    char fileout[] = "./log/work00.log";
    fileout[10] = '0' + work_id / 10;
    fileout[11] = '0' + work_id % 10;
    FILE *fout = fopen(fileout, "w");

    // why can not use *filein 
    char filein[] = "./virtual/memory00";
    filein[16] = work_id / 10 + '0';
    filein[17] = work_id % 10 + '0';
    FILE *fin = fopen(filein, "r");
    int randaccess_time = 200;
    int miss_cnt = 0;
    int physical_addr, virtual_addr;
    int physical_page_id;
    int curpagenum = 0;
    int hit_flag = 0;
    for(int i = 0;i < randaccess_time;i++)
    {
	double randdouble = (double)rand() / (RAND_MAX);
	int randpage;
	for(int j = 0;j < PAGE_NUM; j++)
	{
	    if(randdouble < range[j])
	    {
		randpage = j;
		break;
	    }
	}
	int randoffset = rand() % 256 + 1;
	/* check if the page in page table */
	int page_table_id = mypage[work_id - 1][9];
	/* in page table 
	   one record like 
	   frame number(1 bytes) significant bit(1 bytes) age(1 bytes) remainer bit(1 bytes)
	 */

	virtual_addr = (randpage << 8) + randoffset;
	// hit
	if(memory[page_table_id * 256 + randpage * 4 + 1] == 1)
	{
	    hit_flag = 1;
	    physical_page_id = mypage[work_id - 1][memory[page_table_id * 256 + randpage *4]];
	    physical_addr = (physical_page_id << 8) + randoffset;

	    // C_FIFO or C_LRU
	    updateTable_hit(page_table_id, randpage, C_FIFO);
	}
	// miss
	else
	{
	    hit_flag = 0;
	    miss_cnt++;
	    updateTable_miss(page_table_id, randpage, curpagenum, C_FIFO);
	    if(curpagenum != 9)
		curpagenum++;

	    // read file
	    int t[2];
	    fseek(fin, 256 * randpage, SEEK_SET);
	    fread(t, sizeof(int), 2, fin);
	    //physical_addr = (mypage[work_id - 1][memory[page_table_id * 256 + randpage * 4]] << 8) + randoffset;
	    physical_page_id = mypage[work_id - 1][memory[page_table_id * 256 + randpage * 4]];
	    physical_addr = (physical_page_id << 8) + randoffset;
	    // write memory
	    memory[physical_page_id * 256 + 0] = t[0];
	    memory[physical_page_id * 256 + 1] = t[1];
	}

	/*
	   printf("No %d work's %d times access. virtual address: %d, physical address: %d. content:%d %d\n",
		work_id, i+1, virtual_addr, physical_addr, memory[physical_page_id * 256 + 0], memory[physical_page_id * 256 + 1]);
	*/
	fprintf(fout, "No %d work's %d times access. virtual address: %d, physical address: %d. content:%d %d. %s.\n",
		work_id, i+1, virtual_addr, physical_addr, memory[physical_page_id * 256 + 0], memory[physical_page_id * 256 + 1], hit_flag ? "hit" : "miss");
	
	int sleeptime = rand() % 101;
	sleep((double)sleeptime / 1000);
    }

    /*
       printf("No %d work's missing page rate: %lf\n", work_id, (double)miss_cnt / randaccess_time);
    */
    fprintf(fout, "No %d work's missing page rate: %lf\n", work_id, (double)miss_cnt / randaccess_time);
    fclose(fin);
    fclose(fout);
}

void *work_i(void *param)
{
   int work_id = *(int*)param;
   //printf("No %d work create success.\n", work_id);

   sem_wait(&mutex);
   state[work_id - 1] = WAITING;
   if(freePage - 10 >= 0)
   {
       sem_post(&s[work_id - 1]);
       freePage -= 10;
       state[work_id - 1] = WORKING;
       getPages(work_id);
   }
   sem_post(&mutex);

   sem_wait(&s[work_id - 1]);
   fprintf(stdout, "No %d work start.\n", work_id);
   fprintf(stdout, "NO %d work has page: %d %d %d %d %d %d %d %d %d %d.\n", work_id,
	   mypage[work_id-1][0],mypage[work_id-1][1],mypage[work_id-1][2],mypage[work_id-1][3],mypage[work_id-1][4],
	   mypage[work_id-1][5],mypage[work_id-1][6],mypage[work_id-1][7],mypage[work_id-1][8],mypage[work_id-1][9]);
   
   /* do someting here */
   fun(work_id);
   //sleep(2);
   fprintf(stdout, "No %d work finish.\n", work_id);

   sem_wait(&mutex);
   freePage += 10;
   state[work_id - 1] = IDLE;
   freePages(work_id);
   for(int i = 0; i < PROC_NUM;i++)
   {
       if(state[i] == WAITING)
       {
	    freePage -= 10;
	    state[i] = WORKING;
	    getPages(i + 1); // state[i] 's work_id = i + 1
	    sem_post(&s[i]);
	    break;
       }
   }
   sem_post(&mutex);
}

int main(int argc, char** argv)
{
    srand(time(NULL));
    init();
    pthread_t work[PROC_NUM];
    int work_id[PROC_NUM];
    for(int i = 0; i < PROC_NUM;i++)
	work_id[i] = i + 1;
    for(int i = 0; i < PROC_NUM;i++)
	pthread_create(&work[i], NULL, work_i,(void *)&work_id[i]);
    for(int i = 0; i < PROC_NUM;i++)
	pthread_join(work[i], NULL);
    clear();
    return 0;
}
