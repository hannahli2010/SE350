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
 * @file        k_inc.h
 * @brief       Kernel Macros and Data Structure Header file          
 * @version     V1.2021.01
 * @authors     Yiqing Huang
 * @date        2021 JAN
 *
 * @note        all kernel .c files include this one
 *
 *****************************************************************************/
 
#ifndef K_INC_H_
#define K_INC_H_
#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "common.h"

#ifdef DEBUG_0
#include "uart_polling.h"
#include "printf.h"
#endif /* DEBUG_0 */

/* process states, note we only assume three states in this example */
typedef enum {NEW = 0, RDY, RUN, BLOCKED_ON_RESOURCE} PROC_STATE_E;

typedef struct mem_blk
{
	struct mem_blk    *next;
	U8 	              block[MEM_BLK_SIZE];
} MEM_BLK;

/**
 * @brief PCB data structure definition.
 * @note  You will need to add your own member variables
 *         in order to finish the project 
 */
typedef struct pcb 
{ 
    struct pcb   *mp_next;  /**> next pcb, not used in this example */  
    U32          *mp_sp;    /**> stack pointer of the process       */
    U32          m_pid;     /**> process id                         */
		U32 				 m_priority;/**> priority                           */
    PROC_STATE_E m_state;   /**> state of the process               */
		MEM_BLK      *m_mem_blk; /**> memory blocks                      */
} PCB;


/*
 *==========================================================================
 *                   GLOBAL VARIABLES DECLARATIONS
 *==========================================================================
 */
/* This symbol is defined in the scatter file (see RVCT Linker User Guide)*/  
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit; 
extern U32          *gp_stack;            /* see k_memory.c for details   */  
extern PCB          **gp_pcbs;            /* array of pcbs */
extern PCB          *gp_current_process;  /* the current RUN process      */
extern PROC_INIT    g_proc_table[NUM_TEST_PROCS];
                                          /* process initialization table */

#endif /* ! K_INC_H_ */
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
