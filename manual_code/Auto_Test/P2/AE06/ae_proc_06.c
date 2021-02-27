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
 * @file        ae_proc06.c
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
 * G_10_test_6: test 1 OK
 * G_10_test_6: test 2 OK
 * G_10_test_6: test 3 OK
 * G_10_test_6: test 4 OK
 * G_10_test_6: test 5 OK
 * G_10_test_6: 5/5 tests OK
 * G_10_test_6: 0/5 tests FAIL
 * G_10_test_6: END
 *
 * Expected UART2 Output:
 * HELLO WORLD!
 * 
 *-------------------------------------------------------------------------------*/ 

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"
#include "ae_util.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

#define NUM_TESTS 5

int successfulTests = 0;
char * testName = "G_10_test_6";
int nextProcess;

/**************************************************************************//**
 * @brief: a high priority process
 *****************************************************************************/
void proc1(void)
{
#ifdef DEBUG_0
    printf("Start of proc1\n");
#endif /* DEBUG_0 */
    
    nextProcess = PID_P2;
    // This will block proc1 since no message has been sent yet
    int expected_process;
    MSG_BUF* message = receive_message(&expected_process);
    
    int i = 0;
    while(message->mtext[i] != '\0' && i < 50) {
        uart1_put_char(message->mtext[i]);
        i++;
    }
    
    successfulTests += assertTest(testName, PID_P1, nextProcess, "2");
    successfulTests += assertTest(testName, PID_P2, expected_process, "3");
    successfulTests += assertTest(testName, DEFAULT, message->mtype, "4");
    successfulTests += assertTest(testName, (int)'H', (int) message->mtext[0], "5");
    
    printSummary(testName, successfulTests, NUM_TESTS);
}
/**************************************************************************//**
 * @brief: a medium priority process
 *****************************************************************************/
void proc2(void)
{
#ifdef DEBUG_0
    printf("Start of proc2\n");
#endif /* DEBUG_0 */
    successfulTests += assertTest(testName, 2, nextProcess, "1");

    MSG_BUF *msg = (MSG_BUF*) request_memory_block();

    msg->mtype = DEFAULT;
    msg->mtext[0] = 'H';
    msg->mtext[1] = 'E';
    msg->mtext[2] = 'L';
    msg->mtext[3] = 'L';
    msg->mtext[4] = 'O';
    msg->mtext[5] = ' ';
    msg->mtext[6] = 'W';
    msg->mtext[7] = 'O';
    msg->mtext[8] = 'R';
    msg->mtext[9] = 'L';
    msg->mtext[10] = 'D';
    msg->mtext[11] = '!';
    msg->mtext[12] = '\0';

    nextProcess = PID_P1;
    // send message to proc1 which will unblock and preempt proc2
    send_message(PID_P1, msg);

#ifdef DEBUG_0
    printf("Shouldn't reach here!");
#endif /* DEBUG_0 */
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
