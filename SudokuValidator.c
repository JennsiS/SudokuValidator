/**
 * Universidad del Valle de Guatemala
 * Sistemas operativos
 * Laboratorio 3
 * Jennifer Daniela Sandoval	18962
 * SudokuValidator
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <omp.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
 
//Variables globales
int grid[9][9];
int isvalid_rows, isvalid_columns, isvalid_subarrays;

typedef struct {
	int row;
	int column;		
} parameters;

//Declaracion de funciones de validacion
void *rows(void* parameters);
void *columns(void* parameters);
void *subarrays(void* parameters);

//Funcion que revisa que todas las filas tengan los numeros del 1 al 9 sin repetir
void *rows(void* parameters){
	//Uso de openmp
	omp_set_num_threads(9);
	omp_set_nested(1);
	isvalid_rows=1;
	int i;
	int j;
	# pragma omp parallel for private (isvalid_rows,j) schedule(dynamic)
	for (i=0; i<9; i++){
		//Arreglo en el cual se van agregando 1's correspondientes al numero encontrado en la fila
		int array[9]= {0};
		# pragma omp parallel for private (isvalid_rows,j) schedule(dynamic)
		for (j=0; j<9; j++){
			//Se obtiene el numero escrito en la grilla en la posicion i,j
			int num=grid[i][j];
			if(num<1 || num>9 || array[num-1]==1 ){
				isvalid_rows=0;
			}else{
				array[num-1]=1;
				isvalid_rows=1;
			}
		}
	}
return(0);
}


//Funcion que revisa que todas las columnas tengan los numeros del 1 al 9 sin repetir
void *columns(void* parameters){
	//Uso de openmp
	omp_set_num_threads(9);
	omp_set_nested(1);
	printf("\nEl thread que ejecuta el metodo para ejecutar el metodo de revision de columnas es: %ld\n",syscall(SYS_gettid));
	isvalid_columns=1;
	int k,l;
	# pragma omp parallel for private (isvalid_columns,l) schedule(dynamic)
	for (k=0; k<9; k++){
	printf("\n En la revision de columnas el siguiente es un thread de ejecucion: %ld\n",syscall(SYS_gettid));
		//Arreglo en el cual se van agregando 1's correspondientes al numero encontrado en la columna
		int array[9]= {0};
		#pragma omp parallel for private(isvalid_columns,l) schedule(dynamic)
		for (l=0; l<9; l++){
			//Se obtiene el numero escrito en la grilla en la posicion l,k
			int num=grid[l][k];
			if(num<1 || num>9 || array[num-1]==1 ){
				isvalid_columns=0;
			}else{
				array[num-1]=1;
				isvalid_columns=1;
			}
		}
	}
}


//Funcion que revisa que los subarreglos tengan los numeros del 1 al 9 sin repetir
void *subarrays(void* p){
	//Uso de openmp
	omp_set_num_threads(3);
	omp_set_nested(1);
	parameters *params = (parameters*) p;
	int row = params->row;
	int col = params->column;	
	int array[9]= {0};
	isvalid_subarrays=1;
	int m,n;
	#pragma omp parallel for private(isvalid_subarrays,n) schedule(dynamic)
	for ( m = row; m < row + 3; m++) {
		#pragma omp parallel for private(isvalid_subarrays,n) schedule(dynamic)
		for (n = col; n < col + 3; n++) {
			int num = grid[m][n];
			if (num < 1 || num > 9 || array[num-1] == 1) {
				isvalid_subarrays=0;
			} else {
				array[num-1] = 1;
				isvalid_subarrays=1;		
			}
		}
	}
return(0);
}

 
 int main(int argc, char** argv){
 	 omp_set_num_threads(1);
	 int file;
	 struct stat info;
	 int id_thread;
	 pid_t pid1,pid2;
	 char *pointer;
	 char board_array[2];
	 char buffer[1024];
	 int numprocess;
	 int size;
	 void *ptr;	
	 
	 //Leer el archivo obtenido desde la terminal
	 file=open(argv[1],O_RDONLY);
	 fstat(file,&info);
	 size=info.st_size;
	 	 
	 //Mapearlo a un espacio de memoria
	 ptr=mmap(NULL, size, PROT_READ, MAP_PRIVATE, file, 0);
	 if (ptr==MAP_FAILED){
	 	printf("Map failed\n");
	 	return -1;
	 }
	 
	 //Rellenar la grilla con el contenido del archivo
	 pointer=(char*) ptr;
	 board_array[1]='\0';
	 int i,j;
	 #pragma omp parallel for 
	 for (i=0; i<9;i++){
	 	#pragma omp parallel for 
	 	for (j=0; j<9; j++){
	 		board_array[0]= *pointer;
	 		grid[i][j]=atoi(board_array);
	 		pointer++;
	 	}
	 } 
	 
	 //Obtener el numero del proceso del main
	 numprocess=syscall(SYS_gettid);
	 printf("El thread en el que se ejecuta main es: %d\n",numprocess);
	 
	 //Revisar los subarreglos de 3x3
	 parameters *data = (parameters *) malloc(sizeof(parameters));
	 int p,q;
	 #pragma omp parallel for private(q) schedule(dynamic)
	 for(p=0; p<9; p++){
	 	#pragma omp parallel for private(q) schedule(dynamic)
	 	for (q=0; q<9; q++){
	 		if (p%3 == 0 && q%3 == 0) {
		 		data->row = p;		
				data->column = q;
		 		subarrays(data);
	 		}
	 		
	 	}
	 }	
	 
	 pid1=fork();
	 if(pid1==0){
	 	/*Proceso hijo*/
	 	pid_t parent_pid=getppid();
	 	char parentPID[10];
	 	sprintf(parentPID,"%d", (int)parent_pid);
	 	execlp("ps","ps","-p",parentPID,"-lLF", (char*)NULL);
	 	
	 }else{
	 	/*Proceso padre*/
	 	//Revision de columnas
	 	pthread_t thread_1;
	 	pthread_create(&thread_1, NULL, columns, NULL);
	 	pthread_join(thread_1,NULL);
	 	wait(NULL);
	 	//Revision de filas
	 	rows(NULL);
	 	
	 	//Imprimir si el sudoku es valido o no
	 	if (isvalid_rows==0 || isvalid_columns==0 || isvalid_subarrays==0){
	 		printf("La solucion de sudoku es invalida ");
	 		if (isvalid_rows==0){
	 			printf("\nRevisar los numeros colocados en las filas\n");
	 		}else if(isvalid_columns==0){
	 			printf("\nRevisar los numeros colocados en las columnas\n");
	 		}else{
	 			printf("\nRevisar los numeros colocados en los subarreglos de 3x3\n");
	 		}
	
	 	}else{
	 		printf("La solucion de sudoku es valida\n");
	 	}
	 	
	 	pid2=fork();
	 	if(pid2==0){
	 		printf("\n Antes de terminar el estado de este proceso y sus threads es:\n ");
	 		pid_t parent_pid2=getppid();
	 		char parentPID2[10];
	 		sprintf(parentPID2,"%d", (int)parent_pid2);
	 		execlp("ps","ps","-p",parentPID2,"-lLF", (char*)NULL);
	 	
	 	}else{
	 		wait(NULL);
	 	}
	 
	 }
 
 munmap(ptr,size);
 close(file);
 return(0);
 }
