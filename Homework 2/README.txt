Running of Program
	syscall.cpp and syscall.h must be copied to spimsimulator-code-r739/CPU directory.
	
	SPIMOS_GTU_1.s, SPIMOS_GTU_2.s, SPIMOS_GTU_3.s, MergeSort.s, ProducerConsumer.s  
and  ProducerConsumerWithoutMutex.s must be copied to spimsimulator-code-r739/spim directory.

	After goes spimsimulator-code-r739/spim directory and run make:

	./spim -file SPIMOS_GTU_1.s   runs merge sort part.
	./spim -file SPIMOS_GTU_2.s   runs producer consumer with mutex part.
	./spim -file SPIMOS_GTU_3.s   runs producer consumer without mutex part.
