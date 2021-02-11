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
 * @file        ae_proc5.c
 * @brief       A test to verify that the null process has been initialized
 *              properly. Proc1 will request memory until it becomes blocked,
 *              and each subsequent proc will attempt to request memory (blocking
 *              them as well), until the null process is reached.
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
 * Expected UART output (assuming memory block has ownership.):
 * ABCDE
 * (proc1: received mem block: ...)
 * FGHIJ
 * (proc1: received mem block: ...)
 * KLMNO
 * proc2:
 * proc3:
 * proc5:
 * proc4:
 * proc6:
 * (should reach null proc)
 *-------------------------------------------------------------------------------*/ 

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"
#include "printf.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */


/**************************************************************************//**
 * @brief: Continuously print five letters and then a request a memory block
 *****************************************************************************/
void proc1(void)
{
    int i = 0;
    int ret_val = 20;
    void *p_mem_blk;

    while ( 1 ) {
        if ( i != 0 && i % 5 == 0 ) {
            uart1_put_string("\n\r");
            p_mem_blk = request_memory_block();
#ifdef DEBUG_0
            printf("proc1: received mem block: %x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart1_put_char('A' + i%26);
        i++;
    }
    while( 1 ) {
    } 
}
/**************************************************************************//**
 * @brief  All the following processes simply attempt to request a memory block
 *****************************************************************************/
void proc2(void)
{
    while(1) {
        uart1_put_string("proc2: \n\r");
        request_memory_block();
        release_processor();
    }
}

void proc3(void)
{
    while(1) {
        uart1_put_string("proc3: \n\r");
        request_memory_block();
        release_processor();
    }
}

void proc4(void)
{
    while(1) {
        uart1_put_string("proc4: \n\r");
        request_memory_block();
        release_processor();
    }
}

void proc5(void)
{
    while(1) {
        uart1_put_string("proc5: \n\r");
        request_memory_block();
        release_processor();
    }
}

void proc6(void)
{
    while(1) {
        uart1_put_string("proc6: \n\r");
        request_memory_block();
        release_processor();
    }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
