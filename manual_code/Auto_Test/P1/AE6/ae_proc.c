/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO SE 350 RTOS LAB  
 *
 *                     Copyright 2020-2021 Yiqing Huang
 *
 *          This software is subject to an open source license and 
 *          may be freely redistributed under the terms of MIT License.
 ****************************************************************************
 */

/**************************************************************************//**
 * @file        ae_proc6.c
 * @brief       Two auto test processes to test memory preeption and ownership
 *              
 * @version     V1.2021.01
 * @authors     Yiqing Huang
 * @date        2021 JAN
 * @note        Each process is in an infinite loop. Processes never terminate.
 * @details
 *
 *****************************************************************************/
/*---------------------------------------------------------------------------- 
 * Expected COM1 Output 
 * Assume we only have TWO memory blocks in the system and that if you set a proc's
 *    priority to itself, it gets moved to the end of that priority 
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
 * @brief: a process that keeps allocating memory without freeing
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
								set_process_priority(PID_P2, MEDIUM);
                ret_val = release_memory_block(p_mem_blk);

                if (ret_val == 0) {
                    release_processor();
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
 * @brief  a process that tries to free another process's memory
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
			uart1_put_string("returned to proc 3\n");
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
