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
 * @brief       The goal of this test is to verify that processes can get blocked
 *              on message, can pre-empt when unblocked, and can send and receive
 *              messages in either order.
 *
 *              Proc1 gets blocked on message until proc2 sends one to it. Since
 *              proc1 is of a higher priority, it pre-empts proc2 when being
 *              unblocked. Proc1 then raises the priorities of proc3 and proc4 to
 *              HIGH and releases the processor.
 *
 *              Proc3 sends two messages to proc4 and releases the processor to
 *              proc4. Proc4 then receives the 2 messages.
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

/* initialization table item */
void set_test_procs(PROC_INIT *procs, int num)
{
    int i;
    for( i = 0; i < num; i++ ) {
        procs[i].m_pid        = (U32)(i+1);
        procs[i].m_stack_size = USR_SZ_STACK;
    }
  
    procs[0].mpf_start_pc = &proc1;
    procs[0].m_priority   = HIGH;
    
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
 * @brief: a high priority process that tries to receive a message, resulting in
           it getting blocked. After it receives the messge, it releases the memory
           block (so this test also works if there's only 2 memory blocks, but it
           is not necessary) and raises the priority levels of proc3 and proc4
           to HIGH. It will then release_processor() to proc3.
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
 * @brief: a medium priority process that sends a message to proc1 and gets pre-empted
 *****************************************************************************/
void proc2(void)
{
#ifdef DEBUG_0
    printf("Start of proc2\n");
#endif /* DEBUG_0 */
    successfulTests += assertTest(testName, 2, nextProcess, "1");

    MSG_BUF *msg = (MSG_BUF*) request_memory_block();

    msg->mtype = DEFAULT;
    strcpy(msg->mtext, "HELLO WORLD!\n\r");

    nextProcess = PID_P1;
    // send message to proc1 which will unblock and preempt proc2
    send_message(PID_P1, msg);

    #ifdef DEBUG_0
        printf("Shouldn't reach here (Proc 2)!");
    #endif /* DEBUG_0 */

    while(1) {}
}
/**************************************************************************//**
 * @brief: An initially lowest priority process that gets raised to HIGH.
           It sends two messages to proc4, then releases the processor.
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
    
    strcpy(msg1->mtext, "1 - MSG HEY :)\n\r");
    strcpy(msg2->mtext, "2 - MSG YO!\n\r");

    send_message(PID_P4, msg1);
    send_message(PID_P4, msg2);

    nextProcess = PID_P4;
    // send message to proc1 which will unblock and preempt proc2
    release_processor();
}

/**************************************************************************//**
 * @brief: An initially lowest priority process that gets raised to HIGH.
           It receives two messages from proc3.
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
    uart0_printMsgText(msg1);

    // Assert the first character of the message (ie. assert we got the right message)
    successfulTests += assertTest(testName, (int)'1', (int) msg1->mtext[0], "8");

    // Recieve second message
    MSG_BUF* msg2 = receive_message(NULL);
    // Print message contents
    uart0_printMsgText(msg2);
    
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
