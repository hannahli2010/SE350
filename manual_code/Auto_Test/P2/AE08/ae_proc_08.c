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
 * @brief       Tests after the initial submission
 *              
 * @version     V1.2021.01
 * @authors     Group 10
 * @date        2021 JAN
 * @note        Each process is in an infinite loop. Processes never terminate.
 * @details
 *
 *****************************************************************************/
/*---------------------------------------------------------------------------- 
 * Assume there are 2 memory blocks
 * 
 * Expected UART2 Output:
 *
 * G_10_test_7: test 1 OK
 * G_10_test_7: test 2 OK
 * G_10_test_7: test 3 OK
 * G_10_test_7: test 4 OK
 * G_10_test_7: test 5 OK
 * G_10_test_7: test 6 OK
 * G_10_test_7: test 7 OK
 * G_10_test_7: 7/7 tests OK
 * G_10_test_7: 0/7 tests FAIL
 * G_10_test_7: END
 *
 * Expected UART1 Output:
 * YOU WERE BLOCKED :( 
 * 
 *-------------------------------------------------------------------------------*/ 

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"
#include "ae_util.h"

// #ifdef DEBUG_0
#include "printf.h"
// #endif /* DEBUG_0 */

#define NUM_TESTS 7

int successfulTests = 0;
char * testName = "G_10_test_8";
int nextProcess;

/**************************************************************************//**
 * @brief: a medium priority process that requests two memory blocks, then after
 *         releasing the processor to proc2, sets proc2's priority to HIGH,
 *         sends a message to proc2, then releases a memory block to unblock proc2
 *****************************************************************************/
void proc1(void)
{
    printUart1(testName, "START");

    // Recieve message
    int sender;
    nextProcess = PID_P2;
    MSG_BUF* msg = receive_message(&sender);

    successfulTests += assertTest(testName, nextProcess, PID_P1, "3");

    int i = 0;
    while(msg->mtext[i] != '\0' && i < 50) {
        uart0_put_char(msg->mtext[i]);
        i++;
    }
    // Assert that we got the right message
    successfulTests += assertTest(testName, (int)'D', (int) msg->mtext[0], "4");

    // TODO: test this!!!
    successfulTests += assertTest(testName, PID_P2, (int) msg->m_send_pid, "5");
    successfulTests += assertTest(testName, PID_TIMER_IPROC, (int) msg->m_send_pid, "6"); // for some reason this one passes ??
    successfulTests += assertTest(testName, PID_P1, (int) msg->m_send_pid, "7");
    
    printSummary(testName, successfulTests, NUM_TESTS);
}
/**************************************************************************//**
 * @brief: a medium priority process that requests a memory block (which should
 *         cause it to block on memory), then tries to receive a message.
 *****************************************************************************/
void proc2(void)
{
    successfulTests += assertTest(testName, nextProcess, PID_P2, "1");
    
    MSG_BUF *msg = (MSG_BUF*) request_memory_block();

    msg->mtype = DEFAULT;
    strcpy(msg->mtext, "Delayed message\n");

    k_delayed_send(PID_P1, msg, 5);
    successfulTests += assertTest(testName, nextProcess, PID_P2, "2");

    nextProcess = PID_P1;

    int i = 0;
    while(1) {
        if (i != 0 && i % 5 == 0) {
			uart0_put_string("\n\r");
        }
        uart0_put_char('0' + i%10);
        i++;
    }
}
/**************************************************************************//**
 * @brief: a lowest priority process that releases the processor
 *****************************************************************************/
void proc3(void)
{
    release_processor();
}

/**************************************************************************//**
 * @brief: a lowest priority process that releases the processor
 *****************************************************************************/
void proc4(void)
{
    release_processor();
}

/**************************************************************************//**
 * @brief: a lowest priority process that releases the processor
 *****************************************************************************/
void proc5(void)
{
    release_processor();
}

/**************************************************************************//**
 * @brief: a lowest priority process that releases the processor
 *****************************************************************************/
void proc6(void)
{
    release_processor();
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
