#include "k_priority_queue.h"
#include "printf.h"

void pq_insert(PCB** list, PCB * proc) {
	PCB * it = *list;
	
	if (it == NULL) {
		*list = proc;
		proc->mp_next = NULL;
		return;
	} else if (it->m_priority > proc->m_priority) { // if we need to insert at the front of the queue
		proc->mp_next = *list;
		*list = proc;
		return;
	}
	
	while(it->mp_next && it->mp_next->m_priority <= proc->m_priority) {
		it = it->mp_next;
	}
	
	proc->mp_next = it->mp_next;
	it->mp_next = proc;
}

// insert to the front of the same priority
void pq_insert_front(PCB** list, PCB * proc) {
	PCB * it = *list;
	
	if (it == NULL) {
		*list = proc;
		proc->mp_next = NULL;
		return;
	} else if (it->m_priority >= proc->m_priority) { // if we need to insert at the front of the queue
		proc->mp_next = *list;
		*list = proc;
		return;
	}
	
	while(it->mp_next && it->mp_next->m_priority < proc->m_priority) {
		it = it->mp_next;
	}
	
	proc->mp_next = it->mp_next;
	it->mp_next = proc;
}

PCB * pq_remove(PCB** list) {
	PCB * proc = *list;
	
	if (proc == NULL) {
		return NULL;
	}
	
	*list = proc->mp_next;
	proc->mp_next = NULL;
	return proc;
}

PCB * pq_remove_by_pid(PCB** list, int pid) {
	PCB * it = *list;
	
	if (it == NULL) {
		return NULL;
	}
	
	if (it->m_pid == pid) {
		*list = it->mp_next;
		it->mp_next = NULL;
		return it;
	}
	
	while(it->mp_next && it->mp_next->m_pid != pid) {
		it = it->mp_next;
	}
	
	if (it->mp_next != NULL) {
		PCB * proc = it->mp_next;
		it->mp_next = proc->mp_next;
		proc->mp_next = NULL;
		return proc;
	}
	
	return NULL;
}

void pq_print(PCB** list) {
	printf("Print PQ\n");
	PCB * it = *list;
	
	if (it == NULL) {
		printf("Empty\n");
	}
	
	while(it != NULL) {
		printf("Pid: %d, priority: %d, next: 0x%x\n", it->m_pid, it->m_priority, it->mp_next);
		it = it->mp_next;
	}
}
