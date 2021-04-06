#include "k_queue.h"
#include "printf.h"

// Add to back of queue
void q_insert(MEM_BLK** list, MEM_BLK* block) {
  
    // Insert the block at the back of the queue, so the next pointer is null
    block->mp_next = NULL;

    // If the list is empty, insert at the front of the list
    if (*list == NULL) {
        *list = block;
        return;
    }

    // Find the back of the queue
    MEM_BLK* it = *list;	
    while(it->mp_next != NULL) {
        it = it->mp_next;
    }
    // Insert at the back of the queue
    it->mp_next = block;
}

// Remove from front of queue
MEM_BLK * q_remove(MEM_BLK** list) {

    // If the list is empty, return NULL
    if (*list == NULL) {
        return NULL;
    }

    // Store the block temporarily to return it
    MEM_BLK* block = *list;
    
    // Set front of list to point to next block
    *list = (*list)->mp_next;

    // Clear the removed block's next pointer 
    block->mp_next = NULL;

    return block;
}

// Remove from the queue by a particular address
MEM_BLK * q_remove_by_addr(MEM_BLK **list, void * addr) {
    MEM_BLK* block_it = *list;
    
    // If the list is empty, return null
    if (block_it == NULL ) {
        return NULL;
    } 
    // If the list has one element, and it is the requested block, remove it by setting the list pointer
    else if (block_it->block == addr) {
        *list = block_it->mp_next;
        return block_it;
    }
    // Search the list for the memory block
    else {
        while(block_it->mp_next != NULL) {
            if (block_it->mp_next->block == addr) {
                MEM_BLK *temp = block_it->mp_next;
                block_it->mp_next = temp->mp_next;
                block_it = temp;
                return block_it;
            }
            block_it = block_it->mp_next;
        }
        
        // The memory block was not found
        return NULL;
    }
}
