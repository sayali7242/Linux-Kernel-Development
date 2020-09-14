#include<sys/mman.h>
#include<sys/types.h> 
#include<sys/stat.h> 
#include<stdio.h> 
#include<unistd.h> 
#include<stdlib.h>
#include<string.h> 
#include<fcntl.h> 
#include<errno.h>
#include "struct.h"

int main(){
	int i; 
	int fds = shm_open(var, O_RDWR | O_CREAT, S_IRWXU);
	ftruncate(fds, sizeof(struct shared_data));
	void *return_val = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, fds, 0);
	
	if(return_val == MAP_FAILED){
		perror("mmap process failed\n");
		exit(1);
	}
	
	struct shared_data *shared_data_ptr = (struct shared_data *)return_val; 
	if(shared_data_ptr == NULL){
		perror("Memory allocation failed\n");
	}
	
	while(shared_data_ptr->write_guard == 0){}
	int *local_array = (int*)malloc(sizeof(int)*shared_mem_size);
	srand(1);
	for(i = 0; i < shared_mem_size; i++){
		local_array[i] = rand();
	}
	memcpy((void *)shared_data_ptr->data, (void *)local_array, sizeof(int)*shared_mem_size);
	shared_data_ptr->read_guard = 1;
	while(shared_data_ptr->delete_guard == 0){}
	shm_unlink(var);
	return 0;
}
