/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO SE 350 RTOS LAB  
 *
 *                     Copyright 2020-2021 Yiqing Huang
 *
 *          This software is subject to an open source license and 
 *          may be freely redistributed under the terms of MIT License.
 ****************************************************************************
 */

/**************************************************************************//**
 * @file        ae_proc.c
 * @brief       Two auto test processes: proc1 and proc2
 *              
 * @version     V1.2021.01
 * @authors     Yiqing Huang
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

/**************************************************************************//**
 * @brief: a process that prints five uppercase letters
 *         and then yields the cpu.
 *****************************************************************************/
void proc1(void)
{
    int i = 0;
    int x = 0;
    int ret_val = 10;
    while (1) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
            ret_val = release_processor();
#ifdef DEBUG_0
            printf("proc1: ret_val=%d\n\r", ret_val);
#endif /* DEBUG_0 */
        }
        uart1_put_char('A' + i%26);
        for ( x = 0; x < DELAY; x++); // some artifical delay
        i++;
    }
}

/**************************************************************************//**
 * @brief  a process that prints five numbers
 *         and then yields the cpu.
 *****************************************************************************/
void proc2(void)
{
    int i = 0;
    int x = 0;
    int ret_val = 20;
    while (1) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
            ret_val = release_processor();
#ifdef DEBUG_0
            printf("proc2: ret_val=%d\n\r", ret_val);
#endif /* DEBUG_0 */
        }
        uart1_put_char('0' + i%10);
        for ( x = 0; x < DELAY; x++); // some artifical delay
        i++;
    }
}

void proc3(void)
{
		int x = 0;
    int ret_val = 30;
	
    while(1) {
				uart1_put_string("proc3: Rob\n\r");
			
				int * myVar = request_memory_block();
				*myVar = 44;		
				printf("myVar value: %d, location: 0x%x\n", *myVar, myVar);
				ret_val = release_processor();
			
#ifdef DEBUG_0
				printf("proc3: ret_val=%d\n\r", ret_val);
#endif /* DEBUG_0 */
			
				for ( x = 0; x < DELAY; x++); // some artifical delay
		}
}

void proc4(void)
{
		int i = 0;
		int x = 0;
		int * temp;
    int ret_val = 40;
    while(1) {
				if (i == 0){
					temp = request_memory_block();
				} else if (i == 2) {
					release_memory_block(temp);
				}
        uart1_put_string("proc4: Hannah\n\r");
        ret_val = release_processor();
#ifdef DEBUG_0
        printf("proc4: ret_val=%d\n\r", ret_val);
#endif /* DEBUG_0 */
				i++;
        for ( x = 0; x < DELAY; x++); // some artifical delay
    }
}

void proc5(void)
{
		int x = 0;
    int ret_val = 50;
    while(1) {
        uart1_put_string("proc5: Jackie\n\r");
        ret_val = release_processor();
#ifdef DEBUG_0
        printf("proc5: ret_val=%d\n\r", ret_val);
#endif /* DEBUG_0 */
			
        for ( x = 0; x < DELAY; x++); // some artifical delay
    }
}

void proc6(void)
{
		int x = 0;
    int ret_val = 60;
    while(1) {
        uart1_put_string("proc6: Ethan\n\r");
        ret_val = release_processor();
#ifdef DEBUG_0
        printf("proc6: ret_val=%d\n\r", ret_val);
#endif /* DEBUG_0 */
			
        for ( x = 0; x < DELAY; x++); // some artifical delay
    }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
