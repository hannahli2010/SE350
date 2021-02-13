/**************************************************************************//**
 * @file        ae_util.c
 * @brief       Helper function to print test information
 *              
 * @version     V1.2021.01
 * @authors     Group 10
 * @date        2021 FEB
 * @note        
 * @details
 *
 *****************************************************************************/

#include "rtx.h"
#include "uart_polling.h"
#include "ae_util.h"

void printUart0(char * testName, char * test) {
  uart0_put_string(testName);
  uart0_put_string(": ");
  uart0_put_string(test);
  uart0_put_string("\n\r");
}

int assertTest(char * testName, int returned, int expected, char* testNum) {
  uart0_put_string(testName);
  uart0_put_string(": test ");
  uart0_put_string(testNum);
  if (returned == expected) {
    uart0_put_string(" OK\n\r");
    return 1;
  } else {
    uart0_put_string(" FAIL\n\r");
    return 0;
  }
}

void printFraction(int num, int denom) {
  uart0_put_char('0' + num);
  uart0_put_char('/');
  uart0_put_char('0' + denom);
}

void printSummary(char * testName, int numSucceeded, int numTests) {
    uart0_put_string(testName);
    uart0_put_string(": ");
    printFraction(numSucceeded, numTests);
    uart0_put_string(" tests OK\n\r");
    
    uart0_put_string(testName);
    uart0_put_string(": ");
    printFraction(numTests - numSucceeded, numTests);
    uart0_put_string(" tests FAIL\n\r");

    printUart0(testName, "END");
}
