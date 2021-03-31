/**
 * @brief:  P2 - T4
 * @author: Group 10
 * @date:   2020/03/10
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

/* expected output at COM1 (polled terminal) 
proc2: receiving messages ...
proc1: requesting a mem_blk...
proc1: send messages to proc2...
proc2: got a message - A
abcde
ABCDE
proc1: send a message to proc2...
proc2: got a message - 0
fghij
FGHIJ
proc1: send a message to proc2...
proc2: got a message - 1
klmno
KLMNO
proc1: send a message to proc2...
proc2: got a message - 2
pqrst
PQRST
proc1: send a message to proc2...
proc2: got a message - 3
uvwxy
UVWXY
proc1: send a message to proc2...
proc2: got a message - 4
proc3: entering..., starting delay_send
proc3: done with delay_send
proc4: entering...
proc4: received e
proc4: received d
proc4: received c
proc4: received b
proc4: received a
*/

#include "rtx.h"
#include "ae_proc.h"
#include "ae_util.h"
#include "uart_polling.h"
#include "printf.h"

int g_iterations;
/* initialization table item */
void set_test_procs(PROC_INIT *procs, int num)
{
    int i;
    for( i = 0; i < num; i++ ) {
        procs[i].m_pid        = (U32)(i+1);
        procs[i].m_stack_size = 0x200;
    }
  
    procs[0].mpf_start_pc = &proc1;
    procs[0].m_priority   = LOW;
    
    procs[1].mpf_start_pc = &proc2;
    procs[1].m_priority   = LOW;
    
    procs[2].mpf_start_pc = &proc3;
    procs[2].m_priority   = LOW;
    
    procs[3].mpf_start_pc = &proc4;
    procs[3].m_priority   = LOW;
    
    procs[4].mpf_start_pc = &proc5;
    procs[4].m_priority   = LOW;
    
    procs[5].mpf_start_pc = &proc6;
    procs[5].m_priority   = LOW;
    
    g_iterations = 5;
}

#define NUM_TESTS 96
int successfulTests = 0;
int testNum = 0;
char * testName = "G_10_t4";

/**
 * @brief: a process that prints five uppercase letters on each line
 *         and sends a message to P2
 */
void proc1(void)
{
    int     i = 0;
    int     j = 0;
    int     sendProcId = 0;
    int     senderId;
    void    *p_blk;
    MSG_BUF *p_msg;
    char    *ptr;
    
    #ifdef DEBUG_0
    uart1_put_string("proc1: requesting a mem_blk...\n\r");
    #endif
    
    while (1) {
        if (j == 4) {
            printSummary(testName, successfulTests, NUM_TESTS);
    
            break;
        }
        if (j == 0) {
            p_blk = request_memory_block();
            p_msg = p_blk;
            p_msg->mtype = DEFAULT;
            ptr = p_msg->mtext;
            sendProcId = 2 + i%5;

            *ptr++ = ('0' + sendProcId);
            *ptr++ = '\n';
            *ptr++ = '\r';
            *ptr++ = '\0';

            #ifdef DEBUG_0
                uart1_put_string("proc1: sending ");
                printNumber(i);
                uart1_put_string("th memory block to process ");
                printNumber(sendProcId);
                uart1_put_string("with message val ");
                uart1_put_string(p_msg->mtext);
            #endif

            send_message(sendProcId, p_blk);
        } else {
            p_blk = receive_message(&senderId);
            p_msg = p_blk;

            char str[12];
            char myStr[12];
            int i = 0;
            int num = ++testNum;
            
            if (num == 0) {
                str[i++] = '0';
            }

            while (num > 0) {
                str[i++] = '0' + (num % 10);
                num /= 10;
            }

            int j = 0;
            while (i --> 0) { // lolol
                myStr[j++] = str[i];
            }

            myStr[j] = '\0';

            successfulTests += assertTest(testName, senderId, (int) p_msg->mtext[0] - (int)'0', myStr);

            sendProcId = (testNum+j) % 5 + 2;
            p_msg->mtext[0] = '0' + sendProcId;
            send_message(sendProcId, p_blk);
        }
        i++;
        if (i == 32) {
            #ifdef DEBUG_0
            if (j == 0) {
                uart1_put_string("proc1: sent all mem blocks");
            } else {
                uart1_put_string("proc1: resent all mem blocks");
            }
            #endif

            j++;
            i = 0;
        }
    }
    
    set_process_priority(PID_P1, LOWEST);
    while (1) {
        release_processor();
    }
}

void proc2(void)
{
    void    *p_blk;
    
    // uart1_put_string("proc2: receiving messages ...\n\r");
    while (1) {
        p_blk = receive_message(NULL);
        // uart1_put_string("proc2: got a message - ");
        send_message(PID_P1, p_blk);
    }
}

void proc3(void)
{
    void    *p_blk;
    
    // uart1_put_string("proc3: receiving messages ...\n\r");
    while (1) {
        p_blk = receive_message(NULL);
        // uart1_put_string("proc3: got a message - ");
        send_message(PID_P1, p_blk);
    }
}

void proc4(void)
{
    void    *p_blk;
    
    // uart1_put_string("proc4: receiving messages ...\n\r");
    while (1) {
        p_blk = receive_message(NULL);
        // uart1_put_string("proc4: got a message - ");
        send_message(PID_P1, p_blk);
    }
}

void proc5(void)
{
    void    *p_blk;
    
    // uart1_put_string("proc5: receiving messages ...\n\r");
    while (1) {
        p_blk = receive_message(NULL);
        // uart1_put_string("proc5: got a message - ");
        send_message(PID_P1, p_blk);
    }
}

void proc6(void)
{
    void    *p_blk;
    
    // uart1_put_string("proc6: receiving messages ...\n\r");
    while (1) {
        p_blk = receive_message(NULL);
        // uart1_put_string("proc6: got a message - ");
        send_message(PID_P1, p_blk);
    }
}
