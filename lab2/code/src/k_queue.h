#ifndef Q_H
#define Q_H

#include "k_inc.h"

void q_insert(MEM_BLK**, MEM_BLK *);
MEM_BLK * q_remove(MEM_BLK**);
MEM_BLK * q_remove_by_addr(MEM_BLK **, void *);

#endif
