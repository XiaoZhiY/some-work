#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex;
sem_t empty; // empty buf num
sem_t full; // available product


// queue
int buf[10];
int start = 0;
int end = 0;

int cur = 0; // cur counter

static volatile int productID = 0;


void *product_i(void *param)
{
    int productorId = *(int *)param;
    for(;;)
    {
	sem_wait(&empty);
	sem_wait(&mutex);
	buf[start] = ++productID;
	cur++;
	printf("productor id: %d, productID: %d, cur: %d\n", productorId, productID, cur);
	start = (start + 1) % 10;
	sem_post(&mutex);
	sem_post(&full);

	sleep(2);
    }
}

void *consumer_i(void *param)
{
    int consumerId = *(int*)param;
    for(;;)
    {
	sem_wait(&full);
	sem_wait(&mutex);
	int useproductid = buf[end];
	cur--;
	printf("comsumer id: %d, productID: %d, cur: %d\n", consumerId, useproductid, cur);
	
	end = (end + 1) % 10;
	sem_post(&mutex);
	sem_post(&empty);

	sleep(3);
    }
}


int main(int argc, char **argv)   
{
    pthread_t productor[3];
    pthread_t consumer[2];
//    pthread_attr_init(&attr);
    int pro_id[3] = {1, 2, 3};
    int con_id[2] = {1, 2};
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, 10);
    sem_init(&full, 0, 0);
    
    for(int i = 0; i < 3; i++)
	pthread_create(&productor[i], NULL, product_i, &pro_id[i]);

    for(int i = 0; i < 2; i++)
	pthread_create(&consumer[i], NULL, consumer_i, &con_id[i]);
    
    for(int i = 0; i < 3; i++)
	pthread_join(productor[i],NULL);
    for(int i = 0; i < 2; i++)
	pthread_join(consumer[i],NULL);

    printf("main end\n");
    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    return 0;
}
