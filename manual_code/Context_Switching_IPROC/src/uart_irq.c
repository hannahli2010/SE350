/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO SE 350 RTX LAB  
 *
 *        Copyright 2020-2021 Yiqing Huang and NXP Semiconductors
 *                          All rights reserved.
 *---------------------------------------------------------------------------
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice and the following disclaimer.
 *
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/**************************************************************************//**
 * @file        uart_irq.c
 * @brief       UART IRQ handler. It receives input char through RX interrupt
 *              and then writes a string containing the input char through
 *              TX interrupts. 
 *              
 * @version     V1.2021.01
 * @authors     Yiqing Huang and NXP Semiconductors
 * @date        2021 JAN
 *****************************************************************************/
#include <LPC17xx.h>
#include "uart_irq.h"
#include "uart_polling.h"

#include "k_sys_procs.h"
#include "k_rtx.h"
#include "k_inc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

/**************************************************************************//**
 * @brief: initialize the n_uart
 * NOTES: It only supports UART0. It can be easily extended to support UART1 IRQ.
 * The step number in the comments matches the item number in Section 14.1 on pg 298
 * of LPC17xx_UM
 *****************************************************************************/
int uart_irq_init(int n_uart) {

    LPC_UART_TypeDef *pUart;

    if ( n_uart ==0 ) {
        /*
        Steps 1 & 2: system control configuration.
        Under CMSIS, system_LPC17xx.c does these two steps
         
        -----------------------------------------------------
        Step 1: Power control configuration. 
                See table 46 pg63 in LPC17xx_UM
        -----------------------------------------------------
        Enable UART0 power, this is the default setting
        done in system_LPC17xx.c under CMSIS.
        Enclose the code for your refrence
        //LPC_SC->PCONP |= BIT(3);
    
        -----------------------------------------------------
        Step2: Select the clock source. 
               Default PCLK=CCLK/4 , where CCLK = 100MHZ.
               See tables 40 & 42 on pg56-57 in LPC17xx_UM.
        -----------------------------------------------------
        Check the PLL0 configuration to see how XTAL=12.0MHZ 
        gets to CCLK=100MHZin system_LPC17xx.c file.
        PCLK = CCLK/4, default setting after reset.
        Enclose the code for your reference
        //LPC_SC->PCLKSEL0 &= ~(BIT(7)|BIT(6));    
            
        -----------------------------------------------------
        Step 5: Pin Ctrl Block configuration for TXD and RXD
                See Table 79 on pg108 in LPC17xx_UM.
        -----------------------------------------------------
        Note this is done before Steps3-4 for coding purpose.
        */
        
        /* Pin P0.2 used as TXD0 (Com0) */
        LPC_PINCON->PINSEL0 |= (1 << 4);  
        
        /* Pin P0.3 used as RXD0 (Com0) */
        LPC_PINCON->PINSEL0 |= (1 << 6);  

        pUart = (LPC_UART_TypeDef *) LPC_UART0;     
        
    } else if ( n_uart == 1) {
        
        /* see Table 79 on pg108 in LPC17xx_UM */ 
        /* Pin P2.0 used as TXD1 (Com1) */
        LPC_PINCON->PINSEL4 |= (2 << 0);

        /* Pin P2.1 used as RXD1 (Com1) */
        LPC_PINCON->PINSEL4 |= (2 << 2);          

        pUart = (LPC_UART_TypeDef *) LPC_UART1;
        
    } else {
        return 1; /* not supported yet */
    } 
    
    /*
    -----------------------------------------------------
    Step 3: Transmission Configuration.
            See section 14.4.12.1 pg313-315 in LPC17xx_UM 
            for baud rate calculation.
    -----------------------------------------------------
    */
    
    /* Step 3a: DLAB=1, 8N1 */
    pUart->LCR = UART_8N1; /* see uart.h file */ 

    /* Step 3b: 115200 baud rate @ 25.0 MHZ PCLK */
    pUart->DLM = 0; /* see table 274, pg302 in LPC17xx_UM */
    pUart->DLL = 9;    /* see table 273, pg302 in LPC17xx_UM */
    
    /* FR = 1.507 ~ 1/2, DivAddVal = 1, MulVal = 2
       FR = 1.507 = 25MHZ/(16*9*115200)
       see table 285 on pg312 in LPC_17xxUM
    */
    pUart->FDR = 0x21;       
    
 

    /*
    ----------------------------------------------------- 
    Step 4: FIFO setup.
           see table 278 on pg305 in LPC17xx_UM
    -----------------------------------------------------
        enable Rx and Tx FIFOs, clear Rx and Tx FIFOs
    Trigger level 0 (1 char per interrupt)
    */
    
    pUart->FCR = 0x07;

    /* Step 5 was done between step 2 and step 4 a few lines above */

    /*
    ----------------------------------------------------- 
    Step 6 Interrupt setting and enabling
    -----------------------------------------------------
    */
    /* Step 6a: 
       Enable interrupt bit(s) wihtin the specific peripheral register.
           Interrupt Sources Setting: RBR, THRE or RX Line Stats
       See Table 50 on pg73 in LPC17xx_UM for all possible UART0 interrupt sources
       See Table 275 on pg 302 in LPC17xx_UM for IER setting 
    */
    /* disable the Divisior Latch Access Bit DLAB=0 */
    pUart->LCR &= ~(BIT(7)); 
    
    /* enable RBR and RLS interrupts */
    pUart->IER = IER_RBR | IER_RLS; 
    
    /* Step 6b: enable the UART interrupt from the system level */
    
    if ( n_uart == 0 ) {
        NVIC_EnableIRQ(UART0_IRQn); /* CMSIS function */
    } else if ( n_uart == 1 ) {
        NVIC_EnableIRQ(UART1_IRQn); /* CMSIS function */
    } else {
        return 1; /* not supported yet */
    }
    pUart->THR = '\0';
    return 0;
}


/**
 * @brief: use CMSIS ISR for UART0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine. 
 *       The actual c_UART0_IRQHandler does the rest of irq handling
 */
__asm void UART0_IRQHandler(void)
{
    PRESERVE8
    IMPORT c_UART0_IRQHandler
    CPSID I
    PUSH{r4-r11, lr}

IUART_SAVE
    // save the sp of the current running process into its PCB
    LDR     R1, =__cpp(&gp_current_process) // load R1 with &gp_current_process
    LDR     R2, [R1]                        // load R2 with gp_current_process value
    STR     SP, [R2, #PCB_MSP_OFFSET]       // save MSP to gp_current_process->mp_sp
    
    // save the interrupted process's PCB in gp_pcb_interrupted
    LDR     R3, =__cpp(&gp_pcb_interrupted) // load &gp_pcb_interrupted to R3
    STR     R2, [R3]                        // assign gp_current_process to gp_pcb_interrupted
    
    // GRP 10 IMPLEMENTATION
    // update gp_current_process to point to the gp_pcb_uart_iproc
    LDR     R1, =__cpp(&gp_pcb_uart_iproc) // load R1 with &gp_pcb_uart_iproc
    LDR     R2, [R1]                        // load R2 with gp_pcb_uart_iproc value
    LDR     R3, =__cpp(&gp_current_process) // load R1 with &gp_current_process
    STR     R2, [R3]                        // store the gp_pcb_uart_iproc value to address &gp_current_process
    
IUART_EXEC    
    // update gp_current_process to the PCB of timer_i_proc 
    LDR     R1, =__cpp(&gp_pcb_uart_iproc) // load R1 with &gp_pcb_uart_iproc 
    LDR     R2, [R1]                        // load R2 with gp_pcb_uart_iproc value
    LDR     SP, [R2, #PCB_MSP_OFFSET]       // load MSP with UART_IPROC's SP (i.e. gp_pcb_uart_iproc->mp_sp)
    
    BL c_UART0_IRQHandler                   // execute the uart i-process
    
IUART_RESTORE
    // GRP 10 IMPLEMENTATION
    // update the gp_current_process to gp_pcb_interrupted
    LDR     R1, =__cpp(&gp_pcb_interrupted) // load R1 with &gp_pcb_interrupted
    LDR     R2, [R1]                        // load R2 with gp_pcb_interrupted value
    LDR     R3, =__cpp(&gp_current_process) // load R1 with &gp_current_process
    STR     R2, [R3]                        // store the gp_pcb_interrupted value to address &gp_current_process
    
    // restore the interrupted process's PCB to gp_current_process
    LDR     R1, =__cpp(&gp_pcb_interrupted)
    LDR     R2, [R1]
    LDR     SP, [R2, #PCB_MSP_OFFSET]       // load MSP with gp_current_process->mp_sp

    // Return to the interrupted process
    CPSIE I
    POP{r4-r11, pc}
} 
/**
 * @brief: c UART0 IRQ Handler
 */
void c_UART0_IRQHandler(void)
{
    uartIProc();
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
