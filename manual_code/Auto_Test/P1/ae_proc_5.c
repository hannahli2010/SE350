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
 * @file        ae_proc5.c
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
 * Assume there are 2 memory blocks
 * 
 * Expected UART2 Output:
 *
 * G_10_test_5: START
 * G_10_test_5: test 1 OK
 * G_10_test_5: test 2 OK
 * G_10_test_5: test 3 OK
 * G_10_test_5: test 4 OK
 * G_10_test_5: test 5 OK
 * G_10_test_5: test 6 OK
 * G_10_test_5: test 7 OK
 * G_10_test_5: test 8 OK
 * G_10_test_5: test 9 OK
 * G_10_test_5: 9/9 tests OK
 * G_10_test_5: 0/9 tests FAIL
 * G_10_test_5: END
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
char * testName = "G_10_test_5";
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

    int res1 = 20;
    int res2 = 20;
    void *p_mem_blk1;
    void *p_mem_blk2;
    p_mem_blk1 = request_memory_block();
    p_mem_blk2 = request_memory_block();

    res2 = release_memory_block(p_mem_blk2);
    res1 = release_memory_block(p_mem_blk1);

    successfulTests += assertTest(testName, RTX_OK, res1, "1");
    successfulTests += assertTest(testName, RTX_OK, res2, "2");

    p_mem_blk1 = request_memory_block();
    p_mem_blk2 = request_memory_block();

    res2 = release_memory_block(p_mem_blk2);
    res1 = release_memory_block(p_mem_blk1);

    successfulTests += assertTest(testName, RTX_OK, res1, "3");
    successfulTests += assertTest(testName, RTX_OK, res2, "4");

    p_mem_blk1 = request_memory_block();
    p_mem_blk2 = request_memory_block();
    
    res1 = release_memory_block(p_mem_blk1);
    successfulTests += assertTest(testName, RTX_OK, res1, "5");

    *((int*) p_mem_blk2) = 12;
    p_mem_blk1 = request_memory_block();
    successfulTests += assertTest(testName, 12, *((int*) p_mem_blk2), "6");

    res2 = release_memory_block(p_mem_blk2);
    successfulTests += assertTest(testName, RTX_OK, res2, "7");
    p_mem_blk2 = request_memory_block();
    successfulTests += assertTest(testName, 12, *((int*) p_mem_blk2), "8");

    release_memory_block(p_mem_blk1);
    release_memory_block(p_mem_blk2);
		
    release_processor();
}
/**************************************************************************//**
 * @brief
 *****************************************************************************/
void proc2(void)
{
    void *p_mem_blk;

    set_process_priority(PID_P1, LOW);
    p_mem_blk = request_memory_block();
    *((int*) p_mem_blk) = 12;

    release_processor();
    successfulTests += assertTest(testName, 12, *((int*) p_mem_blk), "9");
    printSummary(testName, successfulTests, NUM_TESTS);
}
/**************************************************************************//**
 * @brief
 *****************************************************************************/
void proc3(void)
{
    void *p_mem_blk;
    while(1){
        p_mem_blk = request_memory_block();
        *((int*) p_mem_blk) = 25;
    }
}

/**************************************************************************//**
 * @brief: a low priority process that releases the processor
 *****************************************************************************/
void proc4(void)
{
    release_processor();
}

/**************************************************************************//**
 * @brief: a low priority process that releases the processor
 *****************************************************************************/
void proc5(void)
{
    release_processor();
}

/**************************************************************************//**
 * @brief: a low priority process that releases the processor
 *****************************************************************************/
void proc6(void)
{
    release_processor();
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
