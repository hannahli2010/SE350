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
 * @brief       Procs 1, 2 and 3 request messages and print the contents 
 *              repeatedly while Proc 4 sends 21 messages in a random order to
 *              the first 3 procedures with delay proportional to the message
 *              number. This test verifies the integrity of the delayed message
 *              queue by ensuring that all 21 messages arrive in the correct 
 *              order
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
 *
 * G_10_test_9: START
 * G_10_test_9: test 1 OK
 * G_10_test_9: test 2 OK
 * G_10_test_9: test 3 OK
 * G_10_test_9: test 4 OK
 * G_10_test_9: test 5 OK
 * G_10_test_9: test 6 OK
 * G_10_test_9: test 7 OK
 * G_10_test_9: test 8 OK
 * G_10_test_9: test 9 OK
 * G_10_test_9: test 10 OK
 * G_10_test_9: test 11 OK
 * G_10_test_9: test 12 OK
 * G_10_test_9: test 13 OK
 * G_10_test_9: test 14 OK
 * G_10_test_9: test 15 OK
 * G_10_test_9: test 16 OK
 * G_10_test_9: test 17 OK
 * G_10_test_9: test 18 OK
 * G_10_test_9: test 19 OK
 * G_10_test_9: test 20 OK
 * G_10_test_9: test 21 OK
 * G_10_test_9: test 22 OK
 * G_10_test_9: test 23 OK
 * G_10_test_9: test 24 OK
 * G_10_test_9: test 25 OK
 * G_10_test_9: test 26 OK
 * G_10_test_9: test 27 OK
 * G_10_test_9: test 28 OK
 * G_10_test_9: test 29 OK
 * G_10_test_9: test 30 OK
 * G_10_test_9: test 31 OK
 * G_10_test_9: test 32 OK
 * G_10_test_9: test 33 OK
 * G_10_test_9: 33/33 tests OK
 * G_10_test_9: 0/33 tests FAIL
 * G_10_test_9: END
 * 
 * Expected UART1 Output:
 * Numbers 1 to 21, each on a new line
 *
 *-------------------------------------------------------------------------------*/ 

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"
#include "ae_util.h"

// #ifdef DEBUG_0
#include "printf.h"
// #endif /* DEBUG_0 */

#define NUM_TESTS 33

int successfulTests = 0;
char * testName = "G_10_test_9";
int nextProcess;

int baseChar = (int) '0';

/**************************************************************************//**
 * @brief: a high priority process that recieves and prints messages continuously
 *****************************************************************************/
void proc1(void)
{
    printUart1(testName, "START");

    MSG_BUF* msg;
    int counter = 0;
    while(1) {
        msg = receive_message(NULL);
        uart0_printMsgText(msg);

        if (counter == 0) {
            successfulTests += assertTest(testName, baseChar + 4, (int) msg->mtext[0], "4"); // 4
        } else if (counter == 1) {
            successfulTests += assertTest(testName, baseChar + 7, (int) msg->mtext[0], "7"); // 7
        } else if (counter == 2) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "14"); // 12
            successfulTests += assertTest(testName, baseChar + 2, (int) msg->mtext[1], "15"); // 

        } else if (counter == 3) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "16"); // 13
            successfulTests += assertTest(testName, baseChar + 3, (int) msg->mtext[1], "17"); // 13

        } else if (counter == 4) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "22"); // 16
            successfulTests += assertTest(testName, baseChar + 6, (int) msg->mtext[1], "23");
        } else if (counter == 5) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "24"); // 17
            successfulTests += assertTest(testName, baseChar + 7, (int) msg->mtext[1], "25"); // 17

        } else if (counter == 6) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "28"); // 19
            successfulTests += assertTest(testName, baseChar + 9, (int) msg->mtext[1], "29"); // 19
        }
        release_memory_block(msg);
        counter++;
    }
}
/**************************************************************************//**
 * @brief: a medium priority process that recieves and prints messages continuously
 *****************************************************************************/
void proc2(void)
{
    MSG_BUF* msg;
    int counter = 0;
    while(1) {
        msg = receive_message(NULL);
        uart0_printMsgText(msg);

        if (counter == 0) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "1"); // 1
        } else if (counter == 1) {
            successfulTests += assertTest(testName, baseChar + 2, (int) msg->mtext[0], "2"); // 2
        } else if (counter == 2) {
            successfulTests += assertTest(testName, baseChar + 5, (int) msg->mtext[0], "5"); // 5
        } else if (counter == 3) {
            successfulTests += assertTest(testName, baseChar + 8, (int) msg->mtext[0], "8"); // 8

        } else if (counter == 4) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "10"); // 10
            successfulTests += assertTest(testName, baseChar + 0, (int) msg->mtext[1], "11");
        } else if (counter == 5) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "12"); // 11
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[1], "13");
        } else if (counter == 6) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "18"); // 14
            successfulTests += assertTest(testName, baseChar + 4, (int) msg->mtext[1], "19"); // 
        } else if (counter == 7) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "26"); // 18
            successfulTests += assertTest(testName, baseChar + 8, (int) msg->mtext[1], "27"); // 
        }
        release_memory_block(msg);
        counter++;
    }
}
/**************************************************************************//**
 * @brief: a low priority process that recieves and prints messages continuously
 *****************************************************************************/
void proc3(void)
{
    MSG_BUF* msg;
    int counter = 0;
    while(1) {
        msg = receive_message(NULL);
        uart0_printMsgText(msg);

        if (counter == 0) {
            successfulTests += assertTest(testName, baseChar + 3, (int) msg->mtext[0], "3"); // 3
        } else if (counter == 1) {
            successfulTests += assertTest(testName, baseChar + 6, (int) msg->mtext[0], "6"); // 6
        } else if (counter == 2) {
            successfulTests += assertTest(testName, baseChar + 9, (int) msg->mtext[0], "9"); // 9

        } else if (counter == 3) {
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[0], "20"); // 15
            successfulTests += assertTest(testName, baseChar + 5, (int) msg->mtext[1], "21"); //

        } else if (counter == 4) {
            successfulTests += assertTest(testName, baseChar + 2, (int) msg->mtext[0], "30"); // 20
            successfulTests += assertTest(testName, baseChar + 0, (int) msg->mtext[1], "31");
        } else if (counter == 5) {
            successfulTests += assertTest(testName, baseChar + 2, (int) msg->mtext[0], "32"); // 21
            successfulTests += assertTest(testName, baseChar + 1, (int) msg->mtext[1], "33"); // 

            // Test complete!
            printSummary(testName, successfulTests, NUM_TESTS);
        }
        release_memory_block(msg);
        counter++;
    }
}

/**************************************************************************//**
 * @brief: A low priority process that sends messages randomly to proc 1, 2 and 3
 *****************************************************************************/
void proc4(void)
{
    MSG_BUF *msg;

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "10 \n\r");
    delayed_send(PID_P2, msg, 100);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "7 \n\r");
    delayed_send(PID_P1, msg, 70);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "12 \n\r");
    delayed_send(PID_P1, msg, 120);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "19 \n\r");
    delayed_send(PID_P1, msg, 190);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "20 \n\r");
    delayed_send(PID_P3, msg, 200);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "14 \n\r");
    delayed_send(PID_P2, msg, 140);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "21 \n\r");
    delayed_send(PID_P3, msg, 210);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "11 \n\r");
    delayed_send(PID_P2, msg, 110);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "8 \n\r");
    delayed_send(PID_P2, msg, 80);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "4 \n\r");
    delayed_send(PID_P1, msg, 40);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "5 \n\r");
    delayed_send(PID_P2, msg, 50);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "9 \n\r");
    delayed_send(PID_P3, msg, 90);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "6 \n\r");
    delayed_send(PID_P3, msg, 60);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "1 \n\r");
    delayed_send(PID_P2, msg, 10);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "2 \n\r");
    delayed_send(PID_P2, msg, 20);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "3 \n\r");
    delayed_send(PID_P3, msg, 30);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "16 \n\r");
    delayed_send(PID_P1, msg, 160);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "17 \n\r");
    delayed_send(PID_P1, msg, 170);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "18 \n\r");
    delayed_send(PID_P2, msg, 180);

    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "13 \n\r");
    delayed_send(PID_P1, msg, 130);
    
    msg = (MSG_BUF*) request_memory_block();
    strcpy(msg->mtext, "15 \n\r");
    delayed_send(PID_P3, msg, 150);

    while (1) {
        release_processor();
    }
}

/**************************************************************************//**
 * @brief: a lowest priority process that releases the processor
 *****************************************************************************/
void proc5(void)
{
    while(1) {}
}

/**************************************************************************//**
 * @brief: a lowest priority process that releases the processor
 *****************************************************************************/
void proc6(void)
{
    while(1) {}
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
