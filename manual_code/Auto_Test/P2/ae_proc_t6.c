/**
 * @brief:  P2 - T4
 * @author: Group 10
 * @date:   2020/03/10
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

/* expected output at COM1 (polled terminal) */
/* initialization table item */

#include "rtx.h"
#include "ae_proc.h"
#include "ae_util.h"
#include "uart_polling.h"

int g_iterations;

void set_test_procs(PROC_INIT *procs, int num)
{
    int i;
    for( i = 0; i < num; i++ ) {
        procs[i].m_pid        = (U32)(i+1);
        procs[i].m_stack_size = 0x200;
    }
  
    procs[0].mpf_start_pc = &proc1;
    procs[0].m_priority   = HIGH;
    
    procs[1].mpf_start_pc = &proc2;
    procs[1].m_priority   = LOW;
    
    procs[2].mpf_start_pc = &proc3;
    procs[2].m_priority   = LOW;
    
    procs[3].mpf_start_pc = &proc4;
    procs[3].m_priority   = LOW;
    
    procs[4].mpf_start_pc = &proc5;
    procs[4].m_priority   = LOWEST;
    
    procs[5].mpf_start_pc = &proc6;
    procs[5].m_priority   = LOWEST;
    
    g_iterations = 5;
}

#define NUM_TESTS 4
int successfulTests = 0;
char * testName = "G_10_t6";

/**
 * @brief: a process that prints five uppercase letters on each line
 *         and sends a message to P2
 */
void proc1(void)
{
    printUart1(testName, "START");

    MSG_BUF* msg = (MSG_BUF*) request_memory_block();
    MSG_BUF* msg2 = (MSG_BUF*) request_memory_block();
    msg->mtype = DEFAULT;
    msg2->mtype = DEFAULT;

    int val = send_message(PID_TIMER_IPROC, (void *) msg);
    successfulTests += assertTest(testName, val, RTX_ERR, "1");

    val = delayed_send(PID_TIMER_IPROC, (void *) msg2, 42);
    successfulTests += assertTest(testName, val, RTX_ERR, "2");

    val = send_message(PID_P1, (void *) msg);
    successfulTests += assertTest(testName, val, RTX_OK, "3");

    val = delayed_send(PID_P1, (void *) msg2, 42);
    successfulTests += assertTest(testName, val, RTX_OK, "4");

    printSummary(testName, successfulTests, NUM_TESTS);
    
    while (1) {
        release_processor();
    }
}

void proc2(void)
{
    while (1) {
        release_processor();
    }
}

void proc3(void)
{
    while (1) {
        release_processor();
    }
}

void proc4(void)
{
    while (1) {
        release_processor();
    }
}

void proc5(void)
{
    while (1) {
        release_processor();
    }
}

void proc6(void)
{
    while (1) {
        release_processor();
    }
}
