#include "k_sys_procs.h"
#include "k_process.h"
#include "k_msg.h"
#include "rtx.h"
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

	// Insert all the messages we just recieved into the delayed message queue
	DELAYED_MSG_BUF* newMsg;
	while (newMsg = (DELAYED_MSG_BUF*) k_receive_message_nb(NULL) ) {
		// If the list is empty, insert at the front of the list
		if (delayed_msg_queue == NULL) {
			newMsg->mp_next = NULL;
			delayed_msg_queue = newMsg;
		}
		// If the message should be inserted before the first element in the 
		//   queue, insert the new message to the head of the list
		else if (newMsg->m_expiry <= delayed_msg_queue->m_expiry) {
			newMsg->mp_next = delayed_msg_queue;
			delayed_msg_queue = newMsg;
		}
		// Otherwise, insert the message to the correct placement in the 
		//   queue based on expiry order
		else {
			DELAYED_MSG_BUF* it = delayed_msg_queue;
			while (it->mp_next != NULL && newMsg->m_expiry <= it->mp_next->m_expiry) {
				it = it->mp_next;
				// Delays are relative to the delay of the message before it
				// Decrememt the message's expiry by the previous message's delay
				newMsg->m_expiry -= it->m_expiry;
			}

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

	// Terminate the interrupt - this is not needed because its done in ITIMER_RESTORE
	// k_release_processor();
}
