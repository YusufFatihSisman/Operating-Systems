/* SPIM S20 MIPS simulator.
   Execute SPIM syscalls, both in simulator and bare mode.

   Copyright (c) 1990-2010, James R. Larus.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of the James R. Larus nor the names of its contributors may be
   used to endorse or promote products derived from this software without specific
   prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define NAME_SIZE 256

enum State{
	READY,
	RUNNING,
	BLOCK,
	TERMINATED
};

struct Thread{
	int id;
	int joinID;
	int mutexID;
	char name[NAME_SIZE];
	enum State state;
	mem_addr PC, nPC;
	double *FPR;		
	float *FGR;		
	int *FWR;
	reg_word R[32];
	reg_word HI, LO;
	reg_word CCR[4][32], CPR[4][32];
	mem_word *stack_seg;
	short *stack_seg_h;
	BYTE_TYPE *stack_seg_b;
	mem_addr stack_bot;
	mem_addr gp_midpoint;
};

struct Mutex{
	int id;
	int value;
	int lock;
};

void init();
void change_thread(enum State state);
int round_robin();
void save_thread();
void load_thread();
void create_thread();
void join_thread();
void notify_for_join(int id);
void exit_thread();
void print_threads_infos();
void mutex_init();
void mutex_lock();
void mutex_unlock();
void process_exit();
void free_thread(int id);


/* Exported functions. */
void SPIM_timerHandler();
int do_syscall ();
void handle_exception ();

#define PRINT_INT_SYSCALL	1
#define PRINT_FLOAT_SYSCALL	2
#define PRINT_DOUBLE_SYSCALL	3
#define PRINT_STRING_SYSCALL	4

#define READ_INT_SYSCALL	5
#define READ_FLOAT_SYSCALL	6
#define READ_DOUBLE_SYSCALL	7
#define READ_STRING_SYSCALL	8

#define SBRK_SYSCALL		9

#define EXIT_SYSCALL		10

#define PRINT_CHARACTER_SYSCALL	11
#define READ_CHARACTER_SYSCALL	12

#define OPEN_SYSCALL		13
#define READ_SYSCALL		14
#define WRITE_SYSCALL		15
#define CLOSE_SYSCALL		16

#define EXIT2_SYSCALL		17

#define INIT_SYSCALL 18
#define CREATE_THREAD_SYSCALL 19
#define JOIN_THREAD_SYSCALL 20
#define THREAD_EXIT_SYSCALL 21
#define MUTEX_INIT_SYSCALL 22
#define MUTEX_LOCK_SYSCALL 23
#define MUTEX_UNLOCK_SYSCALL 24
#define PROCESS_EXIT_SYSCALL 25

