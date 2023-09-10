/* SPIM S20 MIPS simulator.
   Execute SPIM syscalls, both in simulator and bare mode.
   Execute MIPS syscalls in bare mode, when running on MIPS systems.
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


#ifndef _WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef _WIN32
#include <io.h>
#endif

#include "spim.h"
#include "string-stream.h"
#include "inst.h"
#include "reg.h"
#include "mem.h"
#include "spim-utils.h"
#include "sym-tbl.h"
#include "syscall.h"

#include <stdexcept>
#include <iostream>
#include <vector>
using namespace std;

vector<struct Thread> threads;
vector<struct Mutex> mutexes;
int current = 0;
int robinIndex = 1;

#ifdef _WIN32
/* Windows has an handler that is invoked when an invalid argument is passed to a system
   call. https://msdn.microsoft.com/en-us/library/a9yf33zb(v=vs.110).aspx

   All good, except that the handler tries to invoke Watson and then kill spim with an exception.

   Override the handler to just report an error.
*/

#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>


void myInvalidParameterHandler(const wchar_t* expression,
   const wchar_t* function, 
   const wchar_t* file, 
   unsigned int line, 
   uintptr_t pReserved)
{
  if (function != NULL)
    {
      run_error ("Bad parameter to system call: %s\n", function);
    }
  else
    {
      run_error ("Bad parameter to system call\n");
    }
}

static _invalid_parameter_handler oldHandler;

void windowsParameterHandlingControl(int flag )
{
  static _invalid_parameter_handler oldHandler;
  static _invalid_parameter_handler newHandler = myInvalidParameterHandler;

  if (flag == 0)
    {
      oldHandler = _set_invalid_parameter_handler(newHandler);
      _CrtSetReportMode(_CRT_ASSERT, 0); // Disable the message box for assertions.
    }
  else
    {
      newHandler = _set_invalid_parameter_handler(oldHandler);
      _CrtSetReportMode(_CRT_ASSERT, 1);  // Enable the message box for assertions.
    }
}
#endif


/*You implement your handler here*/
void SPIM_timerHandler()
{
   // Implement your handler..
  try {
    if(threads.size() > 1){   
    //  printf("TIMER_HANDLER\n");
      change_thread(READY);
    } 
  } catch ( exception &e ) {
      cerr <<  endl << "Caught: " << e.what( ) << endl;
  };
   
}
/* Decides which syscall to execute or simulate.  Returns zero upon
   exit syscall and non-zero to continue execution. */
int
do_syscall ()
{
#ifdef _WIN32
    windowsParameterHandlingControl(0);
#endif

  /* Syscalls for the source-language version of SPIM.  These are easier to
     use than the real syscall and are portable to non-MIPS operating
     systems. */

  switch (R[REG_V0])
    {
    case PRINT_INT_SYSCALL:
      write_output (console_out, "%d", R[REG_A0]);
      break;

    case PRINT_FLOAT_SYSCALL:
      {
	float val = FPR_S (REG_FA0);

	write_output (console_out, "%.8f", val);
	break;
      }

    case PRINT_DOUBLE_SYSCALL:
      write_output (console_out, "%.18g", FPR[REG_FA0 / 2]);
      break;

    case PRINT_STRING_SYSCALL:
      write_output (console_out, "%s", mem_reference (R[REG_A0]));
      break;

    case READ_INT_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	R[REG_RES] = atol (str);
	break;
      }

    case READ_FLOAT_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	FPR_S (REG_FRES) = (float) atof (str);
	break;
      }

    case READ_DOUBLE_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	FPR [REG_FRES] = atof (str);
	break;
      }

    case READ_STRING_SYSCALL:
      {
	read_input ( (char *) mem_reference (R[REG_A0]), R[REG_A1]);
	data_modified = true;
	break;
      }

    case SBRK_SYSCALL:
      {
	mem_addr x = data_top;
	expand_data (R[REG_A0]);
	R[REG_RES] = x;
	data_modified = true;
	break;
      }

    case PRINT_CHARACTER_SYSCALL:
      write_output (console_out, "%c", R[REG_A0]);
      break;

    case READ_CHARACTER_SYSCALL:
      {
	static char str [2];

	read_input (str, 2);
	if (*str == '\0') *str = '\n';      /* makes xspim = spim */
	R[REG_RES] = (long) str[0];
	break;
      }

    case EXIT_SYSCALL:
      spim_return_value = 0;
      return (0);

    case EXIT2_SYSCALL:
      spim_return_value = R[REG_A0];	/* value passed to spim's exit() call */
      return (0);

    case OPEN_SYSCALL:
      {
#ifdef _WIN32
        R[REG_RES] = _open((char*)mem_reference (R[REG_A0]), R[REG_A1], R[REG_A2]);
#else
	R[REG_RES] = open((char*)mem_reference (R[REG_A0]), R[REG_A1], R[REG_A2]);
#endif
	break;
      }

    case READ_SYSCALL:
      {
	/* Test if address is valid */
	(void)mem_reference (R[REG_A1] + R[REG_A2] - 1);
#ifdef _WIN32
	R[REG_RES] = _read(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#else
	R[REG_RES] = read(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#endif
	data_modified = true;
	break;
      }

    case WRITE_SYSCALL:
      {
	/* Test if address is valid */
	(void)mem_reference (R[REG_A1] + R[REG_A2] - 1);
#ifdef _WIN32
	R[REG_RES] = _write(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#else
	R[REG_RES] = write(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#endif
	break;
      }

    case CLOSE_SYSCALL:
      {
#ifdef _WIN32
	R[REG_RES] = _close(R[REG_A0]);
#else
	R[REG_RES] = close(R[REG_A0]);
#endif
	break;
      }

    case INIT_SYSCALL:
      init();
      break;

    case CREATE_THREAD_SYSCALL:
      create_thread();
      break;

    case JOIN_THREAD_SYSCALL:
      join_thread();
      break;

    case THREAD_EXIT_SYSCALL:
      exit_thread();
      break;

    case MUTEX_INIT_SYSCALL:
      mutex_init();
      break;

    case MUTEX_LOCK_SYSCALL:
      mutex_lock();
      break;

    case MUTEX_UNLOCK_SYSCALL:
      mutex_unlock();
      break; 

    case PROCESS_EXIT_SYSCALL:
      process_exit();
      if(current == -1){
        spim_return_value = 0;
        return(0);
      }
      break; 

    default:
      run_error ("Unknown system call: %d\n", R[REG_V0]);
      break;
    }

#ifdef _WIN32
    windowsParameterHandlingControl(1);
#endif
  return (1);
}

void process_exit(){
  //printf("IN PROCESS EXIT SYSCALL\n");
  int done = 1;
  if(current == 0){
    int i = 1;
    int size = threads.size();
    for(i = 1; i < size; i++){
      if(threads.at(i).state != TERMINATED){
        threads.at(current).joinID = i;
        change_thread(BLOCK);
        PC -= BYTES_PER_WORD;
        done = 0;
        i = size;
      }
    }
    if(done == 1){
      for(i = size - 1; i >= 0; i--){
        free_thread(i);
        threads.pop_back();
      }
      current = -1;
    }  
  }
}

void free_thread(int id){
  free(threads.at(id).stack_seg);
  free(threads.at(id).FPR);
}

void mutex_init(){
//  printf("IN MUTEX INIT SYSCALL\n");
  struct Mutex mutex;
  mutex.id = mutexes.size();
  mutex.value = 1;
  mutex.lock = 0;
  R[REG_RES] = mutex.id;
  mutexes.push_back(mutex);
}

void mutex_unlock(){
//  printf("IN MUTEX UNLOCK SYSCALL\n");
  int id = R[REG_A0];
  int i;
  int size = (int) threads.size();
  if(mutexes.at(id).value == 0){
    for(i = 0; i < size; i++){
      if(threads.at(i).mutexID == id){
        threads.at(i).mutexID = -1;
        threads.at(i).state = READY;
        mutexes.at(id).lock -= 1;
        if(mutexes.at(id).lock < 0){
          mutexes.at(id).value = 1;
          mutexes.at(id).lock = 0;
        }
        i = threads.size();
      }
    }
  }
}

void mutex_lock(){
//  printf("IN MUTEX LOCK SYSCALL\n");
  int id = R[REG_A0];
  if(mutexes.at(id).value == 1){
    mutexes.at(id).value = 0;
  }else if(mutexes.at(id).value == 0){
    mutexes.at(id).lock += 1;
    threads.at(current).mutexID = id;
    PC += BYTES_PER_WORD;
    change_thread(BLOCK);
    PC -= BYTES_PER_WORD;
  }

}

void exit_thread(){
//  printf("IN EXIT THREAD SYSCALL\n");
  notify_for_join(threads.at(current).id);
  PC += BYTES_PER_WORD;
  change_thread(TERMINATED);
  PC -= BYTES_PER_WORD;
}

void join_thread(){
//  printf("IN JOIN THREAD SYSCALL\n");
  int id = R[REG_A0];
  if(threads.at(id).state != TERMINATED){
    threads.at(current).joinID = id;
    PC += BYTES_PER_WORD;
    change_thread(BLOCK);
    PC -= BYTES_PER_WORD;
  }
}

void notify_for_join(int id){
  int i;
  int size = (int) threads.size();
  for(i = 0; i < size; i++){
    if(threads.at(i).state == BLOCK && threads.at(i).joinID == id){
      threads.at(i).state = READY;
    }
  }
}

void create_thread(){
//  printf("IN CREATE THREAD SYSCALL\n");
  struct Thread thread;
  thread.id = threads.size();
  strcpy(thread.name, "thread");
  thread.state = READY;
  thread.PC = R[REG_A0];    
  thread.nPC = R[REG_A0];
  thread.FPR = (double *)xmalloc(FPR_LENGTH * sizeof(double));
  thread.FGR = (float *)FPR;
  thread.FWR = (int *)FPR;
  thread.stack_seg = (mem_word *)xmalloc(STACK_SIZE);
  memcpy(thread.stack_seg, stack_seg, STACK_SIZE);
  thread.stack_seg_h = (short *) thread.stack_seg;
  thread.stack_seg_b = (BYTE_TYPE *) thread.stack_seg;
  thread.stack_bot = stack_bot;
  int i;
  for(i = 0; i < 32; i++){
    thread.R[i] = 0;
  }
  thread.R[REG_SP] = STACK_TOP - BYTES_PER_WORD - 4096;
  R[REG_RES] = thread.id;
  threads.push_back(thread);
}

void change_thread(enum State state){
  printf("-----------CONTEXT SWITCH-----------\n");
  int a = round_robin();
  if(a != -1){
    threads.at(current).state = state;
    threads.at(a).state = RUNNING;
    save_thread();
    current = a;
    load_thread();
  }
  print_threads_infos();
}

int round_robin(){
  int i = robinIndex;
  int j = 0;
  int size = (int) threads.size();
  for(j = 0; j < size; j++){
    if(i != current && threads.at(i).state == READY){
      robinIndex = i + 1;
      robinIndex %= size;
      return i;
    }
    i++;
    i %= size;
  }
  return -1;
}

void save_thread(){
  int i = 0;
  memcpy(threads.at(current).R, R, sizeof(R));
  memcpy(threads.at(current).CCR, CCR, sizeof(CCR));
  memcpy(threads.at(current).CPR, CPR, sizeof(CPR));
  threads.at(current).PC = PC;
  threads.at(current).nPC = nPC;
  threads.at(current).HI = HI;
  threads.at(current).LO = LO;
  threads.at(current).stack_seg = stack_seg;
  threads.at(current).stack_seg = stack_seg;
  threads.at(current).stack_seg_h = stack_seg_h;
  threads.at(current).stack_seg_b = stack_seg_b;
  threads.at(current).stack_bot = stack_bot;
  threads.at(current).gp_midpoint = gp_midpoint;
  for(i = 0; i < FPR_LENGTH; i++){
    threads.at(current).FPR[i] = FPR[i];
  }
  threads.at(current).FGR = FGR;
  threads.at(current).FWR = FWR;
}

void load_thread(){
  int i = 0;
  memcpy(R, threads.at(current).R, sizeof(R));
  memcpy(CCR, threads.at(current).CCR, sizeof(CCR));
  memcpy(CPR, threads.at(current).CPR, sizeof(CPR));
  PC = threads.at(current).PC;
  nPC = threads.at(current).nPC;
  HI = threads.at(current).HI;
  LO = threads.at(current).LO;
  stack_seg = threads.at(current).stack_seg;
  stack_seg = threads.at(current).stack_seg;
  stack_seg_h = threads.at(current).stack_seg_h; 
  stack_seg_b = threads.at(current).stack_seg_b;
  stack_bot = threads.at(current).stack_bot;
  gp_midpoint = threads.at(current).gp_midpoint;
  for(i = 0; i < FPR_LENGTH; i++){
    FPR[i] = threads.at(current).FPR[i];
  }
  FGR = threads.at(current).FGR;
  FWR = threads.at(current).FWR;
}

void init(){
  bool read_result;
  char *str = str_copy((char*)mem_reference(R[REG_A0]));
  int len = strlen(str);
  str[len-1] = '\0';
  initialize_world(DEFAULT_EXCEPTION_HANDLER, false);
  initialize_run_stack(0, NULL);
  read_result = read_assembly_file(str);

  if(read_result == true){
    create_thread();
    PC = find_symbol_address("main") - 4;
    threads.at(current).PC = PC;
    nPC = PC;
    threads.at(current).nPC = nPC;
    threads.at(current).state = RUNNING;
  }else{
    write_output(console_out, "There is no file with name %s\n", str);
    free(str);
    spim_return_value = 0;
  }
  free(str);
  spim_return_value = 0;
}

void print_threads_infos(){
  int i;
  int size = (int) threads.size();
  for(i = 0; i < size; i++){
    printf("Thread ID: %d, Program Counter: %d, Stack Pointer adress: %x, ",
      threads.at(i).id,
      threads.at(i).PC,
      threads.at(i).R[REG_SP]);
    if(threads.at(i).state == READY){
      printf("State: READY\n");
    }else if(threads.at(i).state == RUNNING){
      printf("State: RUNNING\n");
    }else if(threads.at(i).state == BLOCK){
      printf("State: BLOCK\n");
    }else if(threads.at(i).state == TERMINATED){
      printf("State: TERMINATED\n");
    }
  }
}

void
handle_exception ()
{
  if (!quiet && CP0_ExCode != ExcCode_Int)
    error ("Exception occurred at PC=0x%08x\n", CP0_EPC);

  exception_occurred = 0;
  PC = EXCEPTION_ADDR;

  switch (CP0_ExCode)
    {
    case ExcCode_Int:
      break;

    case ExcCode_AdEL:
      if (!quiet)
	error ("  Unaligned address in inst/data fetch: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_AdES:
      if (!quiet)
	error ("  Unaligned address in store: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_IBE:
      if (!quiet)
	error ("  Bad address in text read: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_DBE:
      if (!quiet)
  error ("  Bad address in data/stack read: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_Sys:
      if (!quiet)
	error ("  Error in syscall\n");
      break;

    case ExcCode_Bp:
      exception_occurred = 0;
      return;

    case ExcCode_RI:
      if (!quiet)
	error ("  Reserved instruction execution\n");
      break;

    case ExcCode_CpU:
      if (!quiet)
	error ("  Coprocessor unuable\n");
      break;

    case ExcCode_Ov:
      if (!quiet)
	error ("  Arithmetic overflow\n");
      break;

    case ExcCode_Tr:
      if (!quiet)
	error ("  Trap\n");
      break;

    case ExcCode_FPE:
      if (!quiet)
	error ("  Floating point\n");
      break;

    default:
      if (!quiet)
	error ("Unknown exception: %d\n", CP0_ExCode);
      break;
    }
}
