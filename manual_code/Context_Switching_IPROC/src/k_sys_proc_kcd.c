#include "k_sys_procs.h"
#include "k_process.h"
#include "k_msg.h"
#include "rtx.h"
#include "uart_def.h"
#include "printf.h"
#include "ae_util.h"
#include "uart_polling.h"

int charMap(char c) {
	if (c >= 'A' && c <= 'Z') return (int) c - (int) 'A';
	if (c >= 'a' && c <= 'z') return (int) c - (int) 'a' + 26;
	return -1;
}

char registrationTable[52][16];
char msgBuf[64];

void printProcAndPrio(PCB* p) {
	uart1_put_string("PID: ");
	printNumber(p->m_pid);
	uart1_put_string("\tPriority: ");
	printNumber(p->m_priority);
}

void printProcState(PCB* p) {
	uart1_put_string("\tState: ");
	switch (p->m_state) {
		case NEW: 
			uart1_put_string("NEW"); break;
		case RDY: 
			uart1_put_string("RDY"); break;
		case RUN: 
			uart1_put_string("RUN"); break;
		case BLOCKED_ON_RESOURCE: 
			uart1_put_string("BLOCKED_ON_RESOURCE"); break;
		case BLOCKED_ON_MESSAGE: 
			uart1_put_string("BLOCKED_ON_MESSAGE"); break;
		case IPROC: 
			uart1_put_string("IPROC"); break;
	}
}

int hotKeyDecoder(char hotkey) {
	PCB* it;
	switch(hotkey) {
		case '!':
			it = proc_ready_queue;
			uart1_put_string("\r\nReady Processes: \r\n");
			while (it != NULL) {
				printProcAndPrio(it);
				uart1_put_string("\r\n");
				it = it->mp_next;
			}
			break;
		case '@':
			it = proc_blocked_queue;
			uart1_put_string("\r\nBlocked on Memory Processes: \r\n");
			while (it != NULL) {
				printProcAndPrio(it);
				uart1_put_string("\r\n");
				it = it->mp_next;
			}
			break;
		case '#':
			uart1_put_string("\r\nBlocked on Message Processes: \r\n");
			for (int i = 0; i < NUM_SYS_PROC + NUM_TEST_PROCS; i++) {
				if (gp_pcbs[i]->m_state == BLOCKED_ON_MESSAGE) {
					printProcAndPrio(gp_pcbs[i]);
					uart1_put_string("\r\n");
				}
			}
			break;
		case '$':
			uart1_put_string("\r\nAll Processes: \r\n");
			for (int i = 0; i < NUM_SYS_PROC + NUM_TEST_PROCS; i++) {
				printProcAndPrio(gp_pcbs[i]);
				printProcState(gp_pcbs[i]);
				uart1_put_string("\r\n");
			}
			break;
		case '&':
			uart1_put_string("\r\nNumber of Free Memory Blocks: ");
			printNumber(num_free_mem_blks);
			uart1_put_string("\r\n");
			break;
		case '^':

			
			break;
		default:
			return FALSE;
			// Not a hotkey! Something went wrong...
	}
	return TRUE;
}

void KCDProc(void) {

	for (int i = 0; i < 52; i++) {
		for (int j = 0; j < 16; j++) {
			registrationTable[i][j] = FALSE;
		}
	}

	// Waiting for the user to input a %
	char isWaitingForCommand = FALSE;
	
	// Most recent command that has been entered
	char currCommand = NULL;

	// Variables relating to the current message we're building up
	// MSG_BUF* command_msg_buf = NULL;
	char* msg_char_ptr = NULL;
	// Lets see if we can do this today... (:D)>-|-<
	
	while (1) {
		// MSG_BUF* fullcommand = receive_message(NULL);
		int senderPtr;
		MSG_BUF* msg = receive_message(&senderPtr);
		char inputChar = msg->mtext[0];
		
		if (msg->mtype == KEY_IN) {
 			msg->mtype = CRT_DISPLAY;
			send_message(PID_CRT, msg);
			
			// Implement backspace
			if (inputChar == '\b') {
				if (isWaitingForCommand == TRUE) {
					isWaitingForCommand = FALSE;
				} else if (currCommand != NULL && msg_char_ptr == msgBuf) {
					currCommand = NULL;
					isWaitingForCommand = TRUE;
				} else if (currCommand != NULL) {
					msg_char_ptr--;
				}
			}
			// If a command has been chosen, add the new character to the message buff until new line
			else if (currCommand != NULL && inputChar != '\r') {
				*msg_char_ptr = inputChar;
				msg_char_ptr++;
			} 
			// If the user is waiting for a command
			else if (isWaitingForCommand && inputChar != '\r') {
				// validate character input
				if ((inputChar >= 'A' && inputChar <= 'Z')  ||  (inputChar >= 'a' && inputChar <= 'z')) {
					currCommand = inputChar;
					isWaitingForCommand = FALSE;
				} else {
					// Send an error message and have the user reenter the %
					isWaitingForCommand = FALSE;
					sendUARTMsg("\r\nInvalid command. Only alphabetic commands allowed \r\n");
				}
			}
			#ifdef _DEBUG_HOTKEYS
			else if (hotKeyDecoder(inputChar) == TRUE) {
				// do nothing since hotkeyDecoder handles it
			}
			#endif
			// start collecting the characters in a message buf
			else if (inputChar == '%') {
				isWaitingForCommand = TRUE;
				msg_char_ptr = msgBuf;
				
			} else if (inputChar == '\r') {
				// Determinate the message we have been building and send it to all registered processes
				if (currCommand != NULL) {
					*msg_char_ptr = '\0';
					int charInteger = charMap(currCommand);
					for (int i = 0; i < 16; i++) {
						if (registrationTable[charInteger][i] == TRUE) {
							MSG_BUF* command_msg_buf = (MSG_BUF*) request_memory_block();
							command_msg_buf->mtype = KCD_CMD;
							strcpy(command_msg_buf->mtext, msgBuf);
							send_message(i, command_msg_buf);
						} 
					}
					currCommand = NULL;
				}
			}
		}
		else if (msg->mtype == KCD_REG) {
			char regChar = msg->mtext[0];
			if ((regChar >= 'A' && regChar <= 'Z')  ||  (regChar >= 'a' && regChar <= 'z')) {
				registrationTable[charMap(regChar)][senderPtr] = TRUE;
			}
			release_memory_block(msg);
		}
	}
}
