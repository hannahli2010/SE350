/**
 * @brief:  P2 public testing code 1
 * @author: Yiqing Huang
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

#define NUM_TESTS 23
int successfulTests = 0;
char * testName = "G_10_ae_3";
int nextProcess;

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
    procs[0].m_priority   = MEDIUM;
    
    procs[1].mpf_start_pc = &proc2;
    procs[1].m_priority   = HIGH;
    
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


/**
 * @brief: a process that prints five uppercase letters on each line
 *         and sends a message to P2
 */
void proc1(void)
{
    int     i = 0;
    int     j = 0;
    void    *p_blk;
    MSG_BUF *p_msg;
    char    *ptr;
    int ret = -1;
    
    uart1_put_string("proc1: requesting a mem_blk...\n\r");
    p_blk = request_memory_block();
    p_msg = p_blk;
    p_msg->mtype = DEFAULT;
    ptr = p_msg->mtext;
    *ptr++ = 'A';
    *ptr++ = '\n';
    *ptr++ = '\r';
    *ptr++ = '\0';
    
    uart1_put_string("proc1: send messages to proc2...\n\r");
    ret = send_message(PID_P2, p_blk);
    successfulTests += assertTest(testName, ret, RTX_OK, "2");
    
    while (1) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
            //ret_val = release_processor();
            p_blk = request_memory_block();
            p_msg = p_blk;
            p_msg->mtype = DEFAULT;
            ptr = p_msg->mtext;
            *ptr++ = ('0' + (j++)%10);
            *ptr++ = '\n';
            *ptr++ = '\r';
            *ptr++ = '\0';
            uart1_put_string("proc1: send a message to proc2...\n\r");
            ret = send_message(PID_P2, p_blk);
            successfulTests += assertTest(testName, ret, RTX_OK, "2");
        }
        if ( j == g_iterations ) {
            break;
        }
        uart1_put_char('A' + i%26);
        i++;
    }
    
    set_process_priority(PID_P1, LOWEST);
    while (1) {
        release_processor();
    }
}

/**
 * @brief: a process that prints five lower case letters
 *         and receives messages.
 */
void proc2(void)
{
    int     i = 0;
    int     j = 0;
    MSG_BUF *p_msg;
    void    *p_blk;
    int ret = -1;

    uart1_put_string("proc2: receiving messages ...\n\r");
    p_blk = receive_message(NULL);
    p_msg = p_blk;

    uart1_put_string("proc2: got a message - ");
    uart1_put_string(p_msg->mtext);
    ret = release_memory_block(p_blk);
    successfulTests += assertTest(testName, ret, RTX_OK, "2");
    
    while (1) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
            p_blk = receive_message(NULL);
            p_msg = p_blk;
            uart1_put_string("proc2: got a message - ");
            uart1_put_string(p_msg->mtext);
            ret = release_memory_block(p_blk);
            successfulTests += assertTest(testName, ret, RTX_OK, "2");
            j++;
        }
        if ( j == g_iterations ) {
            break;
        }
        uart1_put_char('a' + i%26);
        i++;
    }
    ret = set_process_priority(PID_P2, LOW);
    successfulTests += assertTest(testName, ret, RTX_OK, "2");
    while(1) {
        release_processor();
    }
}

/**
 * @brief a process that does delayed_send 5 messages in decreasing order of delay
 */

void proc3(void)
{
    char *ptr;
    int delay[] =  {60, 45, 30, 15, 0};
    int i=0;
    int num_msgs = 5;
    MSG_BUF *p_msg;
    void *p_blk;
    int ret = -1;

    uart1_put_string("proc3: entering..., starting delay_send\n\r");    
    while( i < num_msgs ) {
        p_blk = request_memory_block();
#ifdef DEBUG_0
        printf("=0%x, i =[%d], delay= %d\n\r",p_blk, i, delay[i]);
#endif // DEBUG_0
        
        p_msg = p_blk;
        p_msg->mtype = DEFAULT;
        ptr = (char *)p_msg->mtext;
        *ptr++ = 'a' + i;
        *ptr++ = '\n';
        *ptr++ = '\r';
        *ptr++ = '\0';
#ifdef DEBUG_0
        uart1_put_string("proc3: delayed_send to proc4...\n\r");
#endif // DEBUG_0
        ret = delayed_send(PID_P4, p_blk, delay[i++]);
        successfulTests += assertTest(testName, ret, RTX_OK, "2");
    }
    uart1_put_string("proc3: done with delay_send\n\r");
    
    while ( 1 ) {
        release_processor();
    }
}

/**
 * @brief a process that receives messages
 */

void proc4(void)
{
    MSG_BUF *p_msg;
    void    *p_blk;
    int     send_id;
    int ret = -1;
    int i = 0;

    uart1_put_string("proc4: entering...\n\r");
    while(1) {
        p_blk = receive_message(&send_id);
        p_msg = p_blk;
        uart1_put_string("proc4: received ");
        uart1_put_string(p_msg->mtext);
        ret = release_memory_block(p_blk);
        successfulTests += assertTest(testName, ret, RTX_OK, "2");
        i++;
        if (i == 5) {
            printSummary(testName, successfulTests, NUM_TESTS);
        }
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
