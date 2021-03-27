#include "k_sys_procs.h"
#include "k_process.h"
#include "k_msg.h"
#include "rtx.h"
#include "uart_def.h"
#include "printf.h"

uint32_t time = 0;

void nullProc(void)
{
	#ifdef DEBUG_0
		printf("Null Proc\n");
	#endif
    while (1) {
    }
}

void aProc(void) {
	while (1) {
		release_processor();
	}
}

void bProc(void) {
	while (1) {
		release_processor();
	}
}

void cProc(void) {
	while (1) {
		release_processor();
	}
}

void setPrioProc(void) {
	while (1) {
		release_processor();
	}
}

void clockProc(void) {
	while (1) {
		release_processor();
	}
}

void KCDProc(void) {
	while (1) {
		MSG_BUF* msg = receive_message(NULL);
		
		if (msg->mtype == KEY_IN) {
			msg->mtype = CRT_DISPLAY;
			send_message(PID_CRT, msg);
		}
	}
}

/* Message Types 
	#define DEFAULT             0
	#define KCD_REG             1
	#define KCD_CMD             2
	#define CRT_DISPLAY         3
	#define KEY_IN              4
*/

void CRTProc(void) {
	LPC_UART_TypeDef *pUart;
	pUart = (LPC_UART_TypeDef *) LPC_UART0;

	while( 1 ) {
		MSG_BUF* msg = receive_message(NULL);
		__set_CONTROL(0);

		if (msg->mtype == CRT_DISPLAY && msg->mtext[0] != '\0') {
			send_message(PID_UART_IPROC, msg);
			pUart->IER |= IER_THRE;
		}
		else {
			release_memory_block(msg);
		}

		__set_CONTROL(1);
	}	
}

void timerIProc(void) {
	/*
	// get pending requests
	
	while( pending messages to i-process ) {
		insert envelope into the timeout queue ;
	}
	while( first message in queue timeout expired ) {
		env ← dequeue( timeout_queue) ;
		target_pid ← destination_pid from env;
		// forward msg to destination
		k_send_message ( target_pid , env ) ;
	}
	*/

	/*
	Example demonstrating how sending messages modifies queue values
		// Initial queue status
		50 ticks (send at time 50)
		50 ticks (send at time 100)
		50 ticks (send at time 150)

		// State after requesting to send a message with delay 60
		50 ticks (send at time 50) <- message precedeing message is unchanged
		10 ticks (send at time 60) <- new message is relative to previous message
		40 ticks (send at time 100) <- message following inserted message is relative to new message
		50 ticks (send at time 150) <- all following messages are unchanged
	*/

	// Decrement the timer of the first message in the queue, if it exists
	if (delayed_msg_queue != NULL) {
		delayed_msg_queue->m_expiry--;
	}

	// Insert all the messages we just received into the delayed message queue
	DELAYED_MSG_BUF* newMsg;
	while (newMsg = (DELAYED_MSG_BUF*) k_receive_message_nb(NULL) ) {

		// If the list is empty, insert at the front of the list
		if (delayed_msg_queue == NULL) {
			newMsg->mp_next = NULL;
			delayed_msg_queue = newMsg;
		}
		// If the message should be inserted before the first element in the 
		//   queue, insert the new message to the head of the list
		else if (newMsg->m_expiry < delayed_msg_queue->m_expiry) {
			newMsg->mp_next = delayed_msg_queue;
			delayed_msg_queue = newMsg;
		}
		// Otherwise, insert the message to the correct placement in the 
		//   queue based on expiry order
		else {
			DELAYED_MSG_BUF* it = delayed_msg_queue;
			while (it->mp_next != NULL && newMsg->m_expiry - it->m_expiry >= it->mp_next->m_expiry) {
				// Delays are relative to the delay of the message before it
				// Decrememt the message's expiry by the previous message's delay
				newMsg->m_expiry -= it->m_expiry;
				it = it->mp_next;
			}
			newMsg->m_expiry -= it->m_expiry;
			
			// Insert the new message into the queue
			newMsg->mp_next = it->mp_next;
			it->mp_next = newMsg;
		}

		// If the message is the last message in the queue, we don't need to do anything
		//   Otherwise, we need to modify the delay of the proceeding message to be
		//   relative to the message we just inserted
		if (newMsg->mp_next != NULL) {
			newMsg->mp_next->m_expiry -= newMsg->m_expiry;
		}
	}

	// Check if message should be sent
	while (delayed_msg_queue != NULL && delayed_msg_queue->m_expiry == 0) {
		// Get the first message from the queue
		DELAYED_MSG_BUF* msg = delayed_msg_queue;

		// Remove from the queue the message we are sending
		delayed_msg_queue = delayed_msg_queue->mp_next;
		
		// Sending the message to the destination process
		k_send_message(msg->m_real_recv_pid, (void*) msg);
	}
}

uint8_t g_buffer[]= "You Typed a Q\n\r";
uint8_t *gp_buffer = g_buffer;
uint8_t g_char_in;
uint8_t g_char_out;

MSG_BUF* currMsg = NULL;
char* msg_char_ptr;

void uartIProc(void) {
	uint8_t IIR_IntId;        /* Interrupt ID from IIR */
    LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
		
	#ifdef DEBUG_0
		uart1_put_string("Entering c_UART0_IRQHandler\n\r");
	#endif // DEBUG_0

    /* Reading IIR automatically acknowledges the interrupt */
    IIR_IntId = (pUart->IIR) >> 1 ; /* skip pending bit in IIR */ 
    if (IIR_IntId & IIR_RDA) { /* Receive Data Avaialbe */
        /* Read UART. Reading RBR will clear the interrupt */
        g_char_in = pUart->RBR;
		#ifdef DEBUG_0
			uart1_put_string("Reading a char = ");
			uart1_put_char(g_char_in);
			uart1_put_string("\n\r");
		#endif /* DEBUG_0 */

        MSG_BUF* msg = k_request_memory_block();
		msg->mtype = KEY_IN;
		msg->mtext[0] = g_char_in;
		msg->mtext[1] = 0;
		if (g_char_in == '\r') {
			msg->mtext[1] = '\n';
			msg->mtext[2] = 0;
		}
		
        k_send_message(PID_KCD, msg);

    } else if (IIR_IntId & IIR_THRE) {
        /* THRE Interrupt, transmit holding register becomes empty */

		if (currMsg == NULL) {
			currMsg = k_receive_message_nb(NULL);
			// If we didn't actually get a message
			if (currMsg == NULL) {
				#ifdef DEBUG_0
					uart1_put_string("No waiting message. Turning off IER_THRE\n\r");
				#endif /* DEBUG_0 */
				pUart->IER ^= IER_THRE; // toggle the IER_THRE bit
				pUart->THR = '\r';
				return;
			}
			msg_char_ptr = currMsg->mtext;
		}

		g_char_out = *msg_char_ptr;
		#ifdef DEBUG_0
			printf("Writing a char = %c \n\r", g_char_out);
		#endif /* DEBUG_0 */
		pUart->THR = g_char_out;
		msg_char_ptr++;
        
		if (*msg_char_ptr == '\0' ) {
			#ifdef DEBUG_0
				uart1_put_string("Finish writing this message. \n\r");
			#endif /* DEBUG_0 */
			k_release_memory_block(currMsg);
			
			currMsg = k_receive_message_nb(NULL);
			// If we didn't actually get a message
			if (currMsg == NULL) {
				#ifdef DEBUG_0
					uart1_put_string("Turning off IER_THRE. \n\r");
				#endif /* DEBUG_0 */
				pUart->IER ^= IER_THRE; // toggle the IER_THRE bit
				return;
			}
			msg_char_ptr = currMsg->mtext;
		}
       
    } else {  /* not implemented yet */
		#ifdef DEBUG_0
			uart1_put_string("Should not get here!\n\r");
		#endif /* DEBUG_0 */
        return;
    }    
}
