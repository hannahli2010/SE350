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

/*
 *===========================================================================
 *                             MACROS
 *===========================================================================
 */

/* ATTENTION: These OFFSET macros depends on the PCB struct 
 *            member field location in the struct.
 *            You need to update them accordingly 
 *             if you make changes of PCB structure.
 */
#define PCB_MSP_OFFSET      4       /* mp_sp is 4B offset from pcb struct starting addr.    */
#define PCB_STATE_OFFSET    12      /* m_state is 12B offset from pcb struct starting addr. */
#define STACK_SIZE_IPROC    0x200   /* iprocess stack size */
#define NUM_SYS_PROC        10       /* total number of system procs */


/* process states, note we only assume three states in this example */
typedef enum {NEW = 0, RDY, RUN, BLOCKED_ON_RESOURCE, BLOCKED_ON_MESSAGE, IPROC} PROC_STATE_E;  

typedef struct mem_blk
{
	struct mem_blk    *mp_next;
	U8 	              block[MEM_BLK_SIZE];
} MEM_BLK;

/**
 * @brief PCB data structure definition.
 * @note  You will need to add your own member variables
 *         in order to finish the project 
 */
typedef struct pcb 
{ 
    struct pcb   *mp_next;  /**> next pcb, not used in this example      */  
    U32          *mp_sp;    /**> stack pointer of the process, 4B offset */
    U32          m_pid;     /**> process id, 8B offset                   */
    PROC_STATE_E m_state;   /**> state of the process, 12B offset        */
    U32          m_priority; /**> priority                               */
    MEM_BLK      *m_mem_blk; /**> memory blocks                          */
    MSG_BUF      *m_msg_buf; /**> mailbox queue                          */
} PCB;


/* cast-compatible with MSG_BUF */
typedef struct delayed_msg_buf
{
#ifdef K_MSG_ENV
    struct delayed_msg_buf *mp_next;              /**> ptr to next message received*/
    int m_send_pid;             /**> sender pid                  */
    int m_timer_pid;            /**> receiver pid                */
    int m_real_recv_pid;        /**> receiver pid                */
    int m_expiry;
    int m_kdata[3];
#endif
    int mtype;                  /**> user defined message type   */
    char mtext[1];              /**> body of the message         */
} DELAYED_MSG_BUF;

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
extern PCB          *gp_pcb_timer_iproc;  /* points to Timer iprocess pcb */
extern PCB          *gp_pcb_uart_iproc;   /* points to Timer iprocess pcb */ 
extern PCB          *gp_pcb_interrupted;  /* interrupted process's pcb    */
extern PROC_INIT    g_proc_table[];       /* process initialization table */

extern PCB          *proc_ready_queue;    /* ready processes              */
extern PCB          *proc_blocked_queue;  /* processes blocked on memory  */
extern U32          num_free_mem_blks;    /* number of free memory blocks */

extern volatile uint32_t g_timer_count;   /* increment every 1 ms         */
extern DELAYED_MSG_BUF *delayed_msg_queue;

#endif /* ! K_INC_H_ */

/*
 *===========================================================================
 *                          FUNCTIONAL MACROS
 *===========================================================================
 */
#define ENTER_KERNEL_FUNC() ;
    // if (gp_current_process->m_pid != PID_TIMER_IPROC) { __disable_irq(); } \
    // uint32_t ctrl = __get_CONTROL(); \
    // __set_CONTROL(0);

#define EXIT_KERNEL_FUNC() ;
    // __set_CONTROL(ctrl); \
    // if (gp_current_process->m_pid != PID_TIMER_IPROC) { __enable_irq(); }

/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
