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
 * @file        k_msg.c
 * @brief       kernel message passing source code template
 *              
 * @version     V1.2021.01
 * @authors     Yiqing Huang
 * @date        2021 JAN
 *****************************************************************************/
#include "k_msg.h"
#include "k_inc.h"
#include "k_queue.h"
#include "k_rtx.h"
/*
 *==========================================================================
 *                            TO BE IMPLEMENTED
 *==========================================================================
 */
 
DELAYED_MSG_BUF* delayed_msg_queue = NULL;

int k_send_message_actual(int pid, void *p_msg) {
    ENTER_KERNEL_FUNC();
    // Ensure destination process actually exists
    // recv_pcb← convert recv_pidto process obj/PCB reference
    PCB* destProc = get_pcb_by_pid(pid);
    if (destProc == NULL) {
        // Destination process doesn't exist, rip
        EXIT_KERNEL_FUNC();
        return RTX_ERR;
    }

    // set sender_pid, destination_pid fields in envelope
    MSG_BUF* message = (MSG_BUF*) p_msg;
    if (gp_current_process->m_pid != PID_TIMER_IPROC)
        message->m_send_pid = gp_current_process->m_pid;
    message->m_recv_pid = pid;

    // Add message to the back of the destination process' mailbox
    q_insert((MEM_BLK**) &(destProc->m_msg_buf), (MEM_BLK*) message);
 
    // Transfer ownership of the memory block from the target process to the destination process
    // Remove memory block from the sending process
    MEM_BLK* target_blk = q_remove_by_addr(&(gp_current_process->m_mem_blk), p_msg);
    if (target_blk == NULL) {
        EXIT_KERNEL_FUNC();
        return RTX_ERR;
    }

    // If the destination process is blocked on message, set it to ready and then check for preemption
    if (destProc->m_state == BLOCKED_ON_MESSAGE) {
        destProc->m_state = RDY;
        
        // If the destination process has a higher priority than the current process, preempt the current process
        if(destProc->m_priority < gp_current_process->m_priority && gp_current_process->m_pid != PID_TIMER_IPROC){
            PCB * p_pcb_old = gp_current_process;
            gp_current_process = destProc;
            pq_insert_front_ready(p_pcb_old); //insert p_pcb_old to front of that prio in the queue
            process_switch(p_pcb_old);
        } 
        // Otherwise, just insert the destination process into the ready queue
        else {
            pq_insert_ready(destProc); 
        }
    }

    EXIT_KERNEL_FUNC();
    return RTX_OK;
}

int k_send_message(int pid, void *p_msg) {
    if (pid == PID_TIMER_IPROC) return RTX_ERR;
    return k_send_message_actual(pid, p_msg);
}

// To be done after iprocess implementation
int k_delayed_send(int pid, void *p_msg, int delay) {
    ENTER_KERNEL_FUNC();
    if (pid == PID_TIMER_IPROC) return RTX_ERR;
    DELAYED_MSG_BUF* message = (DELAYED_MSG_BUF*) p_msg;
    message->m_expiry = delay;
    message->m_real_recv_pid = pid;
    int status = k_send_message_actual(PID_TIMER_IPROC, p_msg);
    EXIT_KERNEL_FUNC();
    return status;
}

void *k_receive_message_nb(int *p_pid) {
    // Return NULL if the mailbox is empty
    if (gp_current_process->m_msg_buf == NULL) {
        return NULL;
    }

    MSG_BUF* message = (MSG_BUF*) q_remove((MEM_BLK **) &(gp_current_process->m_msg_buf));
    // Give envelope memory block to the target process
    q_insert(&(gp_current_process->m_mem_blk), (MEM_BLK*) ((int *) message - 1));
    
    // If the user requested the function to set the sender id, place the sender id into the provided address
    if (p_pid != NULL) {
        *p_pid = message->m_send_pid;
    }

    return message;
}

void *k_receive_message(int *p_pid) {
    // Infinitely loop / release processor if the mailbox is empty 
    while (gp_current_process->m_msg_buf == NULL) {
        gp_current_process->m_state = BLOCKED_ON_MESSAGE; //set process state
    
        // release processor but don't add ourselves to ready queue
        PCB *p_pcb_old = gp_current_process;
        gp_current_process = scheduler();
        process_switch(p_pcb_old);
    }

    MSG_BUF* message = (MSG_BUF*) q_remove((MEM_BLK **) &(gp_current_process->m_msg_buf));
    // Give envelope memory block to the target process
    q_insert(&(gp_current_process->m_mem_blk), (MEM_BLK*) ((int *) message - 1));
    
    // If the user requested the function to set the sender id, place the sender id into the provided address
    if (p_pid != NULL) {
        *p_pid = message->m_send_pid;
    }

    return message;
}
 
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
