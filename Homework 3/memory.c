#include "memory.h"
#include "helper.h"
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

unsigned int lastUsedCounter = 0;
unsigned int loadTimeCounter = 0;

struct Args args;

struct PageTable pageTable;
int *physicalMem;
int pageFrameCount = 0;

int physicalSize;
int virtualSize;
int frameSize;

int fd;

int threadDatas[6][6];
int accessCount = 0;

void updateThreadData(char *tName, int data){
	if(tName[0] == 'F'){
		threadDatas[FILL][data] += 1;
	}else if(tName[0] == 'Q'){
		threadDatas[QUICK][data] += 1;
	}else if(tName[0] == 'B'){
		threadDatas[BUBBLE][data] += 1;
	}else if(tName[0] == 'M'){
		threadDatas[MERGE][data] += 1;
	}else if(tName[0] == 'l'){
		threadDatas[LINEAR][data] += 1;
	}else if(tName[0] == 'b'){
		threadDatas[BINARY][data] += 1;
	}
}

void getArgs(int argc, char **argv){
	if(argc != 8){
		printf("You must pass 7 argument to this program.\n");
		exit(EXIT_FAILURE);
	}
	args.frameSize = atoi(argv[1]);
	frameSize = pow(2, args.frameSize);
	
	args.numPhysical = atoi(argv[2]);
	physicalSize = pow(2, args.numPhysical);
	
	args.numVirtual = atoi(argv[3]);
	virtualSize = pow(2, args.numVirtual);
	
	args.pageReplacement = argv[4];
	char *temp = argv[5];
	if(temp[0] == 'i'){
		args.tableType = INVERTED;
	}else{
		args.tableType = REGULAR;
	}
	args.pageTablePrintInt = atoi(argv[6]);
	args.filename = argv[7];
}

void reset_references(){
	unsigned int i;
	for(i = 0; i < virtualSize; i++){
		pageTable.pages[i].reference = 0;
	}
}

void changeDataOnPhysical(unsigned int index, int value){
	unsigned int pageIndex = index / frameSize;
	unsigned int physicalAddress = pageTable.pages[pageIndex].frameNumber * frameSize + index % frameSize;
	
	physicalMem[physicalAddress] = value;
	pageTable.pages[pageIndex].modified = 1;
	pageTable.pages[pageIndex].reference = 1;
	pageTable.pages[pageIndex].usedTime = lastUsedCounter++;
	if(lastUsedCounter % CLOCK_INTERRUPT == 0){
		reset_references();
	}
}

void set(unsigned int index, int value, char *tName){
	unsigned int pageIndex = index / frameSize;
	if(pageTable.pages[pageIndex].present == 1){
		changeDataOnPhysical(index, value);
		updateThreadData(tName, WRITE);
	}else{
		if(pageFrameCount < physicalSize){
			updateThreadData(tName, MISS);
			updateThreadData(tName, DISK_READ);
			pageTable.pages[pageIndex].frameNumber = pageFrameCount++;
			pageTable.pages[pageIndex].loadTime = loadTimeCounter++;
			pageTable.pages[pageIndex].present = 1;
			updateThreadData(tName, WRITE);
			changeDataOnPhysical(index, value);
		}else{
			pageFault(index, tName);
			updateThreadData(tName, MISS);
			updateThreadData(tName, WRITE);
			changeDataOnPhysical(index, value);
		}
	}
	accessCount++;
	if(accessCount >= args.pageTablePrintInt){
		accessCount = 0;
		printPageTable();
	}
}

void getVMtoPhysical(unsigned int index, unsigned int physicalPage){
	int fd = open(args.filename, O_RDWR | O_CREAT, 0644);
	int i;
	unsigned int pageIndex = index / frameSize;
	char binary[33];	
	pageTable.pages[pageIndex].present = 1;
	pageTable.pages[pageIndex].frameNumber = physicalPage;
	pageTable.pages[pageIndex].usedTime = lastUsedCounter++;
	pageTable.pages[pageIndex].loadTime = loadTimeCounter++;
	if(lastUsedCounter % CLOCK_INTERRUPT == 0){
		reset_references();
	}
		
	lseek(fd, pageIndex * frameSize * 33, SEEK_SET);
	for(i = 0; i < frameSize; i++){
		int size = read(fd, binary, 33);
		if(size == -1){
			fprintf(stderr, "Read is failed while trying to read %s", args.filename);
			exit(EXIT_FAILURE);
		}
		int num =  binaryToInt(binary);
		physicalMem[i + (physicalPage * frameSize)] = num;	
	}
	close(fd);
}

void updateVirtualMem(unsigned int pageIndex, unsigned int physicalPage){
	int i;
	char binary[33];
	fd = open(args.filename, O_RDWR | O_CREAT, 0644);
	if(fd == -1){
		fprintf(stderr, "Open is failed while trying to open %s", args.filename);
		exit(EXIT_FAILURE);
	}
	lseek(fd, pageIndex * frameSize * 33, SEEK_CUR);
	for(i = 0; i < frameSize; i++){
		intToBinary(physicalMem[i + (physicalPage * frameSize)], binary);
		write(fd, binary, 33);
	}
	close(fd);
}

int get(unsigned int index, char *tName){
	
	unsigned int pageIndex = index / frameSize;
		
	if(pageTable.pages[pageIndex].present == 1){
		updateThreadData(tName, READ);
		pageTable.pages[pageIndex].reference = 1;
		pageTable.pages[pageIndex].usedTime = lastUsedCounter++;
		if(lastUsedCounter % CLOCK_INTERRUPT == 0){
			reset_references();
		}
		return physicalMem[frameSize * pageTable.pages[pageIndex].frameNumber + index % frameSize];
	}else{
		updateThreadData(tName, READ);
		updateThreadData(tName, MISS);
		pageFault(index, tName);
		return physicalMem[frameSize * pageTable.pages[pageIndex].frameNumber + index % frameSize];
	}
	accessCount++;
	if(accessCount >= args.pageTablePrintInt){
		accessCount = 0;
		printPageTable();
	}
	
}

void pageFault(unsigned int index, char *tName){
	
	if(pageFrameCount < physicalSize){
		getVMtoPhysical(index, pageFrameCount);
		updateThreadData(tName, DISK_READ);
		pageFrameCount++;
	}else{
		unsigned int physicalPage;
		if(args.pageReplacement[0] == 'L'){
			physicalPage = LRU(index, tName);
		}else if(args.pageReplacement[0] == 'S'){
			physicalPage = SC(index, tName);
		}else if(args.pageReplacement[0] == 'W'){
			physicalPage = WSC(index, tName);
		}else if(args.pageReplacement[0] == 'N'){
			physicalPage = NRU(index, tName);
		}else if(args.pageReplacement[0] == 'F'){
			physicalPage = FIFO(index, tName);
		}
		getVMtoPhysical(index, physicalPage);
		updateThreadData(tName, DISK_READ);
		updateThreadData(tName, REPLACEMENT);
	}
}

int FIFO(unsigned int index, char *tName){
	unsigned int i;
	unsigned int minIndex;
	
	for(i = 0; i < virtualSize; i++){
		if(pageTable.pages[i].present == 1){
			minIndex = i;
			i = virtualSize;
		}		
	}
	i = minIndex + 1;
	
	while(i < virtualSize){
		if((pageTable.pages[i].present == 1) && (pageTable.pages[minIndex].loadTime > pageTable.pages[i].loadTime)){
			minIndex = i;
		}
		i++;
	}
	
	if(pageTable.pages[minIndex].modified == 1){	
		updateVirtualMem(minIndex, pageTable.pages[minIndex].frameNumber);
		updateThreadData(tName, DISK_WRITE);
	}
	pageTable.pages[minIndex].modified = 0;
	pageTable.pages[minIndex].present = 0;
	pageTable.pages[minIndex].reference = 0;
	pageTable.pages[minIndex].loadTime = 0;	
	pageTable.pages[minIndex].usedTime = 0;
	pageTable.pages[minIndex].loadTime = 0;
	return pageTable.pages[minIndex].frameNumber;
}

int NRU(unsigned int index, char *tName){
	unsigned int i = 0;
	int c = 0;
	unsigned int minIndex;
	
	for(i = 0; i < virtualSize; i++){
		if(pageTable.pages[i].present == 1){
			if(pageTable.pages[i].reference == 0 && pageTable.pages[i].modified == 0){
				minIndex = i;
				i = virtualSize;
			}else if(c < 3 && pageTable.pages[i].reference == 0 && pageTable.pages[i].modified == 1){
				minIndex = i;
				c = 3;
			}else if(c < 2 && pageTable.pages[i].reference == 1 && pageTable.pages[i].modified == 0){
				minIndex = i;
				c = 2;
			}else if(c == 0 && pageTable.pages[i].reference == 1 && pageTable.pages[i].modified == 1){
				minIndex = i;
			}
		}
	}

	if(pageTable.pages[minIndex].modified == 1){	
		updateVirtualMem(minIndex, pageTable.pages[minIndex].frameNumber);
		updateThreadData(tName, DISK_WRITE);
	}

	pageTable.pages[minIndex].modified = 0;
	pageTable.pages[minIndex].present = 0;
	pageTable.pages[minIndex].reference = 0;
	pageTable.pages[minIndex].loadTime = 0;	
	pageTable.pages[minIndex].usedTime = 0;
	pageTable.pages[minIndex].loadTime = 0;
	return pageTable.pages[minIndex].frameNumber;
}

int WSC(unsigned int index, char *tName){
	unsigned int i = 0;
	int j = 0;
	unsigned int tempIndex;
	unsigned int minIndex = -1;
	
	while(j < 2){
		for(i = 0; i < virtualSize; i++){
			if(pageTable.pages[i].present == 1){
				if(pageTable.pages[i].reference == 0 && pageTable.pages[i].modified == 0 && lastUsedCounter - pageTable.pages[i].usedTime > OLD_AGE){
					minIndex = i;
					break;
				}else if(pageTable.pages[i].reference == 0 && lastUsedCounter - pageTable.pages[i].usedTime > OLD_AGE){
					tempIndex = i;
				}else{
					pageTable.pages[i].reference = 0;
				}
			}
		}
		lastUsedCounter++;
		i = 0;
		if(!(j == 1 && tempIndex == -1)){
			j++;
		}
	}
	
	if(minIndex == -1){
		minIndex = tempIndex;
		updateVirtualMem(minIndex, pageTable.pages[minIndex].frameNumber);
		updateThreadData(tName, DISK_WRITE);
	}
	
	pageTable.pages[minIndex].modified = 0;
	pageTable.pages[minIndex].present = 0;
	pageTable.pages[minIndex].reference = 0;
	pageTable.pages[minIndex].loadTime = 0;	
	pageTable.pages[minIndex].usedTime = 0;
	pageTable.pages[minIndex].loadTime = 0;	
	return pageTable.pages[minIndex].frameNumber;
	
}

int SC(unsigned int index, char *tName){
	unsigned int i;
	unsigned int minIndex;
	
	while(1){
		for(i = 0; i < virtualSize; i++){
			if(pageTable.pages[i].present == 1){
				minIndex = i;
				i = virtualSize;
			}
		}
		for(i = 0; i < virtualSize; i++){
			if((pageTable.pages[i].present == 1) && (pageTable.pages[minIndex].loadTime > pageTable.pages[i].loadTime)){
				minIndex = i;
			}
		}
		if(pageTable.pages[minIndex].reference == 1){
			pageTable.pages[minIndex].reference = 0;
			pageTable.pages[minIndex].loadTime = loadTimeCounter++;
		}else{
			break;
		}	
	}
	if(pageTable.pages[minIndex].modified == 1){	
		updateVirtualMem(minIndex, pageTable.pages[minIndex].frameNumber);
		updateThreadData(tName, DISK_WRITE);
	}
	
	pageTable.pages[minIndex].modified = 0;
	pageTable.pages[minIndex].present = 0;
	pageTable.pages[minIndex].reference = 0;
	pageTable.pages[minIndex].loadTime = 0;	
	pageTable.pages[minIndex].usedTime = 0;
	pageTable.pages[minIndex].loadTime = 0;
	return pageTable.pages[minIndex].frameNumber;

}

int LRU(unsigned int index, char *tName){
	unsigned int i;
	unsigned int minIndex;
	
	for(i = 0; i < virtualSize; i++){
		if(pageTable.pages[i].present == 1){
			minIndex = i;
			i = virtualSize;
		}		
	}
	i = minIndex + 1;

	while(i < virtualSize){
		if((pageTable.pages[i].present == 1) && (pageTable.pages[minIndex].usedTime > pageTable.pages[i].usedTime)){
			minIndex = i;
		}
		i++;
	}
	
	if(pageTable.pages[minIndex].modified == 1){	
		updateVirtualMem(minIndex, pageTable.pages[minIndex].frameNumber);
		updateThreadData(tName, DISK_WRITE);
	}
	pageTable.pages[minIndex].modified = 0;
	pageTable.pages[minIndex].present = 0;
	pageTable.pages[minIndex].reference = 0;
	pageTable.pages[minIndex].loadTime = 0;	
	pageTable.pages[minIndex].usedTime = 0;
	pageTable.pages[minIndex].loadTime = 0;
	return pageTable.pages[minIndex].frameNumber;
}

void initTable(){
	pageTable.pages = (struct PageTableEntry *)calloc(virtualSize, sizeof(struct PageTableEntry));
	if(pageTable.pages == NULL){
		fprintf(stderr, "Calloc fail while trying to initialize page table\n");
		exit(EXIT_FAILURE);
	}
}

void initPhysicalMemory(){
	physicalMem = (int *) calloc(physicalSize * frameSize, sizeof(int));
	if(physicalMem == NULL){
		fprintf(stderr, "Calloc fail while trying initialize physical memory\n");
		exit(EXIT_FAILURE);
	} 
}

void initVirtualMemory(){
	int i;
	int num;
	fd = open(args.filename, O_RDWR | O_CREAT, 0644);
	if(fd == -1){
		fprintf(stderr, "Open is failed while trying to open %s", args.filename);
		exit(EXIT_FAILURE);
	}
	if(ftruncate(fd, virtualSize * frameSize * 33) == -1){
		fprintf(stderr, "Ftruncate fail\n");
		exit(EXIT_FAILURE);
	}
	srand(1000);
	for(i = 0; i < virtualSize * frameSize; i++){
		num = rand();
		set(i, num, "Fill");
	}
	close(fd);
}

void printPageTable(){
	unsigned int i;
	for(i = 0; i < virtualSize; i++){
		printf("%d. Page Informations:\n", i);
		if(pageTable.pages[i].present == 1){
			printf("\t present: %d,\t frameNumber: %d\n", pageTable.pages[i].present, pageTable.pages[i].frameNumber);
		}else{
			printf("\t present: %d,\t old frameNumber: %d\n", pageTable.pages[i].present, pageTable.pages[i].frameNumber);
		}
		printf("\t referenced: %d,\t modified: %d\n", pageTable.pages[i].reference, pageTable.pages[i].modified);
		printf("\t lastUsedTime: %d,\t loadTime: %d\n", pageTable.pages[i].usedTime, pageTable.pages[i].loadTime);
	}
}

void freeDatas(){
	free(pageTable.pages);
	free(physicalMem);
}

void saveToVirtual(){
	unsigned int i;
	for(i = 0; i < virtualSize; i++){
		if(pageTable.pages[i].present == 1 && pageTable.pages[i].modified == 1){
			updateVirtualMem(i, pageTable.pages[i].frameNumber);
		}
	}
}