/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO SE 350 RTOS LAB
 *
 *          This software is subject to an open source license and 
 *          may be freely redistributed under the terms of MIT License.
 ****************************************************************************
 */

/**************************************************************************//**
 * @file        ae_proc6.c
 * @brief       The goal of this test is to ensure removing processes from the 
 * 				blocked memory queue maintains the integrity of the queue, 
 * 				allowing multiple processes to be unblocked.
 * 
 *              Proc 1 requests both of the free memory blocks and then releases 
 *              execution to proc 2. Proc 2 and Proc 3 will subsequently request 
 *              memory blocks and become immediately blocked, returning 
 *              execution to Proc 1. 
 * 
 *              Proc 1 will release its memory block, unblocking Proc 3 and then
 * 				releasing execution to Proc 3. Proc 3 will then immediately
 * 				release the memory block it gained, unblocking Proc 2 and
 * 				releasing execution to Proc 2, which then ends the test.
 *              
 * @version     V1.2021.01
 * @authors     Group 10
 * @date        2021 JAN
 * @note        Each process is in an infinite loop. Processes never terminate.
 * @details
 *
 *****************************************************************************/
/*---------------------------------------------------------------------------- 
 * Expected COM1 Output 
 * Assume we only have TWO memory blocks in the system
 * Expected UART output: (assuming memory block has ownership.):
 * ABCDE
 * 01234
 * P3P3P3P3P3
 * FGHIJ
 * returned to proc 3
 * 56789
 * proc2: end of testing
 *-------------------------------------------------------------------------------*/ 

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/**************************************************************************//**
 * @brief: a medium priority process that requests 2 memory blocks and then 
 *         releases the processor. After the processor is returned, it changes
 *         the priority of proc3 and proc2 to HIGH in that order and releases
 *         both a memory block and the processor.
 *****************************************************************************/
void proc1(void)
{
    int i = 0;
    int ret_val = 20;
    void *p_mem_blk;
    p_mem_blk = request_memory_block();

    while ( 1 ) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
            if (i == 5) {
                p_mem_blk = request_memory_block();
#ifdef DEBUG_0
                printf("proc1: received mem block: %x\n", p_mem_blk);
#endif /* DEBUG_0 */
                release_processor();
            }
            if (i == 10) {
                set_process_priority(PID_P3, HIGH);
                set_process_priority(PID_P2, HIGH);
                ret_val = release_memory_block(p_mem_blk);

                if (ret_val == 0) {
                    release_processor();
                }
            }
        }
        uart1_put_char('A' + i%26);
        i++;
    }
}
/**************************************************************************//**
 * @brief: A medium priority process that continuously requests memory blocks
 *****************************************************************************/
void proc2(void)
{
    int i = 0;
    void *p_mem_blk;
    
    while ( 1) {
        if (i == 10) {
            uart1_put_string("\n\r");
            break;
        }
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
            p_mem_blk = request_memory_block();
#ifdef DEBUG_0
            printf("proc2: received %x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart1_put_char('0' + i%10);
        i++;
    }
    uart1_put_string("proc2: end of testing\n\r");
    while ( 1 ) {
    }
}
/**************************************************************************//**
 * @brief: A medium priority process that allocates memory and frees it right away
 *****************************************************************************/
void proc3(void)
{
    int i = 0;
    int ret_val = 20;
    void *p_mem_blk;
    
    while ( 1) {
        if (i == 10) {
            uart1_put_string("\n\r");
            break;
        }
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
      p_mem_blk = request_memory_block();
            uart1_put_string("returned to proc 3\n\r");
            release_memory_block(p_mem_blk);
            release_processor();
#ifdef DEBUG_0
            printf("proc3: received %x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart1_put_string("P3");
        i++;
    }
    uart1_put_string("proc3: end of testing\n\r");
    while ( 1 ) {
    }
}
/**************************************************************************//**
 * @brief  All the following processes simply release the processor
 *****************************************************************************/
void proc4(void)
{
    while(1) {
        uart1_put_string("proc4: \n\r");
        release_processor();
    }
}

void proc5(void)
{
    while(1) {
        uart1_put_string("proc5: \n\r");
        release_processor();
    }
}

void proc6(void)
{
    while(1) {
        uart1_put_string("proc6: \n\r");
        release_processor();
    }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
