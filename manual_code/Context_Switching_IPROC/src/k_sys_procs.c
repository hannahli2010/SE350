#include "k_sys_procs.h"
#include "k_process.h"
#include "k_msg.h"
#include "rtx.h"
#include "uart_def.h"
#include "ae_util.h"
#include "k_queue.h"

#ifdef DEBUG_0
#include "printf.h"
#include "uart_polling.h"
#endif /* DEBUG_0 */

uint32_t time = 0;

#define IS_NUM(c) (c >= '0' && c <= '9')

#ifdef SIM_TARGET
#define CLOCK_DELAY 20     // delay for 1 second on the simulator <- NOT CORRECT YET
#else
#define CLOCK_DELAY 1000   // delay for 1 second on the board <- NOT CORRECT YET
#endif

void nullProc(void) {
	#ifdef DEBUG_0
		printf("Null Proc\n");
	#endif
    while (1) {
    }
}

void registerCommand(char c) {
	MSG_BUF* regMsg = (MSG_BUF*) request_memory_block();
	regMsg->mtext[0] = c;
	regMsg->mtype = KCD_REG;
	send_message(PID_KCD, regMsg);
}

void aProc(void) {
	registerCommand('Z');
	int num = 0;
	MSG_BUF* msg;
	
	// Wait until %Z command is recieved
	while (1) {
		msg = receive_message(NULL);
		if (msg->mtype == KCD_CMD) {
			release_memory_block(msg);
			break;
		}
		release_memory_block(msg);
	}

	while(1) {
		msg = (MSG_BUF*) request_memory_block();
		msg->mtype = COUNT_REPORT;
		numToString(num, msg->mtext);
		send_message(PID_B, msg);
		num++;
		release_processor();
	}

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
}

void bProc(void) {
	while (1) {
		MSG_BUF* msg = receive_message(NULL);
		if (msg->mtype == COUNT_REPORT) {
			send_message(PID_C, msg);
		} else {
			release_memory_block(msg);
		}
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
*/
	MSG_BUF* localQ = NULL;
	MSG_BUF* msg;

	while(1) {
		if (localQ == NULL) {
			msg = receive_message(NULL);
		} else {
			msg = localQ;
			localQ = msg->mp_next;
		}

		if (msg->mtype == COUNT_REPORT) {
			int num = stringToNumAndCount(msg->mtext);
			if (num % 2 == 0) {
				strcpy(msg->mtext, "Process C\n\r");
				msg->mtype = CRT_DISPLAY;
				send_message(PID_CRT, msg);
				
				// START OF HIBERNATE
				MSG_BUF *q = (MSG_BUF*) request_memory_block();
				q->mtype = WAKEUP_10;
				delayed_send(PID_C, q, CLOCK_DELAY*10); // 10 second delay

				while (1) {
					msg = receive_message(NULL);
					if (msg->mtype == WAKEUP_10) {
						release_memory_block(msg);
						break;
					} else {
						// Bad practice warning:
						// Both MEM_BLK and MSG_BUF have a mp_next pointer as the first field so it works
						// But technically with this cast, the data fields are misaligned
						q_insert((MEM_BLK**) &localQ, (MEM_BLK*) msg);
					}
				}
				// END OF HIBERNATE
			}
		}

		release_memory_block(msg);
	}

/*
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
}

// Registers to 'C'
// Command: %C <process_id> <new_priority>
void setPrioProc(void) {
	registerCommand('C');
	
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

char invalidCommandMsg[] = "%W_ is not a valid clock command!\r\n";

#ifdef SIM_TARGET
char timeStr[] = "00:00:00";
#define TIMESTR_OFFSET 0
#else
char timeStr[] = "\x1B[s\x1B[;72f00:00:00\x1B[u";
#define TIMESTR_OFFSET 9
#endif

void clockProc(void) {
	registerCommand('W');

	MSG_BUF* msg;
	char* charPtr;

	int time; // seconds since 00:00:00

	int isClockActive = FALSE;

	while (1) {
		msg = receive_message(NULL);
		charPtr = msg->mtext;
		// printf("Message: '%s' of type %d\n", msg->mtext, msg->mtype);

		if (msg->mtype == KCD_CMD) {
			char clockCommand = *(charPtr++);
			skipWhitespace(&charPtr);
			// Terminate - terminates wall clock
			if (clockCommand == 'T') {
				if (*charPtr == '\0') {
					// Terminate clock
					isClockActive = FALSE;	
				} else {
					// error, was not expecting anything
					sendUARTMsg("%WT clock command expects no arguments\r\n");
				}
			// Reset - Set clock to 00:00:00
			} else if (clockCommand == 'R') {
				if (*charPtr == '\0') {
					time = 0; // :)
					
					// Clock Tick
					if (isClockActive == FALSE) {
						isClockActive = TRUE;
						msg->mtype = CLOCK_TICK;
						delayed_send(PID_CLOCK, msg, CLOCK_DELAY);
						continue;
					}
				} else {
					// error, was not expecting any parameters
					sendUARTMsg("%WS clock command expects no arguments\r\n");
				}
			}
			// Set hh:mm:ss - sets current wall clock time to hh:mm:ss
			else if (clockCommand == 'S') {
				if (*charPtr == '\0') {
					// was supposed to have stuff
				} else if (
					!(
						IS_NUM(charPtr[0]) &&
						IS_NUM(charPtr[1]) &&
						charPtr[2] == ':'  &&
						IS_NUM(charPtr[3]) &&
						IS_NUM(charPtr[4]) &&
						charPtr[5] == ':'  &&
						IS_NUM(charPtr[6]) &&
						IS_NUM(charPtr[7])
					))
				{
					sendUARTMsg("The provided time format was incorrect!\r\n");
					goto DONEZO;
				} else {
					int newTime = 0;
					// parse hh:mm:ss

					// get hours
					int parsedTime = stringToNum(charPtr, 2);
					if (parsedTime > 23) {
						sendUARTMsg("Provided an invalid hours parameter - Hours must be less than 24 \r\n");
						goto DONEZO;
					}
					newTime += parsedTime*60*60;
					charPtr += 3;

					// get minutes
					parsedTime = stringToNum(charPtr, 2);
					if (parsedTime > 59 && parsedTime) {
						sendUARTMsg("Provided an invalid minutes parameter - Minutes must be less than 60 \r\n");
						goto DONEZO;
					}
					newTime += parsedTime*60;
					charPtr += 3;

					// get seconds
					parsedTime = stringToNum(charPtr, 2);
					if (parsedTime > 59) {
						sendUARTMsg("Provided an invalid seconds parameter - Seconds must be less than 60 \r\n");
						goto DONEZO;
					}
					newTime += parsedTime;
					charPtr += 2;

					skipWhitespace(&charPtr);
					if (*charPtr != '\0') {
						sendUARTMsg("No arguments should follow time in the WS command\r\n");
						goto DONEZO;
					}

					time = newTime;

					// now that we have hour, minute, second
					if (isClockActive == FALSE) {
						isClockActive = TRUE;
						msg->mtype = CLOCK_TICK;
						delayed_send(PID_CLOCK, msg, CLOCK_DELAY);
						continue;
					}
				}
			} else {
				invalidCommandMsg[2] = clockCommand;
				sendUARTMsg(invalidCommandMsg);
			}
		} else if (msg->mtype == CLOCK_TICK) {
			if (isClockActive == FALSE) {
				release_memory_block(msg);
			} else {
				time++;
				time %= 24*60*60;

				// char timeStr[8];
				// sprintf(timeStr, "%02d:%02d:%02d", (time/60/60) % 24, (time/60) % 60, time % 60);

				int tempTime = time;
				timeStr[7+TIMESTR_OFFSET] = '0' + ((tempTime % 60) % 10);
				timeStr[6+TIMESTR_OFFSET] = '0' + ((tempTime % 60) / 10);
				tempTime /= 60;
				timeStr[4+TIMESTR_OFFSET] = '0' + ((tempTime % 60) % 10);
				timeStr[3+TIMESTR_OFFSET] = '0' + ((tempTime % 60) / 10);
				tempTime /= 60;
				timeStr[1+TIMESTR_OFFSET] = '0' + ((tempTime % 24) % 10);
				timeStr[0+TIMESTR_OFFSET] = '0' + ((tempTime % 24) / 10);

#ifdef SIM_TARGET
				// The ANSI escape sequence magicks to keep the clock in the top right corner doesn't work in keil uvision :'(
				uart1_put_string(timeStr);
				uart1_put_string("\n");
#else
				sendUARTMsg(timeStr);
#endif

				delayed_send(PID_CLOCK, msg, CLOCK_DELAY);
				continue;
			}
		}

DONEZO:
		release_memory_block(msg);
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
			pUart->IER |= IER_THRE; // turn the IER_THRE bit on
		}
		else {
			release_memory_block(msg);
		}

		__set_CONTROL(1);
	}	
}
