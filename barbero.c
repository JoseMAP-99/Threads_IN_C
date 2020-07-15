/**@uthor: Eduardo Maldonado Fernández, José María Amusquívar Poppe
 * Programa que monitoriza una barbería con dos tipos de hilos.
 * 23/04/19
**/

//BARBERO DORMILÓN 

#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h> 
#include <errno.h>

/*
Este problema fue planteado por Edsger Dijkstra. Una barbería tiene una sala de 
espera con N sillas, más la silla de barbero. Si no hay clientes, el barbero se echa a
dormir. Si un cliente entra en la barbería y encuentra todas las sillas ocupadas, se 
marcha de la barbería. Si el barbero está ocupado, pero hay sillas disponibles, el cliente
se sienta en una de ellas. Si el barbero está dormido, el cliente lo despierta. Se trata de
escribir un programa que coordine al barbero y los clientes, cada uno de los cuales se 
corresponde con un hilo. 
*/

/*
CONDICIONES: 

-Barbero se echa a dormir si no hay clientes 

-Si un cliente entra en la barbería y encuentra todas las sillas ocupadas, se marcha de la barbería 

-Si el barbero está dormido, el cliente lo despierta. 

-Si el barbero está ocupado pero hay sillas disponibles, el cliente se sienta en una de ellas 
*/   

/***Creación del mutex, y variables condición para cada tipo de hilo***/
pthread_mutex_t mutexHilo = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t condBarbero = PTHREAD_COND_INITIALIZER; 
pthread_cond_t condCliente = PTHREAD_COND_INITIALIZER; 

/**Creación de variables para contar el número de sillas disponibles, la cola de clientes
 * que existe, y una variable que avisa si han terminado de llegar clientes, respectivamente
 **/
int numSillas; 
int colaClientes = 0;
int terminado = 0; 

int sillaOcupado = 0; //Cuando ocupado su valor sea 1, el barbero está ocupado.   


void *Barbero(void *arg){ 
    while(1){
	
    	pthread_mutex_lock(&mutexHilo); //Cogiendo el mutex cada inicio de rutina. 

    	while(colaClientes == 0){ 
		printf("El barbero se echa la siesta..\n");
        	pthread_cond_wait(&condBarbero, &mutexHilo); 
		if(terminado == 1)break; //Señal que indica que ya no hay más clientes.
    	}
 
    	if(terminado == 1){
    		pthread_mutex_unlock(&mutexHilo); //Se libera el mutex, y se sale del bucle.
    		break;
    	}
    	printf("El barbero esta atendiendo a un cliente\n");
    	sillaOcupado = 1; 
    	pthread_mutex_unlock(&mutexHilo); //Se libera el mutex mientras el barbero esté ocupado.

//CORTAR PELO() 

    	pthread_mutex_lock(&mutexHilo); //Se recupera el mutex cuando haya terminado.   
    	colaClientes--; 

    	printf("El barbero ha terminado con un cliente\n"); 
    	sillaOcupado = 0;

    	pthread_cond_signal(&condCliente); //Se despierta al cliente que estuviese en espera. 
    	pthread_mutex_unlock(&mutexHilo);	
    } 

    pthread_exit(NULL); 
}   


//Rutina del cliente, se ejecuta un hilo por cliente que llegue a la barbería.
void *Cliente(void *arg){ 

    pthread_mutex_lock(&mutexHilo); 

    //Si la cola de clientes es igual al número de sillas, los clientes próximos se marchan.
    if(colaClientes == numSillas){ 
	printf("No hay sillas vacias, el cliente se marcha\n");
    }else{ 

    colaClientes++; 
    printf("Cliente se sienta en una silla\n");

    //Si el barbero está ocupado, el cliente espera.
    while(sillaOcupado == 1){ 
	printf("El barbero esta ocupado, el cliente espera\n");
        pthread_cond_wait(&condCliente, &mutexHilo); 
    } 

    pthread_cond_signal(&condBarbero);        
    }

    pthread_mutex_unlock(&mutexHilo); 
    pthread_exit(NULL);
} 


//Función que verifica que los datos introducidos son válidos. 
void test_recursos_hilos(int totalClientes, int numeroSillas){ 

    int ret; long int i; void* dummy; 

    pthread_t *clientes; 
    pthread_t *barberoH; 

    clientes=malloc(totalClientes*sizeof(pthread_t)); 

    if (clientes==NULL) {
        fprintf(stderr,"Error en la petición de memoria para pthread_t clientes\n"); 
        exit(-1); 
    }   

    barberoH=malloc(1*sizeof(pthread_t)); 

    if (barberoH==NULL) {
        fprintf(stderr,"Error en la petición de memoria para pthread_t barberoH\n"); 
        exit(-1); 
    } 
	

    ret=pthread_create(&barberoH[0], NULL, Barbero, NULL); 

    if (ret) {
        errno=ret;
        fprintf(stderr,"error %d: %s\n",errno,strerror(errno)); 
        exit(-1); 
    }

    for(i=0; i<totalClientes; i++) { 

        ret = pthread_create(&clientes[i], NULL, Cliente, NULL); 

        if (ret) { 
            errno=ret;
            fprintf(stderr,"error %d: %s\n",errno,strerror(errno)); 
            exit(-1); 
        } 

    } 
  

    for(i=0;i<totalClientes;i++) { 

        ret=pthread_join(clientes[i],&dummy); 

        if (ret) { 
            errno=ret; 
            fprintf(stderr,"Error %d en el join del hilo clientes %d: %s\n",errno,i,strerror(errno)); 
            exit(-1); 
        } 

    } 
    
    //Si los clientes han terminado, entonces se habilita la señal, y se despierta al barbero para que terminé.
    terminado = 1;
    pthread_cond_signal(&condBarbero);

    ret=pthread_join(barberoH[0],&dummy); 

    if (ret) {
        errno=ret; 
        fprintf(stderr,"Error %d en el join del hilo barbero %d: %s\n",errno,i,strerror(errno)); 
        exit(-1); 
    } 
} 

//Función main del programa, recoge los argumentos pasados por consola.
int main(int argc, char* argv[]){ 

    int totalClientes; 

    //Comprobación de argumentos 

    if(argc != 3){ //Comprobamos el número de parametros pasados 
        fprintf(stderr, "Invocación erronea ./fich_ejercicio num_clientes num_sillas\n"); 
        exit(-1); 
    }   

    totalClientes = atoi(argv[1]); 

    if (totalClientes < 0) { 
        fprintf(stderr,"Invocación erronea. El número de clientes debe ser >= 0: ./fich_ejercicio num_clientes num_sillas\n"); 
        exit(-1); 
    } 

    numSillas = atoi(argv[2]); 

    if (numSillas <= 0) { 
        fprintf(stderr,"Invocación erronea. El número de sillas total debe ser > 0: ./fich_ejercicio num_clientes num_sillas\n"); 
        exit(-1); 
    } 

    printf("**********INICIO**********\n"); 

    test_recursos_hilos(totalClientes, numSillas); 

    printf("**********FIN**********\n"); 

    exit(0); 
} 
