#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#define OLD_AGE 25
#define CLOCK_INTERRUPT 40

#define FILL 0
#define QUICK 1
#define BUBBLE 2
#define MERGE 3
#define LINEAR 4
#define BINARY 5

#define READ 0
#define WRITE 1
#define MISS 2
#define REPLACEMENT 3
#define DISK_READ 4
#define DISK_WRITE 5

enum TableType{
	REGULAR,
	INVERTED
};

struct Args{
	int frameSize;
	int numPhysical;
	int numVirtual;
	char *pageReplacement;
	enum TableType tableType;
	int pageTablePrintInt;
	char *filename;
};

struct PageTableEntry{
	int present;
	int reference;		
	int modified;		
	int frameNumber;
	unsigned int usedTime;		
	unsigned int loadTime;	    
};

struct PageTable{
	struct PageTableEntry *pages;
};

void saveToVirtual();
void freeDatas();
void printPageTable();
void updateThreadData(char *tName, int data);
void getArgs(int argc, char **argv);
void pageFault(unsigned int index, char *tName);
void reset_references();
void changeDataOnPhysical(unsigned int index, int value);
void set(unsigned int index, int value, char *tName);
void getVMtoPhysical(unsigned int index, unsigned int physicalPage);
void updateVirtualMem(unsigned int pageIndex, unsigned int physicalPage);
int FIFO(unsigned int index, char *tName);
int NRU(unsigned int index, char *tName);
int WSC(unsigned int index, char *tName);
int SC(unsigned int index, char *tName);
int LRU(unsigned int index, char *tName);
int get(unsigned int index, char *tName);
void initTable();
void initPhysicalMemory();
void initVirtualMemory();

#endif