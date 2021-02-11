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
 * @file        ae_proc4.c
 * @brief       The goal of this test is to ensure priority in the blocked queue 
 *              is maintained after set_priority calls in the blocked queue.
 *              Proc 1 requests both of the free memory blocks and then releases 
 *              execution to proc 2. Proc 2 and proc 3 will subsequently request 
 *              memory blocks and become immediately blocked, returning 
 *              execution to proc 1. 
 * 
 *              Proc 1 will then set the priority of Proc 2 to be lower than
 *              Proc 3. This should cause Proc 2 and Proc 3 to swap positions in
 *              the blocked memory queue, such that when Proc 1 releases its 
 *              second memory block and the processor, then the Proc 3 will gain
 *              execution and end the test.
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
 * Assume we only have TWO memory blocks in the system.
 * Expected UART output: (assuming memory block has ownership.):
 * ABCDE
 * 01234
 * P3P3P3P3P3
 * FGHIJ
 * P3P3P3P3P3
 * proc3: end of testing
 *-------------------------------------------------------------------------------*/ 

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */


/**************************************************************************//**
 * @brief: a medium priority process that requests 2 memory blocks, then
 *         releases the processor. When it is re-entered, it sets the priority
 *         of Proc 2 to "LOWEST", 
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
                set_process_priority(PID_P2, LOWEST);
                ret_val = release_memory_block(p_mem_blk);

                if (ret_val == 0) {
                    release_processor();
                    break;
                }
            }
        }
        uart1_put_char('A' + i%26);
        i++;
    }
    while( 1 ) {

    } 
}
/**************************************************************************//**
 * @brief: A process that prints 5 consecutive integers, and then requests a 
 *         memory block without freeing.
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
 * @brief: A process that prints P3 five times, and then requests a memory
 *         block without freeing.
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
