/**@uthor: Eduardo Maldonado Fernández, José María Amusquívar Poppe
 * Programa que monitoriza una canoa con caníbales y misioneros.
 * 23/04/19
**/

//Misioneros y caníbales.
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h> //En este caso, requerido para generar un valor aleatorio.

/**Variables que cuentan las personas en la canoa, el número de misioneros en
 * la canoa, y el número de caníbales en la canoa, respectivamente.
 **/ 
int personasEnCanoa = 0;
int nMis = 0; int nCan = 0;

/***Creación del mutex, y de una única variable condición***/
pthread_mutex_t mutexHilo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condHilo = PTHREAD_COND_INITIALIZER;

//Rutina de caníbales, crea un hilo por cada caníbal.
void* llegaCanibal(void* arg){	
	pthread_mutex_lock(&mutexHilo);
	
	//Mientras el número de misioneros en la canoa sea 2, se duerme el hilo.
	while(nMis == 2){
		pthread_cond_wait(&condHilo, &mutexHilo);
	}

	//Se incrementa el número de personas en la canoa, así como el de caníbales.
	personasEnCanoa++; nCan++;
	printf("Canibal llega al bote --> %d\n", personasEnCanoa);
	
	//Mientras en la canoa no hayan 3 pasajeros, se manda a los dos hilos a dormir.
	while(personasEnCanoa <= 2){
		pthread_cond_wait(&condHilo, &mutexHilo);
		//Una vez se despiertan, ya no son necesarios, así que se terminan.
		pthread_mutex_unlock(&mutexHilo);
		pthread_exit(NULL);
	}
	
	//Una vez se llena la canoa, ésta se vacía inmediatamente.
	personasEnCanoa = 0; nCan = 0; nMis = 0;
	printf("****Se ha vaciado el bote****\n");

	//Se despierta a los dos hilos dormidos, se libera el mutex y se termina el hilo.
	pthread_cond_broadcast(&condHilo);					
	pthread_mutex_unlock(&mutexHilo);
	pthread_exit(NULL);
}


//Rutina de los misioneros, crea un hilo por misionero.
void* llegaMisionero(void* arg){
	pthread_mutex_lock(&mutexHilo);

	//Si en la canoa se haya un misionero y un caníbal, no puede meterse otro misionero.
	while(nCan == 1 && nMis == 1){
		pthread_cond_wait(&condHilo, &mutexHilo); //Se manda a dormir.
	}

	//Se suma una persona a la canoa, así como el número de misioneros.
	personasEnCanoa++; nMis++;
	printf("Misionero llega al bote --> %d\n", personasEnCanoa);

	//Mientras en la canoa no hayan 3 pasajeros, se manda a los dos hilos a dormir.
	while(personasEnCanoa <= 2){
		pthread_cond_wait(&condHilo, &mutexHilo);
		//Una vez se despiertan, ya no son necesarios, así que se terminan.
		pthread_mutex_unlock(&mutexHilo);
		pthread_exit(NULL);
	}

	//Una vez se llena la canoa, ésta se vacía inmediatamente.
	personasEnCanoa = 0; nCan = 0; nMis = 0;
	printf("****Se ha vaciado el bote****\n");

	//Se despierta a los dos hilos dormidos, se libera el mutex y se termina el hilo.
	pthread_cond_broadcast(&condHilo);			
	pthread_mutex_unlock(&mutexHilo);	
	pthread_exit(NULL);
}


//Función main, verifica los argumentos, y además crea los distintos hilos.
int main(int argc, char* argv[]){
	srand(time(NULL));

	int numCan; int numMis;
	pthread_t *misioneros;
	pthread_t *canibales;

	if(argc != 3){
		fprintf(stderr, "Sintaxis erronea ./ejecutable num_Misioneros num_Canibales...\n");
		exit(-1);
	}

	numMis = atoi(argv[1]);
	if (numMis < 0) {
		fprintf(stderr,"Datos erroneos. El número de misioneros debe ser >= 0: ./ejecutable num_Misioneros num_Canibales...\n");
		exit(-1);
	}

	numCan = atoi(argv[2]);
	if (numCan < 0) {
		fprintf(stderr,"Datos erroneos. El número de canibales debe ser >= 0: ./ejecutable num_Misioneros num_Canibales...\n");
		exit(-1);
	}
	
	//Si la suma de misioneros y caníbales es distinto de algún múltiplo de 3, se realizan distintas acciones.
	if((numMis + numCan) % 3 != 0){
		printf("Se procederá a redondear el numero de canibales y/o misioneros a un valor multiplo de 3\n");
		int div = (numMis+numCan)%3; //Se obtiene el módulo 3 de la suma de ambos.

		if(div == 1){
			numMis++; numCan++; //Si es 1, se suma uno a cada variable.
		}else{		
			//Si es 2, se añade uno aleatoriamente a alguno de los dos.
			if((rand()%2) == 1){ //Un rand() para 0 y 1.
				numMis++;
			}else{
				numCan++;
			}
		}
	}

	printf("Misioneros: %d	Canibales: %d\n", numMis, numCan);
	printf("**********INICIO**********\n");

        int i; int ret; void* dummy;

	misioneros = malloc(numMis*sizeof(pthread_t));
	if(misioneros == NULL) {
    		fprintf(stderr,"Error en la petición de memoria para pthread_t consumidores\n");
   		 exit(-1);
  	}

	canibales = malloc(numCan*sizeof(pthread_t));
	if(canibales == NULL) {
    		fprintf(stderr,"Error en la petición de memoria para pthread_t consumidores\n");
   		 exit(-1);
  	}

	for(i = 0; i < numMis; i++) {
    		ret = pthread_create(&misioneros[i], NULL, llegaMisionero, NULL);  
    		if(ret){
			errno = ret;
			fprintf(stderr,"error %d: %s\n",errno,strerror(errno));
      			exit(-1);
    		}
  	}	
	
	for(i = 0; i < numCan; i++) {
    		ret = pthread_create(&canibales[i], NULL, llegaCanibal, NULL);   
    		if(ret){
			errno = ret;
			fprintf(stderr,"error %d: %s\n",errno,strerror(errno));
      			exit(-1);
    		}
  	}

	for(i = 0; i < numMis; i++) {
    		ret=pthread_join(misioneros[i],&dummy);
    		if(ret){
			errno=ret;
			fprintf(stderr,"Error %d en el join del hilo productor %d: %s\n",errno,i,strerror(errno));
			exit(-1);
    		}
  	}
	
  	for(i = 0; i < numCan; i++) {
    		ret=pthread_join(canibales[i],&dummy);
    		if(ret){
			errno=ret;
			fprintf(stderr,"Error %d en el join del hilo consumidor %d: %s\n",errno,i,strerror(errno));
			exit(-1);
    		}
  	}
printf("***********FIN************\n");
exit(0);
}
