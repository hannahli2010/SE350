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
 * @file        ae_proc08.c
 * @brief       The goal of this test set is to ensure that messages sent at the
 *              same time arrive at the destination processes at the same time,
 *              and the highest priority process runs first.
 * 
 *              Additionally, we send two messages to P1 set to arrive at the
 *              same time. The first message sent should be the first message
 *              to arrive.
 *      
 *              Finally, we have proc5 send two messages to proc6 with zero 
 *              delay to ensure that instant messages are function properly. 
 * 
 * @version     V1.2021.01
 * @authors     Group 10
 * @date        2021 JAN
 * @note        Each process is in an infinite loop. Processes never terminate.
 * @details
 *
 *****************************************************************************/
/*---------------------------------------------------------------------------- 
 * Expected UART2 Output:
 * G_10_test_10: START
 * G_10_test_10: test 1 OK
 * G_10_test_10: test 2 OK
 * G_10_test_10: test 3 OK
 * G_10_test_10: test 4 OK
 * G_10_test_10: test 5 OK
 * G_10_test_10: test 6 OK
 * G_10_test_10: 6/6 tests OK
 * G_10_test_10: 0/6 tests FAIL
 * G_10_test_10: END
 *
 * Expected UART1 Output:
 * 1: This message arrives first :) 
 * 2: This message arrives second :| 
 * 3: This message arrives third :( 
 * 4: This message arrives last >:( 
 * 5: Instant Message! 
 * 6: Snail Mail... 
 *
 *-------------------------------------------------------------------------------*/ 

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"
#include "ae_util.h"

// #ifdef DEBUG_0
#include "printf.h"
// #endif /* DEBUG_0 */

#define NUM_TESTS 6

int successfulTests = 0;
char * testName = "G_10_test_13";
int nextProcess;

int baseChar = (int) '0';

void set_test_procs(PROC_INIT *procs, int num) {
    int i;
    for( i = 0; i < num; i++ ) {
        procs[i].m_pid        = (U32)(i+1);
        procs[i].m_stack_size = USR_SZ_STACK;
    }
  
    procs[0].mpf_start_pc = &proc1;
    procs[0].m_priority   = MEDIUM;
    
    procs[1].mpf_start_pc = &proc2;
    procs[1].m_priority   = MEDIUM;
    
    procs[2].mpf_start_pc = &proc3;
    procs[2].m_priority   = LOW;
    
    procs[3].mpf_start_pc = &proc4;
    procs[3].m_priority   = LOW;
    
    procs[4].mpf_start_pc = &proc5;
    procs[4].m_priority   = LOW;
    
    procs[5].mpf_start_pc = &proc6;
    procs[5].m_priority   = LOW;
}

/**************************************************************************//**
 * @brief: a medium priority process that recieves and prints two messages
 *****************************************************************************/
void proc1(void)
{
    int i = 0;
    while(1) {
        uart1_put_string("Process 1\r\n");
        for(i = 0; i < 10000; i++);
        release_processor();
    }
}

/**************************************************************************//**
 * @brief: a high priority process that recieves a single message
 *****************************************************************************/
void proc2(void)
{
    int i = 0;
    while(1) {
        uart1_put_string("Process 2\r\n");
        for(i = 0; i < 10000; i++);
        release_processor();
    }
}
/**************************************************************************//**
 * @brief: a low priority process that recieves a single message, and sets proc5
 *         and proc6 to run afterwards
 *****************************************************************************/
void proc3(void)
{
    int i = 0;
    while(1) {
        uart1_put_string("Process 3\r\n");
        for(i = 0; i < 10000; i++);
        release_processor();
    }
}

/**************************************************************************//**
 * @brief: A low priority process that recieves 3 messages set to arrive at the 
 *         same time
 *****************************************************************************/
void proc4(void)
{
    int i = 0;
    while(1) {
        uart1_put_string("Process 4\r\n");
        for(i = 0; i < 10000; i++);
        release_processor();
    }
}

/**************************************************************************//**
 * @brief: A low priority process that recieves two messages and prints contents
 *****************************************************************************/
void proc5(void)
{ 
    int i = 0;
    while(1) {
        uart1_put_string("Process 5\r\n");
        for(i = 0; i < 10000; i++);
        release_processor();
    }
}

/**************************************************************************//**
 * @brief: A low priority process that sends two zero delay messages to pro5
 *****************************************************************************/
void proc6(void)
{
    int i = 0;
    while(1) {
        uart1_put_string("Process 6\r\n");
        for(i = 0; i < 10000; i++);
        release_processor();
    }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
