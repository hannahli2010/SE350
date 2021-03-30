#include "k_sys_procs.h"
#include "k_process.h"
#include "k_msg.h"
#include "rtx.h"
#include "uart_def.h"
#include "printf.h"
#include "ae_util.h"

#ifdef DEBUG_0
#include "uart_polling.h"
#endif /* DEBUG_0 */

uint32_t time = 0;

void nullProc(void) {
	#ifdef DEBUG_0
		printf("Null Proc\n");
	#endif
    while (1) {
    }
}

void aProc(void) {
	/*
	Register %Z command with the KCD
	Wait till %Z command is received
	num = 0
	loop forever
		get a message envelope
		set message_type field of to count_report
		set message_textfield of to num
		send the message to process B
		num = num + 1
		release_processor()
	endloop
	*/
	while (1) {
		set_process_priority(PID_A, LOWEST);
		release_processor();
	}
}

void bProc(void) {
	/*
	loop forever
		receive a message
		send the message to process C
	endloop
	*/
	while (1) {
		set_process_priority(PID_B, LOWEST);
		release_processor();
	}
}

void cProc(void) {
	/*
	create a local message queue
	loop forever
		if (local message queue is empty) then
			p <-receive a message
		else
			p <-dequeue the first message from the local message queue
		endif
		
		if msg_typeof p == count_report then
			if message_text% 20 == 0 then
				send "Process C" to CRT with msg envelope p
				hibernatefor 10 sec
			endif
		endif
		deallocate message envelope p
		release_processor()
	endloop


hibernate:
	q <-request_memory_block()
	use q to delayed_sendto itself with 10 sec delay and msg_type=wakeup10
	loop forever
		// block and let other process execute
		p <-receive a message
		if (msg_typeof p == wakeup10) then
			exit this loop
		else
			put message (p) on the local message queue
			for later processing
		endif
	endloop
	*/
	while (1) {
		set_process_priority(PID_C, LOWEST);
		release_processor();
	}
}

// Registers to 'C'
// Command: %C <process_id> <new_priority>
void setPrioProc(void) {
	MSG_BUF* regMsg = (MSG_BUF*) request_memory_block();
	regMsg->mtext[0] = 'C';
	regMsg->mtype = KCD_REG;
	send_message(PID_KCD, regMsg);
	
	int procId;

	while (1) {
		MSG_BUF* msg = receive_message(NULL);

		if (msg->mtype == KCD_CMD) {
			char* charPtr = msg->mtext;
			
			// Ditch all the white space until we get to a number char
			skipWhitespace(&charPtr);
		
			int numDigits = 0;
			for (;*charPtr >= '0' && *charPtr <= '9'; charPtr++) {
				numDigits++;
			}

			if (numDigits != 0) {
				procId = stringToNum(charPtr-numDigits, numDigits);	
			} else {
				sendUARTMsg("Set priority failed - invalid process ID!\r\n");
				release_memory_block(msg);
				continue;
			}

			skipWhitespace(&charPtr);
			numDigits = 0; // 3A
			for (;*charPtr >= '0' && *charPtr <= '9'; charPtr++) {
				numDigits++;
			}

			// Error if there is extra data after the expected parameters,
			// eg. '%C 5 4X' OR '%C 5 4 X'
			skipWhitespace(&charPtr);
			if (*charPtr != '\0') {
				goto SET_PRIO_ERROR;
			}

			if (numDigits != 0) {
				int res = set_process_priority(procId, stringToNum(charPtr-numDigits, numDigits));
				if (res == RTX_OK) {
					release_memory_block(msg);
					sendUARTMsg("Successfully set process priority!\r\n");
					continue;
				}
			}
			
			SET_PRIO_ERROR:
			sendUARTMsg("Set priority failed - invalid process priority!\r\n");
			release_memory_block(msg);
		} else {
			// this proc isn't expecting other message types
			release_memory_block(msg);
		}
	}
}

/* Registers to 'W'
 * Commands:
 * 		%WR			 			- sets clock time to 00:00:00
 * 		%WS hh:mm:ss 	- sets current wall clock time to hh:mm:ss
 * 		%WT			 			- terminates wall clock
 */
void clockProc(void) {
	while (1) {
		set_process_priority(PID_CLOCK, LOWEST);
		release_processor();
	}
}

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
