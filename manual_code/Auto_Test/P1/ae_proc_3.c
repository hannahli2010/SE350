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
 * @file        ae_proc3.c
 * @brief       The goal of this test is to ensure adding, removing and setting
                the priorities of processes in the blocked memory queue maintains
                the integrity of the blocked memory queue.
 * 
 *              Proc 1 requests both of the free memory blocks and then releases 
 *              execution to proc 2. Proc 2 and Proc 3 will subsequently request 
 *              memory blocks and become immediately blocked, returning 
 *              execution to Proc 1. 
 * 
 *              Proc 1 will set the priorities of Proc 3 and Proc 2 to High.
 *              This should swap the order of Proc 3 and Proc 2 in the blocked
 *              memory queue, such that when Proc 1 releases its first memory
 *              block, it is Proc 3 which is unblocked and preempts Proc 1 (even
 *              though Proc 2 chronologically was blocked first).
 *
 *              Proc 3 will then immediately release the memory block it gained, 
 *              thus unblocking Proc 2. Note that Proc 2 should not preempt Proc
 *              3, as they have the same priority. Instead, we release the processor,
 *              and only then does execution return to Proc 2. Proc 2 releases the
 *              the memory block it acquired, and then sets the priorities of Proc
 *              1, 3 and itself to Lowest (prempting itself) to begin the final
 *              testing steps.
 *
 *              In the final set of tests, we perform similar tests to those above,
 *              but instead we use Proc 4, 5, and 6 and we keep all three of their 
 *              priorities identical (Low). We begin by having Proc 4 request a 
 *              memory block, releasing the processor to Proc 5 and 6 who immediately
 *              block themselves by requesting memory and return execution back to
 *              Proc 4.
 *              
 *              Proc 4 should release its memory block, and this time, it is the first
 *              chronologically blocked process, Proc 5 which is unblocked. However,
 *              Proc 5 should not preempt Proc 4, as they still have the same priority.
 *              Once Proc 4 releases the processor, Proc 5 should be the highest
 *              priority unblocked process, gain execution, and print the summary.
 *              
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
 * G_10_test_3: START
 * G_10_test_3: test 1 OK
 * G_10_test_3: test 2 OK
 * G_10_test_3: test 3 OK
 * G_10_test_3: test 4 OK
 * G_10_test_3: test 5 OK
 * G_10_test_3: test 6 OK
 * G_10_test_3: test 7 OK
 * G_10_test_3: test 8 OK
 * G_10_test_3: test 9 OK
 * G_10_test_3: 9/9 tests OK
 * G_10_test_3: 0/9 tests FAIL
 * G_10_test_3: END
 * 
 * Expected UART1 Output:
 *  
 * ABCDE
 * 01234
 * P3P3P3P3P3
 * FGHIJ
 * returned to proc 3
 * 56789
 * P4P4P4P4P4
 * P5P5P5P5P5
 * P6P6P6P6P6
 * P4P4P4P4P4
 * P5P5P5P5P5
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
char * testName = "G_10_test_3";
int nextProcess = PID_P1;

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
    procs[2].m_priority   = MEDIUM;
    
    procs[3].mpf_start_pc = &proc4;
    procs[3].m_priority   = LOW;
    
    procs[4].mpf_start_pc = &proc5;
    procs[4].m_priority   = LOW;
    
    procs[5].mpf_start_pc = &proc6;
    procs[5].m_priority   = LOW;
}

/**************************************************************************//**
 * @brief: a medium priority process that requests 2 memory blocks and then 
 *         releases the processor. After the processor is returned, it changes
 *         the priority of proc3 and proc2 to HIGH in that order and releases
 *         both a memory block and the processor.
 *****************************************************************************/
void proc1(void)
{
    printUart1(testName, "START");

    int i = 0;
    int ret_val = 20;
    void *p_mem_blk;
    p_mem_blk = request_memory_block();

    while ( 1 ) {
        if ( i != 0 && i%5 == 0 ) {
            uart0_put_string("\n\r");
            if (i == 5) {
                p_mem_blk = request_memory_block();
#ifdef DEBUG_0
                printf("proc1: received mem block: %x\n", p_mem_blk);
#endif /* DEBUG_0 */
                // After the following process switch, we expect that the next
                //  process will be P2, the process with the next highest priority
                nextProcess = PID_P2;
                release_processor();
                
                successfulTests += assertTest(testName, nextProcess, PID_P1, "3");
            }
            if (i == 10) {
                set_process_priority(PID_P3, HIGH);
                set_process_priority(PID_P2, HIGH);
                
                // After the following memory release, we expect to P3 to become
                //  unblocked, because it had its priority set to High first, and
                //  should therefore be first in the blocked memory queue. Because
                //  P3's priority is higher than P1, P3 should preempt P1, and should
                //  run next
                nextProcess = PID_P3;
                ret_val = release_memory_block(p_mem_blk);

                successfulTests += assertTest(testName, nextProcess, PID_P1, "6"); ///

                if (ret_val == 0) {
                    release_processor();
                }
            }
        }
        uart0_put_char('A' + i%26);
        i++;
    }
}
/**************************************************************************//**
 * @brief: A medium priority process that continuously requests memory blocks
 *****************************************************************************/
void proc2(void)
{
    int i = 0;
    void *p_mem_blk;
    
    successfulTests += assertTest(testName, nextProcess, PID_P2, "1");
    while ( 1) {
        if (i == 10) {
            uart0_put_string("\n\r");
            break;
        }
        if ( i != 0 && i%5 == 0 ) {
            uart0_put_string("\n\r");

            // After the following memory request, we expect to become
            //  blocked, and so the next process to execute will be PID_P3
            nextProcess = PID_P3;
            p_mem_blk = request_memory_block();
            successfulTests += assertTest(testName, nextProcess, PID_P2, "6");
#ifdef DEBUG_0
            printf("proc2: received %x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart0_put_char('0' + i%10);
        i++;
    }

    set_process_priority(PID_P1, LOWEST);
    set_process_priority(PID_P3, LOWEST);
    release_memory_block(p_mem_blk);
    
    // After the following set priority, we expect P4 to preempt P2
    nextProcess = PID_P4;
    set_process_priority(PID_P2, LOWEST);
    while ( 1 ) {
    }
}
/**************************************************************************//**
 * @brief: A medium priority process that allocates memory and frees it right away
 *****************************************************************************/
void proc3(void)
{
    int i = 0;
    int ret_val = 20;
    void *p_mem_blk;
    successfulTests += assertTest(testName, nextProcess, PID_P3, "2");
    
    while ( 1) {
        if (i == 10) {
            uart0_put_string("\n\r");
            break;
        }
        if ( i != 0 && i%5 == 0 ) {
            uart0_put_string("\n\r");
            
            // After the following memory request, we expect to become
            //  blocked, and so the next process to execute will be PID_P1
            nextProcess = PID_P1;
            p_mem_blk = request_memory_block();
            successfulTests += assertTest(testName, nextProcess, PID_P3, "4");

            uart0_put_string("returned to proc 3\n\r");

            // After the following memory release, we expect P2 to become
            //  unblocked, but because P3 and P2 have the same priority, P3
            //  should not be preempted.
            nextProcess = PID_P3;
            release_memory_block(p_mem_blk);
            successfulTests += assertTest(testName, nextProcess, PID_P3, "5");
            
            // After the following process switch, we expect that the next
            //  process will be P2, the unblocked process with the next highest priority
            nextProcess = PID_P2;
            release_processor();
#ifdef DEBUG_0
            printf("proc3: received %x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart0_put_string("P3");
        i++;
    }
    uart0_put_string("proc3: end of testing\n\r");
    while ( 1 ) {
    }
}

/**************************************************************************//**
 * @brief: a low priority process that requests 2 memory blocks, then
 *         releases the processor. After it returns to the process, it frees
 *         one memory block
 *****************************************************************************/
void proc4(void)
{
    int i = 0;
    int ret_val = 20;
    void *p_mem_blk;

    while ( 1 ) {
        if ( i != 0 && i%5 == 0 ) {
            uart0_put_string("\n\r");
            if (i == 5) {
                p_mem_blk = request_memory_block();
#ifdef DEBUG_0
                printf("proc4: received mem block: %x\n", p_mem_blk);
#endif /* DEBUG_0 */

                // After the following process switch, we expect that the next
                //  process will be P5, the process with the next highest priority
                nextProcess = PID_P5;
                release_processor();
                successfulTests += assertTest(testName, nextProcess, PID_P4, "8");
            }
            if (i == 10) {
                // After the following memory release, we expect to P4 to become
                //  unblocked, because it was added to the blocked memory queue first.
                //  Because P4's priority is the same as P5, it will not be preempted
                ret_val = release_memory_block(p_mem_blk);

                if (ret_val == 0) {
                    // After the following process switch, we expect that the next
                    //  process will be P5, the process with the next highest priority
                    //  which was just unblocked
                    nextProcess = PID_P5;
                    release_processor();
                    break;
                }
            }
        }
        uart0_put_string("P4");
        i++;
    }
    while( 1 ) {

    } 
}

/**************************************************************************//**
 * @brief a low priority process that keeps allocating memory without freeing
 *****************************************************************************/
void proc5(void)
{
    int i = 0;
    void *p_mem_blk;
    
    successfulTests += assertTest(testName, nextProcess, PID_P5, "7");
    
    while ( 1) {
        if (i == 10) {
            uart0_put_string("\n\r");
            break;
        }
        if ( i != 0 && i%5 == 0 ) {
            uart0_put_string("\n\r");
            
            // After the following memory request, we expect to become
            //  blocked, and so the next process to execute will be PID_P6
            nextProcess = PID_P6;
            p_mem_blk = request_memory_block();
            successfulTests += assertTest(testName, nextProcess, PID_P5, "9");
#ifdef DEBUG_0
            printf("proc5: received %x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart0_put_string("P5");
        i++;
    }
    
    printSummary(testName, successfulTests, NUM_TESTS);
    while(1){
    }
}

/**************************************************************************//**
 * @brief a low priority process that keeps allocating memory without freeing
 *****************************************************************************/
void proc6(void)
{
    // successfulTests += assertTest(testName, nextProcess, PID_P6, "Extra2");
    int i = 0;
    int ret_val = 20;
    void *p_mem_blk;
    
    while ( 1) {
        if ( i != 0 && i%5 == 0 ) {
            uart0_put_string("\n\r");

            // After the following memory request, we expect to become
            //  blocked, and so the next process to execute will be PID_P4
            nextProcess = PID_P4;
            p_mem_blk = request_memory_block();
#ifdef DEBUG_0
            printf("proc6: received %x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart0_put_string("P6");
        i++;
    }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
