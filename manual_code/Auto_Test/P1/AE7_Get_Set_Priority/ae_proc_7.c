/**************************************************************************//**
 * @file        ae_proc0.c
 * @brief       Ensure that get/set priority function correctly for invalid
 *              input parameters. We test setting the null process' priority,
 *              setting a process' priority to the null priority, and setting
 *              or getting priorities of processes that don't exist.
 *              
 * @version     V1.2021.01
 * @authors     Group 10
 * @date        2021 JAN
 * @note        Each process is in an infinite loop. Processes never terminate.
 * @details
 *
 *****************************************************************************/

#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

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
 * @brief: a process that prints five uppercase letters
 *         and then yields the cpu.
 *****************************************************************************/
void proc1(void)
{
    int x = 0;
    // Test getting a process priority
    x = get_process_priority(PID_P2);
    uart1_put_string("get_process_priority(PID_P2) returned: ");
    uart1_put_1digit_int(x);
    uart1_put_string(" (expected: 1)\n\r");

    // Test setting a process priority
    x = set_process_priority(PID_P2, LOWEST);
    uart1_put_string("set_process_priority(PID_P2, LOWEST) returned: ");
    uart1_put_rtx_success(x);
    uart1_put_string(" (expected: RTX_OK)\n\r");

    x = get_process_priority(PID_P2);
    uart1_put_string("get_process_priority(PID_P2) returned: ");
    uart1_put_1digit_int(x);
    uart1_put_string(" (expected: 3)\n\r");

    // Test setting the null process' priority (Should fail)
    x = set_process_priority(PID_NULL, MEDIUM);
    uart1_put_string("set_process_priority(PID_NULL, MEDIUM) returned: ");
    uart1_put_rtx_success(x);
    uart1_put_string(" (expected: RTX_ERR)\n\r");

    x = get_process_priority(PID_NULL);
    uart1_put_string("get_process_priority(PID_NULL) returned: ");
    uart1_put_1digit_int(x);
    uart1_put_string(" (expected: 4)\n\r");

    // Test setting another process' priority to the null priority (Should fail)
    x = set_process_priority(PID_P2, PRI_NULL);
    uart1_put_string("set_process_priority(PID_P2, PRI_NULL) returned: ");
    uart1_put_rtx_success(x);
    uart1_put_string(" (expected: RTX_ERR)\n\r");

    x = get_process_priority(PID_P2);
    uart1_put_string("get_process_priority(PID_P2) returned: ");
    uart1_put_1digit_int(x);
    uart1_put_string(" (expected: 3)\n\r");

    // Test setting the priority of a non-existant process (Should fail)
    x = set_process_priority(-1, LOWEST);
    uart1_put_string("set_process_priority(-1, LOWEST) returned: ");
    uart1_put_rtx_success(x);
    uart1_put_string(" (expected: RTX_ERR)\n\r");
    
    // Test getting the priority of a non-existant process (Should fail)
    x = get_process_priority(-1);
    uart1_put_string("get_process_priority(-1) returned: ");
    uart1_put_1digit_int(x);
    uart1_put_string(" (expected: -1)\n\r");
    
    
    uart1_put_string("proc1: End of testing \n\r");

    while(1) {
        
    }
}

/**************************************************************************//**
 * @brief  The following proceses are unused, and will not be executed
 *****************************************************************************/
void proc2(void)
{
    while(1) {
        uart1_put_string("proc2:\n\r");
        release_processor();
    }
}

void proc3(void)
{
    while(1) {
        uart1_put_string("proc3:\n\r");
        release_processor();
    }
}

void proc4(void)
{
    while(1) {
        uart1_put_string("proc4:\n\r");
        release_processor();
    }
}

void proc5(void)
{
    while(1) {
        uart1_put_string("proc5:\n\r");
        release_processor();
    }
}

void proc6(void)
{
    while(1) {
        uart1_put_string("proc6:\n\r");
        release_processor();
    }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
