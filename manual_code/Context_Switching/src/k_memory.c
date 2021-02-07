/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO SE 350 RTX LAB  
 *
 *                     Copyright 2020-2021 Yiqing Huang
 *                          All rights reserved.
 *---------------------------------------------------------------------------
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice and the following disclaimer.
 *
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
 

/**************************************************************************//**
 * @file        k_memory.c
 * @brief       kernel memory managment routines starter source code 
 *              
 * @version     V1.2021.01
 * @authors     Yiqing Huang
 * @date        2021 JAN
 * @attention   only two processes are initialized, not all six user test procs
 *****************************************************************************/

#include "k_memory.h"
#include "k_rtx.h"

/*
 *==========================================================================
 *                            GLOBAL VARIABLES
 *==========================================================================
 */

U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address        */
	           /* stack grows down. Fully decremental stack             */

U32 *heap_start;
						 
// Pointer to the first free block in the heap
MEM_BLK *freeList;
PCB *proc_blocked_queue;


void pq_insert_blocked(PCB * proc) {
	pq_insert(&proc_blocked_queue, proc);
}

PCB* pq_remove_by_pid_blocked(int pid) {
	return pq_remove_by_pid(&proc_blocked_queue, pid);
}

/**************************************************************************//**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|  
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|          
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address

*/
/**************************************************************************//**
 * @brief   	initialize the memory
 * @details     allocating memory for PCBs
 *              initial stack base pointer for processes
 *****************************************************************************/
void memory_init(void)
{
	U8 *p_end = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;
	int i;
	MEM_BLK *mem_it; // temp iterator for the heap
  
	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += (NUM_TEST_PROCS + 1) * sizeof(PCB *);
	
	// for null proc
	// p_end += sizeof(PCB *);
	// gp_pcbs[0] = (PCB *)p_end;
	// p_end += sizeof(PCB);
  
	for ( i = 0; i < NUM_TEST_PROCS + 1; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB); 
	}
#ifdef DEBUG_0  
	printf("gp_pcbs[0] = 0x%x \r\n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \r\n", gp_pcbs[1]);
	printf("gp_pcbs[2] = 0x%x \r\n", gp_pcbs[2]);
	printf("gp_pcbs[3] = 0x%x \r\n", gp_pcbs[3]);
	printf("gp_pcbs[4] = 0x%x \r\n", gp_pcbs[4]);
	printf("gp_pcbs[5] = 0x%x \r\n", gp_pcbs[5]);
	printf("gp_pcbs[6] = 0x%x \r\n", gp_pcbs[6]);
#endif
	
	/* prepare for alloc_stack() to allocate memory for stacks */
	
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}
  
	/* allocate memory for heap, not implemented yet*/
  // mem block size = MEM_BLK_SIZE
	// #mem blocks = MEM_NUM_BLKS
	
	freeList = (MEM_BLK *) p_end; // set freeList to first free block
	heap_start = (U32*) p_end;
	
	// start from p_end, set value at mem_it to the memory address of next block (aka mem_it + MEM_BLK_SIZE)
	for (mem_it = (MEM_BLK *) p_end; mem_it < (MEM_BLK *)gp_stack - sizeof(MEM_BLK); mem_it += sizeof(MEM_BLK)) {
		mem_it->next = mem_it + sizeof(MEM_BLK);
	}
	
	mem_it->next = NULL; // set end of freeList
	proc_blocked_queue = NULL;
}

/**************************************************************************//**
 * @brief   	allocate stack for a process, align to 8 bytes boundary
 * @param       size stack size in bytes
 * @return      the base of the stack (i.e. high address)
 * @post        gp_stack is updated.
 *****************************************************************************/

U32 *alloc_stack(U32 size_b) 
{
	U32 *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */
	
	/* update gp_stack */
	gp_stack = (U32 *)((U8 *)sp - size_b);
	
	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack; 
	}
	return sp;
}
/*
 *==========================================================================
 *                            TO BE IMPLEMENTED
 *==========================================================================
 */

void *k_request_memory_block_nb(void) {
	MEM_BLK * freeBlock = freeList;
#ifdef DEBUG_0 
	printf("k_request_memory_block_nb: entering...\n");
#endif /* ! DEBUG_0 */
	
	if (freeBlock == NULL) {
		return NULL;
	} else {
		freeList = freeBlock->next;
		// could set freeBlock->next to like null or something
		return freeBlock;
	}
}

void *k_request_memory_block(void) {
	MEM_BLK *ptr;
#ifdef DEBUG_0 
	printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */
	// Might need to switch user modes??
	// atomic(on) -> maybe add a safety flag? (a bit?)
	while((ptr = k_request_memory_block_nb()) == NULL) {
		gp_current_process->m_state = BLOCKED_ON_RESOURCE; //set process state
		
		// add gp_current_process to the blockedPBC queue
		pq_insert(&proc_blocked_queue, gp_current_process);
		
		// release processor but don't want to add ourselves to ready queue
		PCB *p_pcb_old = gp_current_process;
		gp_current_process = scheduler();
		printf("Scheduler returned pid: %d\n", gp_current_process->m_pid);
		process_switch(p_pcb_old);
	}
	
	ptr->next = gp_current_process->m_mem_blk;
	gp_current_process->m_mem_blk = ptr;
	// atomic(off);
	return ptr->block;
}

int k_release_memory_block(void *p_mem_blk) {
	// dequeue a blocked-on-memory PCB
	PCB * first = pq_remove(&proc_blocked_queue);
	
#ifdef DEBUG_0 
	printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif /* ! DEBUG_0 */
	U8 *mem_addr = (U8*) p_mem_blk;
	
	if ((U32*) mem_addr < heap_start || (U32*) mem_addr >= gp_stack) {
		return RTX_ERR;
	}
	
	BOOL isOwner = FALSE;
	MEM_BLK* block_it = gp_current_process->m_mem_blk;
	while(block_it != NULL) {
		if (block_it->block == mem_addr) {
			isOwner = TRUE;
			break;
		}
		block_it = block_it->next;
	}
	
	// error when trying to release a block that the process does not own
	if (isOwner == FALSE) {
		printf("Pid %d tried to release a block it does not own: 0x%x\n", gp_current_process->m_pid, mem_addr);
		return RTX_ERR;
	}
	
	((MEM_BLK*)p_mem_blk)->next = freeList;
	freeList = p_mem_blk;
	if (first != NULL) {
		first->m_state = RDY;
		pq_insert_ready(first);
		
		// assign the mem_blk to the PCB -> gives control back to the proc that was blocked in k_request_memory_block
		
		return release_if_preempted();
	}
	return RTX_OK;
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
