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
 * @brief       The goal of this test is to ensure that processes which are
 *              blocked on resource (memory) are still capable of recieving 
 *              messages. In this test, proc1 steals all avaiable memory, and 
 *              allows proc2 to become blocked. When proc1 regains execution, it
 *              send a message to proc2, then releases memory to allow proc2 to
 *              become unblocked. Proc2 will then attempts to recieve the message
 *              it would have gotten while blocked.
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

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

#define NUM_TESTS 7

int successfulTests = 0;
char * testName = "G_10_test_7";
int nextProcess;

/* initialization table item */
void set_test_procs(PROC_INIT *procs, int num)
{
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
    procs[2].m_priority   = LOWEST;
    
    procs[3].mpf_start_pc = &proc4;
    procs[3].m_priority   = LOWEST;
    
    procs[4].mpf_start_pc = &proc5;
    procs[4].m_priority   = LOWEST;
    
    procs[5].mpf_start_pc = &proc6;
    procs[5].m_priority   = LOWEST;
}

/**************************************************************************//**
 * @brief: a medium priority process that requests two memory blocks, then after
 *         releasing the processor to proc2, sets proc2's priority to HIGH,
 *         sends a message to proc2, then releases a memory block to unblock proc2
 *****************************************************************************/
void proc1(void)
{
    printUart1(testName, "START");

    nextProcess = PID_P2;
    MSG_BUF *msg = (MSG_BUF*) request_memory_block();
    void *temp = request_memory_block();

    msg->mtype = DEFAULT;
    strcpy(msg->mtext, "YOU WERE BLOCKED :( \n\r");

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
    uart0_printMsgText(msg);

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
