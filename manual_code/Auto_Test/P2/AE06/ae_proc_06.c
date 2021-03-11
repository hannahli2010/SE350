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
 * Expected UART2 Output:
 *
 * G_10_test_6: test 1 OK
 * G_10_test_6: test 2 OK
 * G_10_test_6: test 3 OK
 * G_10_test_6: test 4 OK
 * G_10_test_6: test 5 OK
 * G_10_test_6: test 6 OK
 * G_10_test_6: test 7 OK
 * G_10_test_6: test 8 OK
 * G_10_test_6: test 9 OK
 * G_10_test_6: 9/9 tests OK
 * G_10_test_6: 0/9 tests FAIL
 * G_10_test_6: END
 *
 * Expected UART1 Output:
 * HELLO WORLD!
 * 1 - MSG HEY :)
 * 2 - MSG YO!
 * 
 *-------------------------------------------------------------------------------*/ 

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"
#include "ae_util.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

#define NUM_TESTS 9

int successfulTests = 0;
char * testName = "G_10_test_6";
int nextProcess;

/**************************************************************************//**
 * @brief: a high priority process
 *****************************************************************************/
void proc1(void)
{
    printUart1(testName, "START");
#ifdef DEBUG_0
    printf("Start of proc1\n");
#endif /* DEBUG_0 */
    
    nextProcess = PID_P2;
    // This will block proc1 since no message has been sent yet
    int expected_process;
    MSG_BUF* message = receive_message(&expected_process);
    // Print the Hello World message we previously received
    int i = 0;
    while(message->mtext[i] != '\0' && i < 50) {
        uart0_put_char(message->mtext[i]);
        i++;
    }
    
    // Assert we got the message sender, the message type, and the first message character
    successfulTests += assertTest(testName, PID_P2, expected_process, "2");
    successfulTests += assertTest(testName, DEFAULT, message->mtype, "3");
    successfulTests += assertTest(testName, (int)'H', (int) message->mtext[0], "4");

    // Release the block containing the message we just just recieved
    int release_result = release_memory_block((void*) message);
    successfulTests += assertTest(testName, RTX_OK, release_result, "5");

    // Set the priority of the next processes
    set_process_priority(PID_P3, HIGH);
    set_process_priority(PID_P4, HIGH);

    nextProcess = PID_P3;
    release_processor();

    #ifdef DEBUG_0
        printf("Shouldn't reach here (Proc 1)!");
    #endif /* DEBUG_0 */

    while(1) {}
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
    strcpy(msg->mtext, "HELLO WORLD!\n");

    nextProcess = PID_P1;
    // send message to proc1 which will unblock and preempt proc2
    send_message(PID_P1, msg);

    #ifdef DEBUG_0
        printf("Shouldn't reach here (Proc 2)!");
    #endif /* DEBUG_0 */

    while(1) {}
}
/**************************************************************************//**
 * @brief: a process that sends two messages
 *****************************************************************************/
void proc3(void)
{
#ifdef DEBUG_0
    printf("Start of proc3\n");
#endif /* DEBUG_0 */
    successfulTests += assertTest(testName, 3, nextProcess, "6");

    MSG_BUF *msg1 = (MSG_BUF*) request_memory_block();
    MSG_BUF *msg2 = (MSG_BUF*) request_memory_block();

    msg1->mtype = DEFAULT;
    msg2->mtype = DEFAULT;
    
    strcpy(msg1->mtext, "1 - MSG HEY :)\n");
    strcpy(msg2->mtext, "2 - MSG YO!\n");

    send_message(PID_P4, msg1);
    send_message(PID_P4, msg2);

    nextProcess = PID_P4;
    // send message to proc1 which will unblock and preempt proc2
    release_processor();
}

/**************************************************************************//**
 * @brief: A process that recieves two messages
 *****************************************************************************/
void proc4(void)
{
#ifdef DEBUG_0
    printf("Start of proc4\n");
#endif /* DEBUG_0 */
    successfulTests += assertTest(testName, 4, nextProcess, "7");

    // Recieve first message
    MSG_BUF* msg1 = receive_message(NULL);   
    // Print message contents
    int i = 0;
    while(msg1->mtext[i] != '\0' && i < 50) {
        uart0_put_char(msg1->mtext[i]);
        i++;
    }
    // Assert the first character of the message (ie. assert we got the right message)
    successfulTests += assertTest(testName, (int)'1', (int) msg1->mtext[0], "8");

    // Recieve second message
    MSG_BUF* msg2 = receive_message(NULL);
    i = 0;
    // Print message contents
    while(msg2->mtext[i] != '\0' && i < 50) {
        uart0_put_char(msg2->mtext[i]);
        i++;
    }
    
    // Assert the first character of the message (ie. assert we got the right message)
    successfulTests += assertTest(testName, (int)'2', (int) msg2->mtext[0], "9");

    printSummary(testName, successfulTests, NUM_TESTS);
#ifdef DEBUG_0
    printf("End of Testing");
#endif /* DEBUG_0 */
    while(1) {
    }
}

/**************************************************************************//**
 * @brief: a lowest priority process that releases the processor
 *****************************************************************************/
void proc5(void)
{
    #ifdef DEBUG_0
        printf("Start of proc5\n");
    #endif /* DEBUG_0 */
    while(1) {
    }
}

/**************************************************************************//**
 * @brief: a lowest priority process that releases the processor
 *****************************************************************************/
void proc6(void)
{
    #ifdef DEBUG_0
        printf("Start of proc6\n");
    #endif /* DEBUG_0 */
    while(1) {
    }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
