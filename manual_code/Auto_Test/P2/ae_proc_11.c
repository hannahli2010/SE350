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
    procs[1].m_priority   = HIGH;
    
    procs[2].mpf_start_pc = &proc3;
    procs[2].m_priority   = LOWEST;
    
    procs[3].mpf_start_pc = &proc4;
    procs[3].m_priority   = LOWEST;
    
    procs[4].mpf_start_pc = &proc5;
    procs[4].m_priority   = LOWEST;
    
    procs[5].mpf_start_pc = &proc6;
    procs[5].m_priority   = LOWEST;
    
    g_iterations = 5;
}

#define NUM_TESTS 3
int successfulTests = 0;
char * testName = "G_10_t11";

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

    release_processor();

    int val = send_message(PID_P2, (void *) msg);

    release_memory_block(msg2);
    
    release_processor();

    while(1) {

    }
}

void proc2(void)
{
    MSG_BUF* msg1 = (MSG_BUF*) request_memory_block();
    // PROC 2 BLOCKS

    // PROC 2 RETURNS - request a message
    MSG_BUF* msg2 = (MSG_BUF*) receive_message(NULL);

    if (msg1 != msg2) {
        successfulTests += assertTest(testName, 1, 1, "1");
    } else {
        successfulTests += assertTest(testName, 1, -1, "1");  
    }

    int res = release_memory_block(msg1);
    successfulTests += assertTest(testName, res, RTX_OK, "2");

    res = release_memory_block(msg2);
    successfulTests += assertTest(testName, res, RTX_OK, "3");

    printSummary(testName, successfulTests, NUM_TESTS);

    while(1) {

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
