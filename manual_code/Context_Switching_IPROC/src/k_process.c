/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO SE 350 RTX LAB  
 *
 *           Copyright 2020-2021 Yiqing Huang and Thomas Reidemeister
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
 * @file        k_process.c 
 * @brief       kernel process management C file 
 *              
 * @version     V1.2021.01
 * @authors     Yiqing Huang, Thomas Reidemeister
 * @date        2021 JAN
 * @note        The example code shows one way of implementing context switching.
 *              The code only has minimal sanity check. 
 *              There is no stack overflow check.
 *              The implementation assumes only two simple user processes and 
 *              NO HARDWARE INTERRUPTS. 
 *              The purpose is to show one way of doing context switch
 *              under stated assumptions. 
 *              These assumptions are not true in the required RTX Project!!!
 *              If you decide to use this piece of code, 
 *              you need to understand the assumptions and the limitations.
 *
 *****************************************************************************/

#include "k_process.h"
#include "nullProc.h"
#include "k_rtx.h"
/*
 *==========================================================================
 *                            GLOBAL VARIABLES
 *==========================================================================
 */
 
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */
PCB *gp_pcb_timer_iproc = NULL; /* points to Timer iprocess pcb */ 
PCB *gp_pcb_interrupted;        /* interrupted process's pcb    */


/* process initialization table */
PROC_INIT g_proc_table[NUM_TEST_PROCS + 1]; /* user test procs + timer_iproc */

PCB * proc_ready_queue = NULL;

void  pq_insert_ready(PCB * proc) {
    if (proc->m_state != NEW)
		proc->m_state = RDY;
	pq_insert(&proc_ready_queue, proc);
}

void pq_insert_front_ready(PCB * proc) {
    if (proc->m_state != NEW)
		proc->m_state = RDY;
	pq_insert_front(&proc_ready_queue, proc);
}

PCB *pq_remove_by_pid_ready(int pid) {
	return pq_remove_by_pid(&proc_ready_queue, pid);
}

/**************************************************************************//**
 * @biref initialize all processes in the system
 * @note  We assume there are only two user processes in the system in 
 *        this example starter code.
 *        The preprocessor ECE350_DEMO should be defined
 *****************************************************************************/
void process_init(PROC_INIT *proc_info, int num) 
{
    int i;
    U32 *sp;
  
    /* fill out the initialization table */
#ifdef SE350_DEMO
    for ( i = 0; i < 2; i++ ) {
#else
    for ( i = 0; i < num; i++ ) {
#endif /* SE350_DEMO */
        g_proc_table[i].m_pid        = proc_info[i].m_pid;
        g_proc_table[i].m_stack_size = proc_info[i].m_stack_size;
        g_proc_table[i].mpf_start_pc = proc_info[i].mpf_start_pc;
		g_proc_table[i].m_priority = proc_info[i].m_priority;
    }
    
	int j;
    /* initilize exception stack frame (i.e. initial context) for each process */
	sp = alloc_stack(USR_SZ_STACK);
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&nullProc); // PC contains the entry point of the process
	for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	gp_pcbs[0]->m_pid = PID_NULL;
	gp_pcbs[0]->m_state = NEW;
	gp_pcbs[0]->mp_sp = sp;
	gp_pcbs[0]->m_priority = PRI_NULL;
	gp_pcbs[0]->m_mem_blk = NULL;
	pq_insert(&proc_ready_queue, gp_pcbs[0]);
  
    for ( i = 1; i <= num; i++ ) {
        int j;
        (gp_pcbs[i])->m_pid = (g_proc_table[i-1]).m_pid;
        (gp_pcbs[i])->m_state = NEW;
        
        sp = alloc_stack((g_proc_table[i-1]).m_stack_size);
        *(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
        *(--sp)  = (U32)((g_proc_table[i-1]).mpf_start_pc); // PC contains the entry point of the process
        for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
            *(--sp) = 0x0;
        }
        (gp_pcbs[i])->mp_sp = sp;
		(gp_pcbs[i])->m_priority = (g_proc_table[i-1]).m_priority;
		(gp_pcbs[i])->m_mem_blk = NULL;

		pq_insert(&proc_ready_queue, gp_pcbs[i]);
    }

#ifdef TIMER_IPROC
    /* Timer i-proc initialization */
    gp_pcb_timer_iproc = gp_pcbs[i];
    gp_pcb_timer_iproc->m_pid = PID_TIMER_IPROC;
    gp_pcb_timer_iproc->m_state = IPROC;
    gp_pcb_timer_iproc->mp_sp = alloc_stack(STACK_SIZE_IPROC);
    /* NOTE we do not need to create exception stack frame for an IPROC
       since they are running in handler mode and never get into the handler
       mode from the thread mode and they never exit from the handler mode
       back to thread mode either 
    */
#endif
}

/**************************************************************************//**
 * @brief   scheduler, pick the pid of the next to run process
 * @return  PCB pointer of the next to run process
 *          NULL if error happens
 * @post    if gp_current_process was NULL, then it gets set to pcbs[0].
 *          No other effect on other global variables.
 *****************************************************************************/

PCB *scheduler(void)
{
	// base case for start up (no process running)
    if (gp_current_process == NULL) {
		PCB *front = pq_remove(&proc_ready_queue);
		gp_current_process = front;
		return front;
    }

	// pq_print(&proc_ready_queue);
	if (proc_ready_queue == NULL) {
		return NULL; // null process
	} else {
		return proc_ready_queue;
	}
}

/**************************************************************************//**
 * @brief     pick the next pcb when periodically 
 *            SIM_TARGET: period = 200 clock ticks
 *            otherwise:   period = 6000 clock ticks
 * @return    PCB pointer of the next to run process
 *            NULL if error happens
 * @post      if gp_current_process was NULL, then it gets set to pcbs[0].
 *            No other effect on other global variables.
 * @attention You should write your own scheduler, do not use this one in your
 *            project. This is only to demonstrate how a timer interrupt
 *            can affect the scheduling decision.
 *****************************************************************************/

PCB *scheduler_tms(void)
{
#ifdef SIM_TARGET
    if ( g_timer_count %200 == 0 ) {
#else
    if ( g_timer_count %12000 == 0 ) {
#endif
        return scheduler();
    } 
    return gp_current_process;
}

/**************************************************************************//**
 * @brief   switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 * @param   p_pcb_old, the old pcb that was in RUN
 * @return  RTX_OK upon success
 *          RTX_ERR upon failure
 * @pre     p_pcb_old and gp_current_process are pointing to valid PCBs.
 * @post    if gp_current_process was NULL, then it gets set to pcbs[0].
 *          No other effect on other global variables.
 *****************************************************************************/

int process_switch(PCB *p_pcb_old) 
{
    PROC_STATE_E state;
    
    state = gp_current_process->m_state;

    if (state == NEW) {
        if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
            p_pcb_old->mp_sp = (U32 *) __get_MSP();

			pq_remove_by_pid(&proc_ready_queue, gp_current_process->m_pid);
        }
        gp_current_process->m_state = RUN;
        __set_MSP((U32) gp_current_process->mp_sp);
        __rte();  // pop exception stack frame from the stack for a new processes
    } 
    
    /* The following will only execute if the if block above is FALSE */

    if (gp_current_process != p_pcb_old) {
        if (state == RDY) {
            p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
            gp_current_process->m_state = RUN;
            __set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack    
        
			pq_remove_by_pid(&proc_ready_queue, gp_current_process->m_pid);
		} else {
            gp_current_process = p_pcb_old; // revert back to the old proc on error
            return RTX_ERR;
        } 
    }
    return RTX_OK;
}

/**************************************************************************//**
 * @brief   release_processor(). 
 * @return  RTX_ERR on error and zero on success
 * @post    gp_current_process gets updated to next to run process
 *****************************************************************************/

int k_release_processor(void)
{
	PCB *p_pcb_old = gp_current_process;
	PCB *p_pcb_next = scheduler();
	
	if ( p_pcb_next == NULL  ) { // shouldn't happen since the null proc is always ready
		return RTX_ERR;
	}
	
	if (p_pcb_old == NULL) {
		p_pcb_old = p_pcb_next; // should only happen on the initial call of this func (when gp_curr_proc was NULL)
		process_switch(p_pcb_old);
		return RTX_OK;
	}
	
	if (p_pcb_old->m_priority >= p_pcb_next->m_priority) { // only release if there is a proc with same or greater priority
		#ifdef DEBUG_0
		printf("Pid %d is replaced by pid %d\n", p_pcb_old->m_pid, p_pcb_next->m_pid);
		#endif
		gp_current_process = p_pcb_next;
		pq_insert_ready(p_pcb_old); // add old proc to ready queue
		process_switch(p_pcb_old);
	} else {
		#ifdef DEBUG_0
		printf("Did not release pid %d with pid %d\n", p_pcb_old->m_pid, p_pcb_next->m_pid);
		#endif
	}
	return RTX_OK;
}


// release current process if it should be preempted
int release_if_preempted(void) {
	PCB *p_pcb_old = gp_current_process;
	PCB *p_pcb_next = scheduler();
	
	if ( p_pcb_next == NULL  ) {
		p_pcb_next = p_pcb_old; // revert back to the old process
		return RTX_OK; // not necessarily an error case?
	}
	
	if ( p_pcb_old == NULL ) { // shouldn't ever happen
		p_pcb_old = p_pcb_next;
	}
	
	if (p_pcb_old->m_priority >= p_pcb_next->m_priority) { // don't preempt if same priority
		#ifdef DEBUG_0
		printf("Pid %d is preempted by pid %d\n", p_pcb_old->m_pid, p_pcb_next->m_pid);
		#endif
		
		gp_current_process = p_pcb_next;
		pq_insert_ready(p_pcb_old); // add old proc to ready queue
		process_switch(p_pcb_old);
	} else {
		#ifdef DEBUG_0
		printf("Did not preempt pid %d with pid %d\n", p_pcb_old->m_pid, p_pcb_next->m_pid);
		#endif
	}
	return RTX_OK;
}

// get a PCB by its pid from gp_pcbs array
PCB * get_pcb_by_pid(int pid) {
	for(int i = 0; i < NUM_TEST_PROCS + 1; i++){ // might need to store num from proc_init and use that instead of NUM_TEST_PROCS
		if (gp_pcbs[i] && gp_pcbs[i]->m_pid == pid) {
			return gp_pcbs[i];
		}
	}
	return NULL;
}

// get a process's priority
int k_get_process_priority(int pid) {
	uint32_t ctrl = __get_CONTROL();
	__set_CONTROL(0);
	if (pid == PID_NULL) {
		__set_CONTROL(ctrl);
        return PRI_NULL;
	}
	
	PCB * proc = get_pcb_by_pid(pid);
	
	if (proc == NULL) {
		__set_CONTROL(ctrl);
		return RTX_ERR;
	}
    __set_CONTROL(ctrl);
	return proc->m_priority;
}

// set a process's priority
int k_set_process_priority(int pid, int prio) {
	uint32_t ctrl = __get_CONTROL();
	__set_CONTROL(0);
	// Return error if the process pid is the null process, or if the 
	//  process priority is invalid
	if (prio < HIGH || prio >= PRI_NULL || pid == PID_NULL) {
		__set_CONTROL(ctrl);
		return RTX_ERR;
	}
	
	// Find the PCB associated with the pid
	PCB * proc = get_pcb_by_pid(pid);
	
	// If the process doesn't exist, return error
	if (proc == NULL) {
		__set_CONTROL(ctrl);
		return RTX_ERR;
	}	

	// Save the previous priority, update the process priority
	int prevPrio = proc->m_priority;
	proc->m_priority = prio;
	
	// If priority is unchanged, return early and leave queues unchanged
	if (prevPrio == prio) {
		__set_CONTROL(ctrl);
		return RTX_OK;
	}
	
	if (gp_current_process->m_pid == pid) {
		// insert curr proc to ready queue if we changed its priority
		pq_insert_ready(gp_current_process);
	} else {
		// If the process exists in the ready queue, remove and reinsert it
		PCB * found = pq_remove_by_pid(&proc_ready_queue, pid);
		if (found != NULL) {
			pq_insert_ready(found);
			// Insert the current process in the ready queue as well, such that
			//   the scheduler can choose between the processes
			pq_insert_front_ready(gp_current_process);
		} else {
			// If the process exists in the blocked queue, remove it and then reinsert it
			found = pq_remove_by_pid_blocked(pid);
			if (found != NULL) {
				// Return early, as no premption should occur in this case
				pq_insert_blocked(found);
				__set_CONTROL(ctrl);
				return RTX_OK;
			}
		}
	}
	
	// Save the current process, and then see what the scheduler picks to determine premption
	//  This covers the case where we set our process lower another process priority, and
	//  the case where we set another process' priority higher than ours
	PCB * p_pcb_old = gp_current_process;
	PCB * p_pcb_next = scheduler();
	// If the scheduler still chose the current process out of the ready queue,
	//   we shouldn't preempt ourselves - reset our state to running
	if (p_pcb_next == p_pcb_old){
		// Remove the current process from where we inserted it in the ready queue,
		//  and reset its state to running
		pq_remove(&proc_ready_queue);
		gp_current_process->m_state = RUN;
	} else {
		// Otherwise, switch to the new process
		gp_current_process = p_pcb_next;
		process_switch(p_pcb_old);
	}

	__set_CONTROL(ctrl);
	return RTX_OK;
}

/**************************************************************************//**
 * @brief   run a new process based on the scheduler_tms() 
 * @return  RTX_ERR on error and zero on success
 * @post    gp_current_process gets updated to next to run process
 *****************************************************************************/
int k_run_new_process(void)
{
    PCB *p_new_pcb = NULL;
    PCB *p_old_pcb = gp_current_process;
    
    if (gp_current_process == NULL) {
        return RTX_ERR;
    }
    
    // making scheduling decision
    p_new_pcb = scheduler_tms();
    
    if ( p_new_pcb == gp_current_process ) {
        return RTX_OK;
    }
    
    if ( p_new_pcb == NULL) {
        return RTX_ERR;
    }
    
    gp_current_process = p_new_pcb;
    process_switch(p_old_pcb);
    return RTX_OK;
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
