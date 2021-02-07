#include "nullProc.h"
#include "rtx.h"
#include "printf.h"


void nullProc(void)
{
    while (1) {
			printf("Null Proc try to release\n");
      for (int x = 0; x < 1000; x++); // some artifical delay
			release_processor();
    }
}
