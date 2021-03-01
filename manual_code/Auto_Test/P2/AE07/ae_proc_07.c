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
 * @file        ae_proc07.c
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
 * Expected UART1 Output:
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
 * Expected UART2 Output:
 * YOU WERE BLOCKED :( 
 * 
 *-------------------------------------------------------------------------------*/ 

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"
#include "ae_util.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

#define NUM_TESTS 7

int successfulTests = 0;
char * testName = "G_10_test_7";
int nextProcess;

/**************************************************************************//**
 * @brief: a medium priority process that requests two memory blocks, then after
 *         releasing the processor to proc2, sets proc2's priority to HIGH,
 *         sends a message to proc2, then releases a memory block to unblock proc2
 *****************************************************************************/
void proc1(void)
{
    nextProcess = PID_P2;
    MSG_BUF *msg = (MSG_BUF*) request_memory_block();
    void *temp = request_memory_block();

    msg->mtype = DEFAULT;
    strcpy(msg->mtext, "YOU WERE BLOCKED :( \n");

    // Release the processor to P2
    nextProcess = PID_P2;
    release_processor();

    // Once P2 blocks, we should return back to P1
    successfulTests += assertTest(testName, nextProcess, PID_P1, "2");

    // Set the priority of P2 to high, such that when it unblocks we immediately prempt
    set_process_priority(PID_P2, HIGH);

    // Send message to P2, which should NOT unblock P2
    int send_result = send_message(PID_P2, msg);
    successfulTests += assertTest(testName, send_result, RTX_OK, "3");
    successfulTests += assertTest(testName, nextProcess, PID_P1, "4");

    // Releasing the other memory block should unblock P2 and allow it to pre-empt us
    nextProcess = PID_P2;
    release_memory_block(temp);
}
/**************************************************************************//**
 * @brief: a medium priority process that requests a memory block (which should
 *         cause it to block on memory), then tries to receive a message.
 *****************************************************************************/
void proc2(void)
{
    successfulTests += assertTest(testName, nextProcess, PID_P2, "1");
    
    nextProcess = PID_P1;
    request_memory_block(); // should block on memory

    // Once unblocked on memory, we should have a message waiting for us
    successfulTests += assertTest(testName, nextProcess, PID_P2, "5");

    // Recieve message
    int sender;
    MSG_BUF* msg = receive_message(&sender);
    successfulTests += assertTest(testName, sender, PID_P1, "6");
    // Print message contents
    int i = 0;
    while(msg->mtext[i] != '\0' && i < 50) {
        uart1_put_char(msg->mtext[i]);
        i++;
    }
    // Assert that we got the right message
    successfulTests += assertTest(testName, (int)'Y', (int) msg->mtext[0], "7");

    printSummary(testName, successfulTests, NUM_TESTS);

    while(1) {}
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
