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

#define MSG_BUF_SIZE 92

int registrationTable[52];
char msgBuf[MSG_BUF_SIZE];

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

void printMessageType(int type) {
	uart1_put_string("\t Message Type: ");
	switch (type) {
		case DEFAULT: 
			uart1_put_string("DEFAULT"); break;
		case KCD_REG: 
			uart1_put_string("KCD_REG"); break;
		case KCD_CMD: 
			uart1_put_string("KCD_CMD"); break;
		case CRT_DISPLAY: 
			uart1_put_string("CRT_DISPLAY"); break;
		case KEY_IN: 
			uart1_put_string("KEY_IN"); break;
		case CLOCK_TICK: 
			uart1_put_string("CLOCK_TICK"); break;
		case COUNT_REPORT: 
			uart1_put_string("COUNT_REPORT"); break;
		case WAKEUP_10: 
			uart1_put_string("WAKEUP_10"); break;
	}
}

void printMessageRecord(MSG_RECORD msgRecord) {
	uart1_put_string("  Sender: ");
	printNumber(msgRecord.m_send_pid);
	uart1_put_string("\tReceiver: ");
	printNumber(msgRecord.m_recv_pid);
	uart1_put_string("\tTimestamp: ");
	printNumber(msgRecord.m_timestamp);
	printMessageType(msgRecord.mtype);
	uart1_put_string("\r\n    Message: '");
	printStringEscaped(msgRecord.mtext, 16);
	uart1_put_string("'\r\n");
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
			uart1_put_string("\r\nSent Messages: \r\n");
			for (int i = 0; i < sent_msgs_size; i++) {
				printMessageRecord(sent_msgs[(i + sent_msgs_start) % 10]);
			}
			
			uart1_put_string("\r\nReceived Messages:\r\n");
			for (int i = 0; i < received_msgs_size; i++) {
				printMessageRecord(received_msgs[(i + received_msgs_start) % 10]);
			}
			
			break;
		default:
			return FALSE;
			// Not a hotkey! Something went wrong...
	}
	return TRUE;
}

void KCDProc(void) {
	for (int i = 0; i < 52; i++) {
		registrationTable[i] = -1;
	}

	char* msg_char_ptr = msgBuf;
	int bufCounter = 0;
	char command;
	int charInteger;
	
	while (1) {
		int senderPid;
		MSG_BUF* msg = receive_message(&senderPid);
		char inputChar = msg->mtext[0];
		
		if (msg->mtype == KEY_IN) {
 			msg->mtype = CRT_DISPLAY;
			 
			// Implement backspace
			if (inputChar == '\b' || (int) inputChar == 127)  {
				if (bufCounter > 0) {
					msg_char_ptr--;
					bufCounter--;
					if (inputChar == '\b') {
						msg->mtext[1] = ' ';
						msg->mtext[2] = '\b';
						msg->mtext[3] = '\0';
					}
				}
				send_message(PID_CRT, msg);
				continue;
			}

			send_message(PID_CRT, msg);

			// Stop filling the buffer if the size is going to be exceeded
			// Leave a space for the \0 character
			if (bufCounter < MSG_BUF_SIZE - 1) {
				*msg_char_ptr = inputChar;
				msg_char_ptr++;
			}
			bufCounter++;

			if (inputChar == '\r') {
				// Terminate the message
				*msg_char_ptr = '\0';
				
				if (bufCounter < 2 || *msgBuf != '%' || bufCounter > MSG_BUF_SIZE) {
					sendUARTMsg("Invalid Command\r\n");
					goto FINISH_ENTER_COMMAND;
				}

				command = *(msgBuf+1);
				charInteger = charMap(command);

				if (charInteger == -1) {
					sendUARTMsg("Command not found\r\n");
					goto FINISH_ENTER_COMMAND;
				}
				
				if (registrationTable[charInteger] != -1) {
					MSG_BUF* command_msg_buf = (MSG_BUF*) request_memory_block();
					command_msg_buf->mtype = KCD_CMD;
					strcpy(command_msg_buf->mtext, msgBuf);
					send_message(registrationTable[charInteger], command_msg_buf);
				}
				
FINISH_ENTER_COMMAND:
				bufCounter = 0;
				msg_char_ptr = msgBuf;
			}
			
			#ifdef _DEBUG_HOTKEYS
			else if (hotKeyDecoder(inputChar) == TRUE) {
				// do nothing since hotkeyDecoder handles it
			}
			#endif
		}
		else if (msg->mtype == KCD_REG) {
			char regChar = msg->mtext[1];
			if ((regChar >= 'A' && regChar <= 'Z')  ||  (regChar >= 'a' && regChar <= 'z')) {
				registrationTable[charMap(regChar)] = senderPid;
			}
			release_memory_block(msg);
		}
	}
}
