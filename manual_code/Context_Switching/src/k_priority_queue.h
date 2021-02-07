#ifndef PQ_H
#define PQ_H

#include "k_inc.h"

void pq_insert(PCB** list, PCB * proc);

PCB * pq_remove(PCB** list);

PCB * pq_remove_by_pid(PCB** list, int pid);

void pq_print(PCB** list);

#endif
