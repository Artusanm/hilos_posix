/*
 * main.c
 *
 *  Created on: 24 oct. 2019
 *      Author: antonio
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TAM_BUFFER 10
#define NUM_ITEMS 20

typedef struct{
	char buffer[TAM_BUFFER];
	int indexP;
	int indexC;
	pthread_mutex_t mutex;
	sem_t sem_huecos;
	sem_t sem_items;

}TBuffer;

char generarItem(){
	char c;

	c=65+(rand()%26); //26 letras, y 65 la mayuscula en ascii
	printf("PROD: %c\n", c);
	return c;
}

void consumeItem(char c){
	printf("\tCON: %c\n", c);
}

void *productor(void *d){
	int i;
	char c;
	TBuffer *buf = (TBuffer *) d;

	for (i=0;i<NUM_ITEMS;i++){
		c = generarItem();
		sem_wait(&buf->sem_huecos);
		pthread_mutex_lock(&buf->mutex);
		//Colocar el char dentro del buffer:
		buf->buffer[buf->indexP] = c;
		buf->indexP = (buf->indexP +1)%TAM_BUFFER;
		pthread_mutex_unlock(&buf->mutex);
		sem_post(&buf->sem_items);
		sleep(rand()%2);
	}
	return NULL;
}

void *consumidor(void *d){
	int i;
	char c;
	TBuffer *buf = (TBuffer *) d;

	for(i=0;i<NUM_ITEMS;i++){
		sem_wait(&buf->sem_items); //Espera a tener un item que consumir
		pthread_mutex_lock(&buf->mutex);
		//Quitar el char del buffer:
		c = buf->buffer[buf->indexC];
		buf->indexC = (buf->indexC+1) % TAM_BUFFER; //Avanzar circularmente por el buffer
		pthread_mutex_unlock(&buf->mutex);
		sem_post(&buf->sem_huecos); //Avisar al productor de que se ha dejado un hueco libre
		consumeItem(c);
		sleep(rand()%3);
	}

	return NULL;

}

int main(){
	srand(time(0));
	TBuffer buf;
	pthread_t prod, cons;

	//Init estructura:
	pthread_mutex_init(&buf.mutex, 0);
	buf.indexC = buf.indexP = 0;

	sem_init(&buf.sem_huecos, 0, TAM_BUFFER);
	sem_init(&buf.sem_items, 0, 0);

	pthread_create(&prod, 0, productor, &buf);
	pthread_create(&cons, 0, consumidor, &buf);

	pthread_join(prod,0);
	pthread_join(cons,0);

	sem_destroy(&buf.sem_huecos);
	sem_destroy(&buf.sem_items);
	pthread_mutex_destroy(&buf.mutex);
}
