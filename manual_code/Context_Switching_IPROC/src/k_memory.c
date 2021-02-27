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
#include "k_queue.h"
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
    
    freeList = (MEM_BLK *) p_end; // set freeList to first free block
    heap_start = (U32*) p_end;
    
    // start from p_end, set value at mem_it to the memory address of next block (aka mem_it + MEM_BLK_SIZE)
    
    #ifndef TEST_2_MEM_BLK
    const int mem_num_blks = MEM_NUM_BLKS;
    #else
    const int mem_num_blks = 2;
    #endif

    int num_blocks = 1;
    mem_it = (MEM_BLK *) p_end;
    while (num_blocks < mem_num_blks && mem_it < (MEM_BLK *) gp_stack - 1) {
        mem_it->mp_next = mem_it + 1;
        mem_it++;
        num_blocks++;
    }
    
    mem_it->mp_next = NULL; // set end of freeList
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
        freeList = freeBlock->mp_next;
        // could set freeBlock->mp_next to like null or something
        return freeBlock;
    }
}

void *k_request_memory_block(void) {
    uint32_t ctrl = __get_CONTROL();
    __set_CONTROL(0);
    PCB * curr_proc = gp_current_process;
    MEM_BLK *prevAddr = curr_proc->m_mem_blk;
    
#ifdef DEBUG_0 
    printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */
    // Might need to switch user modes??
    // atomic(on) -> maybe add a safety flag? (a bit?)
    
    curr_proc->m_mem_blk = k_request_memory_block_nb();
    
    while(curr_proc->m_mem_blk == NULL) {
        gp_current_process->m_state = BLOCKED_ON_RESOURCE; //set process state
        
        // add gp_current_process to the blockedPBC queue
        pq_insert(&proc_blocked_queue, gp_current_process);
        
        // release processor but don't add ourselves to ready queue
        PCB *p_pcb_old = gp_current_process;
        gp_current_process = scheduler();
        #ifdef DEBUG_0  
        printf("Scheduler returned pid: %d\n", gp_current_process->m_pid);
        #endif
        process_switch(p_pcb_old);
    }
    
    // atomic(off);
    curr_proc->m_mem_blk->mp_next = prevAddr;
    __set_CONTROL(ctrl);
    return curr_proc->m_mem_blk->block;
}

int k_release_memory_block(void *p_mem_blk) {
    uint32_t ctrl = __get_CONTROL();
    __set_CONTROL(0);
    // dequeue a blocked-on-memory PCB
    PCB * first = pq_remove(&proc_blocked_queue);
    
#ifdef DEBUG_0 
    printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif /* ! DEBUG_0 */
    U8 *mem_addr = (U8*) p_mem_blk;
    
    if ((U32*) mem_addr < heap_start || (U32*) mem_addr >= gp_stack) {
        __set_CONTROL(ctrl);
        return RTX_ERR;
    }
    
    MEM_BLK* target_blk = q_remove_by_addr(&(gp_current_process->m_mem_blk), mem_addr);
    
    if (target_blk == NULL) {
            __set_CONTROL(ctrl);
            return RTX_ERR; // did not find the block with mem_addr
    }
    
    // target_blk should be the proc corresponding to mem_addr
    
    if (first == NULL) {
        // add new blocks to the front of our freeList
        target_blk->mp_next = freeList;
        freeList = target_blk;
    } else  {
        first->m_state = RDY;
        
        target_blk->mp_next = first->m_mem_blk;
        first->m_mem_blk = target_blk;
        
        if(first->m_priority < gp_current_process->m_priority){
            PCB * p_pcb_old = gp_current_process;
            gp_current_process = first;
            pq_insert_front_ready(p_pcb_old); //insert p_pcb_old to front of that prio in the queue
            process_switch(p_pcb_old);
        } else {
            pq_insert_ready(first);
        }
    }
    __set_CONTROL(ctrl);
    return RTX_OK;
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
