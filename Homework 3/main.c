#include "memory.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include<pthread.h>

extern unsigned int frameSize;
extern unsigned int physicalSize;
extern unsigned int virtualSize;
extern int threadDatas[][6];

pthread_mutex_t lock;

int print = 0;

int partition(unsigned int lower, unsigned int upper){
	pthread_mutex_lock(&lock);
	int pivot = get(upper, "Quick");
	pthread_mutex_unlock(&lock);
	int i = lower - 1;
	int j;
	int first;
	int second;
	for(j = lower; j < upper; j++){
		pthread_mutex_lock(&lock);
		if(get(j, "Quick") < pivot){;
			i++;
			first = get(i, "Quick");
			second = get(j, "Quick");
			set(i, second, "Quick");
			set(j, first, "Quick");
		}
		pthread_mutex_unlock(&lock);
	}
	i++;
	pthread_mutex_lock(&lock);
	first = get(i, "Quick");
	second = get(upper, "Quick");
	set(i, second, "Quick");
	set(upper, first, "Quick");
	pthread_mutex_unlock(&lock);
	return i;
}

void quickSort(unsigned int lower, unsigned int upper){
	int piv;
	if((int)lower < (int)upper){
		piv = partition(lower, upper);
		quickSort(lower, piv - 1);		
		quickSort(piv + 1, upper);
	}
}

void merge(){
	unsigned int upper = virtualSize * frameSize - 1;
	unsigned int mid = upper / 2;
	
	int i = 0;
	int j = 0;
	int k = 0;
	
	unsigned int l = mid + 1;
	unsigned int r = upper - mid;
	
	int left[l];
	int right[r];
	
	int member;
	for(i = 0; i < l; i++){
		member = get(i, "Merge");
		left[i] = member;
	}

	for(i = 0; i < r; i++){
		member = get(i + mid + 1, "Merge");
		right[i] = member;
	}
	i = 0;
	
	while(i < l && j < r){
		if(left[i] <= right[j]){
			set(k++, left[i++], "Merge");
		}else{
			set(k++, right[j++], "Merge");
		}
			
	}
	
	while(i < l)
		set(k++, left[i++], "Merge");
		
	while(j < r)
		set(k++, right[j++], "Merge");
}

int linearSearch(int value){
	unsigned int size = virtualSize * frameSize;
	unsigned int i;	
	
	for(i = 0; i < size; i++){
		pthread_mutex_lock(&lock);
		if(value == get(i, "linear")){
			pthread_mutex_unlock(&lock);
			return i;
		}
		pthread_mutex_unlock(&lock);
	}
	return -1;	
}

int binarySearch(unsigned int lower, unsigned int upper, int value){
	
	unsigned int mid;
	if((int)upper >= (int)lower){
		mid = (upper - lower) / 2 + lower;
		pthread_mutex_lock(&lock);
		if(value == get(mid, "binary")){
			pthread_mutex_unlock(&lock);
			return mid;
		}
		pthread_mutex_unlock(&lock);
		pthread_mutex_lock(&lock);
		if(value < get(mid, "binary")){
			pthread_mutex_unlock(&lock);
			return binarySearch(lower, mid - 1, value);
		}
		pthread_mutex_unlock(&lock);
		return binarySearch(mid + 1, upper, value);
	}
	return - 1;
}

void *binary_thread(void *arg){
	unsigned int upper = virtualSize * frameSize - 1;
	printf("binarySearch for 2126918223 : %d\n", binarySearch(0, upper, 2126918223));
	printf("binarySearch for 1263774204: %d\n", binarySearch(0, upper, 1263774204));
	printf("binarySearch for 43533414: %d\n", binarySearch(0, upper, 43533414));
	printf("binarySearch for -266605: %d\n", binarySearch(0, upper, -266605));
	printf("binarySearch for 700034: %d\n", binarySearch(0, upper, 700034));
	
	return(NULL);
}

void *linear_thread(void *arg){
	printf("linearSearch for 2126918223 : %d\n", linearSearch(2126918223));
	printf("linearSearch for 1263774204: %d\n", linearSearch(1263774204));
	printf("linearSearch for 43533414: %d\n", linearSearch(43533414));
	printf("linearSearch for -266605: %d\n", linearSearch(-266605));
	printf("linearSearch for 700034: %d\n", linearSearch(700034));
	return(NULL);
}

void *quick_thread(void *arg){
	unsigned int lower = virtualSize * frameSize / 2;
	quickSort(0, lower - 1);
	return(NULL);
}

void *bubble_thread(void *arg){
	unsigned int lower = virtualSize * frameSize / 2;
	unsigned int upper = virtualSize * frameSize;
	unsigned int i;
	unsigned int j;
	int first;
	int second;
	for(i = 0; i < upper - 1; i++){
		for(j = lower; j < upper - i - 1; j++){
			pthread_mutex_lock(&lock);
			first = get(j, "Bubble");
			second = get(j + 1, "Bubble");
			if(first > second){
				set(j, second, "Bubble");
				set(j+1, first, "Bubble");
			}
			pthread_mutex_unlock(&lock);
		}
	}
	return(NULL);
}

int main(int argc, char **argv){
	pthread_t bubble_ptid;
	pthread_t quick_ptid;
	pthread_t linear_ptid;
	pthread_t binary_ptid;
	unsigned int pageReplacements = 0;

	getArgs(argc, argv);
	initTable();
	initPhysicalMemory();
	initVirtualMemory();
	unsigned int i;
	unsigned int upper = virtualSize * frameSize;

	if(print == 1){
		printf("-----AFTER INIT------\n");
		for(i = 0; i < upper; i++){
			printf("%d. element: %d\n", i, get(i, "a"));
		}
	}	
	
	
	if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        exit(EXIT_FAILURE);
    }

	pthread_create(&bubble_ptid, NULL, &bubble_thread, NULL);
	pthread_create(&quick_ptid, NULL, &quick_thread, NULL);

	pthread_join(bubble_ptid, NULL);
	pthread_join(quick_ptid, NULL);

	merge();
	if(print == 1){
		printf("-----AFTER MERGE------\n");
		for(i = 0; i < upper; i++){
			printf("%d. element: %d\n", i, get(i, "a"));
		}
	}	

	pthread_create(&linear_ptid, NULL, &linear_thread, NULL);
	pthread_create(&binary_ptid, NULL, &binary_thread, NULL);

	pthread_join(linear_ptid, NULL);
	pthread_join(binary_ptid, NULL);

	pthread_mutex_destroy(&lock);

	for(i = 0; i < 6; i++){
		if(i == FILL){
			printf("Datas of Fill Thread:\n");
		}else if(i == QUICK){
			printf("Datas of Quick Sort Thread:\n");
		}else if(i == BUBBLE){
			printf("Datas of Bubble Sort Thread:\n");
		}else if(i == MERGE){
			printf("Datas of Merge Thread:\n");
		}else if(i == LINEAR){
			printf("Datas of Linear Search Thread:\n");
		}else if(i == BINARY){
			printf("Datas of Binary Search Thread:\n");
		}
		printf("reads: %d, \t writes: %d, \t page misses: %d, \t page replacements: %d, \t disk reads: %d, \t disk writes: %d,\n",
			threadDatas[i][READ], threadDatas[i][WRITE], threadDatas[i][MISS], threadDatas[i][REPLACEMENT], threadDatas[i][DISK_READ], threadDatas[i][DISK_WRITE]);
		pageReplacements += threadDatas[i][REPLACEMENT];
	}
	saveToVirtual();
	freeDatas();
	return pageReplacements;
}