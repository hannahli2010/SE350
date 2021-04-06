/**
 * @brief:  P2 - T3
 * @author: Group 10
 * @date:   2020/03/10
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

/* expected output at COM1 (polled terminal) 
proc1: start sending messages...
proc1: send a message 0
proc1: send a message 1
proc1: send a message 2
proc1: send a message 3
proc1: send a message 4
proc1: send a message 5
proc1: send a message 6
proc1: send a message 7
proc1: send a message 8
proc1: send a message 9
proc1: send a message 10
proc1: send a message 11
proc1: send a message 12
proc1: send a message 13
proc1: send a message 14
proc1: send a message 15
proc1: send a message 16
proc1: send a message 17
proc1: send a message 18
proc1: send a message 19
proc1: send a message 20
proc1: send a message 21
proc1: send a message 22
proc1: send a message 23
proc1: send a message 24
proc1: send a message 25
proc1: send a message 26
proc1: send a message 27
proc1: send a message 28
proc1: send a message 29
proc1: send a message 30
proc1: send a message 31
proc1: Request for memory after last memory block was sent
proc2: receiving first message ...
proc2: got a message - 0hi
proc1: Unblocked
proc1: Set priority to LOWEST
proc2: receiving more messages ...
proc2: got a message - 1hi, Number 1
proc2: got a message - 2hi, Number 2
proc2: got a message - 3hi, Number 3
proc2: got a message - 4hi, Number 4
proc2: got a message - 5hi, Number 5
proc2: got a message - 6hi, Number 6
proc2: got a message - 7hi, Number 7
proc2: got a message - 8hi, Number 8
proc2: got a message - 9hi, Number 9
proc2: got a message - 0hi, Number 10
proc2: got a message - 1hi, Number 11
proc2: got a message - 2hi, Number 12
proc2: got a message - 3hi, Number 13
proc2: got a message - 4hi, Number 14
proc2: got a message - 5hi, Number 15
proc2: got a message - 6hi, Number 16
proc2: got a message - 7hi, Number 17
proc2: got a message - 8hi, Number 18
proc2: got a message - 9hi, Number 19
proc2: got a message - 0hi, Number 20
proc2: got a message - 1hi, Number 21
proc2: got a message - 2hi, Number 22
proc2: got a message - 3hi, Number 23
proc2: got a message - 4hi, Number 24
proc2: got a message - 5hi, Number 25
proc2: got a message - 6hi, Number 26
proc2: got a message - 7hi, Number 27
proc2: got a message - 8hi, Number 28
proc2: got a message - 9hi, Number 29
proc2: got a message - 0hi, Number 30
proc2: got a message - 1hi, Number 31
proc2: got a message - 2ha, Number 32
proc2: Done
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
    procs[0].m_priority   = HIGH;
    
    procs[1].mpf_start_pc = &proc2;
    procs[1].m_priority   = MEDIUM;
    
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
    void    *p_blk;
    MSG_BUF *p_msg;
    char    *ptr;
    
    uart1_put_string("proc1: start sending messages...\n\r");
    
    while (1) {
        p_blk = request_memory_block();
        p_msg = p_blk;
        p_msg->mtype = DEFAULT;
        ptr = p_msg->mtext;
        *ptr++ = ('0' + i%10);
        *ptr++ = 'h';
        *ptr++ = 'i';
        *ptr++ = '\0';
        uart1_put_string("proc1: send a message ");
        printNumber(i);
        uart1_put_string("\n\r");
        send_message(PID_P2, p_blk);

        i++;
        if ( i == 32 ) {
            break;
        }
    }

    uart1_put_string("proc1: Request for memory after last memory block was sent\n\r");
    p_blk = request_memory_block();
    uart1_put_string("proc1: Unblocked\n\r");
    p_msg = p_blk;
    p_msg->mtype = DEFAULT;
    ptr = p_msg->mtext;
    *ptr++ = ('0' + i%10);
    *ptr++ = 'h';
    *ptr++ = 'a';
    *ptr++ = '\0';
    send_message(PID_P2, p_blk);
    
    uart1_put_string("proc1: Set priority to LOWEST\n\r");
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
    MSG_BUF *p_msg;
    void    *p_blk;
    
    uart1_put_string("proc2: receiving first message ...\n\r");
    p_blk = receive_message(NULL);
    p_msg = p_blk;

    uart1_put_string("proc2: got a message - ");
    uart1_put_string(p_msg->mtext);
    uart1_put_string("\n\r");
    i++;

    release_memory_block(p_blk);
    
    uart1_put_string("proc2: receiving more messages ...\n\r");

    while (1) {
        p_blk = receive_message(NULL);
        p_msg = p_blk;
        uart1_put_string("proc2: got a message - ");
        uart1_put_string(p_msg->mtext);
        uart1_put_string(", Number ");
        printNumber(i);
        uart1_put_string("\n\r");
        release_memory_block(p_blk);
        if ( i == 32 ) {
            break;
        }
        i++;
    }
    uart1_put_string("proc2: Done\n\r");
    while(1) {
        release_processor();
    }
}

void proc3(void)
{
    while(1) {
        uart1_put_string("proc4:\n\r");
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
