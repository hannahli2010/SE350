/**************************************************************************//**
 * @file        ae_proc4.c
 * @brief       Ensure that get/set priority function correctly for invalid
 *              input parameters. We test setting the null process' priority,
 *              setting a process' priority to the null priority, and setting
 *              or getting priorities of processes that don't exist.
 *
 *              We also test that the null priority is reachable in this test
 *              set by blocking all other functions after the initial set of 
 *              tests, but we exclude this from our test summary since it is
 *              impossible to break from the null process in this deliverable.
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
 * G_10_test_4: START
 * G_10_test_4: test 1 OK
 * G_10_test_4: test 2 OK
 * G_10_test_4: test 3 OK
 * G_10_test_4: test 4 OK
 * G_10_test_4: test 5 OK
 * G_10_test_4: test 6 OK
 * G_10_test_4: test 7 OK
 * G_10_test_4: test 8 OK
 * G_10_test_4: test 9 OK
 * G_10_test_4: 9/9 tests OK
 * G_10_test_4: 0/9 tests FAIL
 * G_10_test_4: END
 *
 * Expected UART2 output:
 * 
 * get_process_priority(PID_P3) returned: 1 (expected: 1)
 * set_process_priority(PID_P3, LOWEST) returned: RTX_OK (expected: RTX_OK)
 * get_process_priority(PID_P3) returned: 3 (expected: 3)
 * set_process_priority(PID_NULL, MEDIUM) returned: RTX_ERR (expected: RTX_ERR)
 * get_process_priority(PID_NULL) returned: 4 (expected: 4)
 * set_process_priority(PID_P3, PRI_NULL) returned: RTX_ERR (expected: RTX_ERR)
 * get_process_priority(PID_P3) returned: 3 (expected: 3)
 * set_process_priority(-1, LOWEST) returned: RTX_ERR (expected: RTX_ERR)
 * get_process_priority(-1) returned: -1 (expected: -1)
 * ABCDE
 * (proc2: received mem block: ...)
 * FGHIJ
 * (proc2: received mem block: ...)
 * KLMNO
 * proc4:
 * proc5:
 * proc6:
 * proc1:
 * proc3:
 * (should reach null proc)
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
char * testName = "G_10_test_4";

void uart1_put_1digit_int(int i)
{
    if (i > 9 || i < -9) {
        uart1_put_string("invalid value");
    } else if (i < 0) {
        uart1_put_char('-');
        uart1_put_char('0' - i);
    } else {
        uart1_put_char('0' + i);
    }
}

void uart1_put_rtx_success(int i)
{
    if (i == RTX_OK) uart1_put_string("RTX_OK");
    else uart1_put_string("RTX_ERR");
}

/**************************************************************************//**
 * @brief: Tests for setting and getting priorities
 *****************************************************************************/
void proc1(void)
{
    printUart0(testName, "START");

    int x = 0;
    // Test getting a process priority
    x = get_process_priority(PID_P3);
    successfulTests += assertTest(testName, x, 1, "1");

    uart1_put_string("get_process_priority(PID_P3) returned: ");
    uart1_put_1digit_int(x);
    uart1_put_string(" (expected: 1)\n\r");

    // Test setting a process priority
    x = set_process_priority(PID_P3, LOWEST);
    successfulTests += assertTest(testName, x, RTX_OK, "2");
    uart1_put_string("set_process_priority(PID_P3, LOWEST) returned: ");
    uart1_put_rtx_success(x);
    uart1_put_string(" (expected: RTX_OK)\n\r");

    x = get_process_priority(PID_P3);
    successfulTests += assertTest(testName, x, 3, "3");
    uart1_put_string("get_process_priority(PID_P3) returned: ");
    uart1_put_1digit_int(x);
    uart1_put_string(" (expected: 3)\n\r");

    // Test setting the null process' priority (Should fail)
    x = set_process_priority(PID_NULL, MEDIUM);
    successfulTests += assertTest(testName, x, RTX_ERR, "4");
    uart1_put_string("set_process_priority(PID_NULL, MEDIUM) returned: ");
    uart1_put_rtx_success(x);
    uart1_put_string(" (expected: RTX_ERR)\n\r");

    x = get_process_priority(PID_NULL);
    successfulTests += assertTest(testName, x, 4, "5");
    uart1_put_string("get_process_priority(PID_NULL) returned: ");
    uart1_put_1digit_int(x);
    uart1_put_string(" (expected: 4)\n\r");

    // Test setting another process' priority to the null priority (Should fail)
    x = set_process_priority(PID_P3, PRI_NULL);
    successfulTests += assertTest(testName, x, RTX_ERR, "6");
    uart1_put_string("set_process_priority(PID_P3, PRI_NULL) returned: ");
    uart1_put_rtx_success(x);
    uart1_put_string(" (expected: RTX_ERR)\n\r");

    x = get_process_priority(PID_P3);
    successfulTests += assertTest(testName, x, 3, "7");
    uart1_put_string("get_process_priority(PID_P3) returned: ");
    uart1_put_1digit_int(x);
    uart1_put_string(" (expected: 3)\n\r");

    // Test setting the priority of a non-existant process (Should fail)
    x = set_process_priority(-1, LOWEST);
    successfulTests += assertTest(testName, x, RTX_ERR, "8");
    uart1_put_string("set_process_priority(-1, LOWEST) returned: ");
    uart1_put_rtx_success(x);
    uart1_put_string(" (expected: RTX_ERR)\n\r");
    
    // Test getting the priority of a non-existant process (Should fail)
    x = get_process_priority(-1);
    successfulTests += assertTest(testName, x, -1, "9");
    uart1_put_string("get_process_priority(-1) returned: ");
    uart1_put_1digit_int(x);
    uart1_put_string(" (expected: -1)\n\r");
    
    
    printSummary(testName, successfulTests, NUM_TESTS);

    release_processor();

    while(1) {
        uart1_put_string("proc1:\n\r");
        request_memory_block();
        release_processor();                
    }
}

/**************************************************************************//**
 * @brief: Continuously print five letters and then a request a memory block,
           until eventually becoming blocked.
 *****************************************************************************/
void proc2(void)
{
    int i = 0;
    int ret_val = 20;
    void *p_mem_blk;

    while ( 1 ) {
        if (i != 0 && i % 5 == 0) {
            uart1_put_string("\n\r");
            p_mem_blk = request_memory_block();
#ifdef DEBUG_0
            printf("proc2: received mem block: %x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart1_put_char('A' + i%26);
        i++;
    }
}

/**************************************************************************//**
 * @brief  All the following processes simply attempt to request a memory block
 *****************************************************************************/ 
void proc3(void)
{
    while(1) {
        uart1_put_string("proc3:\n\r");
        request_memory_block();
        release_processor();
    }
}

void proc4(void)
{
    while(1) {
        uart1_put_string("proc4:\n\r");
        request_memory_block();
        release_processor();
    }
}

void proc5(void)
{
    while(1) {
        uart1_put_string("proc5:\n\r");
        request_memory_block();
        release_processor();
    }
}

void proc6(void)
{
    while(1) {
        uart1_put_string("proc6:\n\r");
        request_memory_block();
        release_processor();
    }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
