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
char * testName = "G_10_test_10";
int nextProcess;

int baseChar = (int) '0';

/**************************************************************************//**
 * @brief: a medium priority process that recieves and prints two messages
 *****************************************************************************/
void proc1(void)
{
    printUart1(testName, "START");

    MSG_BUF* msg = receive_message(NULL);
    uart0_printMsgText(msg);

    successfulTests += assertTest(testName, (int)'2', (int) msg->mtext[0], "2");

    msg = receive_message(NULL);
    uart0_printMsgText(msg);

    successfulTests += assertTest(testName, (int)'3', (int) msg->mtext[0], "3");
    
    set_process_priority(PID_P1, LOWEST);
}

/**************************************************************************//**
 * @brief: a high priority process that recieves a single message
 *****************************************************************************/
void proc2(void)
{
    MSG_BUF* msg = receive_message(NULL);
    uart0_printMsgText(msg);

    successfulTests += assertTest(testName, (int)'1', (int) msg->mtext[0], "1");
    
    set_process_priority(PID_P2, LOWEST);
}
/**************************************************************************//**
 * @brief: a low priority process that recieves a single message, and sets proc5
 *         and proc6 to run afterwards
 *****************************************************************************/
void proc3(void)
{
    MSG_BUF* msg = receive_message(NULL);
    uart0_printMsgText(msg);
    
    successfulTests += assertTest(testName, (int)'4', (int) msg->mtext[0], "4");

    set_process_priority(PID_P5, LOW);
    set_process_priority(PID_P6, LOW);
    set_process_priority(PID_P1, LOWEST);
    set_process_priority(PID_P2, LOWEST);
    set_process_priority(PID_P4, LOWEST);
    set_process_priority(PID_P3, LOWEST);
}

/**************************************************************************//**
 * @brief: A low priority process that recieves 3 messages set to arrive at the 
 *         same time
 *****************************************************************************/
void proc4(void)
{
    MSG_BUF *msg;

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "4: This message arrives last >:( \n\r");
    delayed_send(PID_P3, msg, 100);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "1: This message arrives first :) \n\r");
    delayed_send(PID_P2, msg, 100);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "2: This message arrives second :| \n\r");
    delayed_send(PID_P1, msg, 100);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "3: This message arrives third :( \n\r");
    delayed_send(PID_P1, msg, 100);

    while(1) {
        release_processor();
    }
}

/**************************************************************************//**
 * @brief: A low priority process that recieves two messages and prints contents
 *****************************************************************************/
void proc5(void)
{ 
    MSG_BUF* msg = receive_message(NULL);
    uart0_printMsgText(msg);

    successfulTests += assertTest(testName, (int)'5', (int) msg->mtext[0], "5");

    msg = receive_message(NULL);
    uart0_printMsgText(msg);

    successfulTests += assertTest(testName, (int)'6', (int) msg->mtext[0], "6");

    printSummary(testName, successfulTests, NUM_TESTS);

    while(1) {}
}

/**************************************************************************//**
 * @brief: A low priority process that sends two zero delay messages to pro5
 *****************************************************************************/
void proc6(void)
{
    MSG_BUF* msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "5: Instant Message! \n\r");
    delayed_send(PID_P5, msg, 0);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "6: Snail Mail... \n\r");
    delayed_send(PID_P5, msg, 0);

    while(1) {
        release_processor();
    }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
