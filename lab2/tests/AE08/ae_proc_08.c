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
 * Expected UART2 Output:
 * G_10_test_8: START
 * G_10_test_8: test 1 OK
 * G_10_test_8: test 2 OK
 * G_10_test_8: test 3 OK
 * G_10_test_8: test 4 OK
 * G_10_test_8: test 5 OK
 * G_10_test_8: test 6 OK
 * G_10_test_8: test 7 OK
 * G_10_test_8: test 8 OK
 * G_10_test_8: test 9 OK
 * G_10_test_8: test 10 OK
 * G_10_test_8: test 11 OK
 * G_10_test_8: test 12 OK
 * G_10_test_8: 12/12 tests OK
 * G_10_test_8: 0/12 tests FAIL
 * G_10_test_8: END
 *
 * Expected UART1 Output:
 * 01234
 * 56789
 * 0Short message to proc 2
 * 1234
 * 56789
 * 012New short message to proc 2
 * ABLong message to proc 1
 * 
 *-------------------------------------------------------------------------------*/ 

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"
#include "ae_util.h"

// #ifdef DEBUG_0
#include "printf.h"
// #endif /* DEBUG_0 */

#define NUM_TESTS 12

int successfulTests = 0;
char * testName = "G_10_test_8";
int nextProcess;

/**************************************************************************//**
 * @brief: a high priority process that recieves a message and prints it.
 *****************************************************************************/
void proc1(void)
{
    printUart1(testName, "START");

    nextProcess = PID_P2;

    // Receive message
    int sender;
    MSG_BUF* msg = receive_message(&sender);

    successfulTests += assertTest(testName, nextProcess, PID_P1, "10");

    uart0_printMsgText(msg);

    // Assert that we got the right message
    successfulTests += assertTest(testName, (int)'L', (int) msg->mtext[0], "11");
    successfulTests += assertTest(testName, PID_P3, sender, "12");
    
    printSummary(testName, successfulTests, NUM_TESTS);
}
/**************************************************************************//**
 * @brief: a medium priority process that receives and prints a message, then
 *         sends another delayed message to itself, then receives and prints it.
 *****************************************************************************/
void proc2(void)
{
    successfulTests += assertTest(testName, nextProcess, PID_P2, "1");
    // Recieve message
    int sender;
    nextProcess = PID_P3;
    MSG_BUF* msg = receive_message(&sender);
    
    uart0_printMsgText(msg);

    successfulTests += assertTest(testName, nextProcess, PID_P2, "4");
    // Assert that we got the right message
    successfulTests += assertTest(testName, (int)'S', (int) msg->mtext[0], "5");
    successfulTests += assertTest(testName, sender, PID_P3, "6");

    strcpy(msg->mtext, "New short message to proc 2\n\r");
    delayed_send(PID_P2, msg, 30);

    nextProcess = PID_P2;

    // Receive message
    msg = receive_message(&sender);

    successfulTests += assertTest(testName, nextProcess, PID_P2, "7");

    uart0_printMsgText(msg);

    // Assert that we got the right message
    successfulTests += assertTest(testName, (int)'N', (int) msg->mtext[0], "8");
    successfulTests += assertTest(testName, PID_P2, (int) msg->m_send_pid, "9");

    nextProcess = PID_P1;

    int i = 0;
    while(1) {
        if (i != 0 && i % 50000 == 0) {
			uart0_put_string("\n\r");
        }
        if (i % 10000 == 0) {
            uart0_put_char('A' + (i/10000) % 26);
        }
        i++;
    }
}
/**************************************************************************//**
 * @brief: a low priority process that sends a message to proc1 with a long delay
 *         and sends a message to proc2 with a short delay.
 *****************************************************************************/
void proc3(void)
{
    successfulTests += assertTest(testName, nextProcess, PID_P3, "2");
    MSG_BUF *msg1 = (MSG_BUF*) request_memory_block();
    MSG_BUF *msg2 = (MSG_BUF*) request_memory_block();
    // MSG_BUF *msg3 = (MSG_BUF*) request_memory_block();

    msg1->mtype = DEFAULT;
    strcpy(msg1->mtext, "Long message to proc 1\n\r");
    msg2->mtype = DEFAULT;
    strcpy(msg2->mtext, "Short message to proc 2\n\r");

    delayed_send(PID_P1, msg1, 80);
    delayed_send(PID_P2, msg2, 30);
    successfulTests += assertTest(testName, nextProcess, PID_P3, "3");
    nextProcess = PID_P2;

    int i = 0;
    while(1) {
        if (i != 0 && i % 50000 == 0) {
			uart0_put_string("\n\r");
        }
        if (i % 10000 == 0) {
            uart0_put_char('0' + (i/10000) % 10);
        }
        i++;
    }
}

/**************************************************************************//**
 * @brief: a lowest priority process that does nothing
 *****************************************************************************/
void proc4(void)
{
    while (1) { }
}

/**************************************************************************//**
 * @brief: a lowest priority process that does nothing
 *****************************************************************************/
void proc5(void)
{
    while (1) { }
}

/**************************************************************************//**
 * @brief: a lowest priority process that does nothing
 *****************************************************************************/
void proc6(void)
{
    while (1) { }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
