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

void printUart1(char * testName, char * test) {
    uart1_put_string(testName);
    uart1_put_string(": ");
    uart1_put_string(test);
    uart1_put_string("\n\r");
}

int assertTest(char * testName, int returned, int expected, char* testNum) {
    uart1_put_string(testName);
    uart1_put_string(": test ");
    uart1_put_string(testNum);
    if (returned == expected) {
        uart1_put_string(" OK\n\r");
        return 1;
    } else {
        uart1_put_string(" FAIL\n\r");
        return 0;
    }
}

void printNumber(int num) {
    char str[12];
    int i = 0;
    
    if (num == 0) {
        str[i++] = '0';
    }

    while (num > 0) {
        str[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i --> 0) { // lolol
        uart1_put_char(str[i]);
    }
}

void printFraction(int num, int denom) {
    printNumber(num);
    uart1_put_char('/');
    printNumber(denom);
}

void printSummary(char * testName, int numSucceeded, int numTests) {
    uart1_put_string(testName);
    uart1_put_string(": ");
    printFraction(numSucceeded, numTests);
    uart1_put_string(" tests OK\n\r");
    
    uart1_put_string(testName);
    uart1_put_string(": ");
    printFraction(numTests - numSucceeded, numTests);
    uart1_put_string(" tests FAIL\n\r");

    printUart1(testName, "END");
}

void strcpy(char* d, char* s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) {
        d[i] = s[i];
    }
    d[i] = '\0';
}

void uart0_printMsgText(MSG_BUF* msg) {
    for (int i = 0; msg->mtext[i] && i < 50; i++) {
        uart0_put_char(msg->mtext[i]);
    }
}
