#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5

char buffer[BUFFER_SIZE];
int in = 0;
int out = 0;
int count = 0;
int done = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void *producer(void *arg){
	FILE *fp = fopen("message.txt", "r");
	if (fp == NULL) {
		perror("ERROR: connot open message.txt");
		pthread_exit(NULL);
	}
	int ch;
	while ((ch = fgetc(fp)) != EOF){
		pthread_mutex_lock(&mutex);
		while(count == BUFFER_SIZE){
			pthread_cond_wait(&not_full, &mutex);
		} 
		buffer[in] = (char)ch;
		in = (in +1) % BUFFER_SIZE;
		count++;
		
		pthread_cond_signal(&not_empty);
		pthread_mutex_unlock(&mutex);
	}
	fclose(fp);
	
	pthread_mutex_lock(&mutex);
	done = 1;
	pthread_cond_signal(&not_empty);
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}

void *consumer(void *arg){
	for(;;){
		pthread_mutex_lock(&mutex);
		while (count == 0 && !done){
			pthread_cond_wait(&not_empty, &mutex);
		}
		if (count == 0 && done){
			pthread_mutex_unlock(&mutex);
			break;
		}
		char ch = buffer[out];
		out = (out + 1) % BUFFER_SIZE;
		count--;

		pthread_cond_signal(&not_full);
		pthread_mutex_unlock(&mutex);
		putchar(ch);
	}
	putchar('\n');
	pthread_exit(NULL);
}

int main(void){
	pthread_t prod_tid, cons_tid;

	if (pthread_create(&prod_tid, NULL, producer, NULL) != 0){
		perror("pthread_create producer");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_create(&cons_tid, NULL, consumer, NULL) != 0){
		perror("pthread_create consumer");
		exit(EXIT_FAILURE);
	}
	
	pthread_join(prod_tid, NULL);
	pthread_join(cons_tid, NULL);

	return 0;
}

