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

void numToString(int num, char* buf) {
    int i = 0;
    
    if (num == 0) {
        buf[i++] = '0';
    }

    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    buf[i--] = '\0';
    
    // Reverse the order of the numbers in the buffer
    int j = 0;
    char temp;
    while(j < i) {
        temp = buf[j];
        buf[j] = buf[i];
        buf[i] = temp;
        j++;
        i--;
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

    while (i --> 0) { // lolol https://stackoverflow.com/questions/1642028/what-is-the-operator-in-c-c
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

void strncpy(char* d, char* s, int len) {
    int i;
    for (i = 0; s[i] != '\0' && i < len; i++) {
        d[i] = s[i];
    }
    if (i == len) {
        d[len-1] = '\0';
    } else {
        d[i] = '\0';
    }
}

void printStringEscaped(char* s, int limit) {
    for (int i = 0; s[i] && i < limit; i++) {
        switch (s[i]) {
            case '\b':
                uart1_put_string("\\b");
                break;
            case '\r':
                uart1_put_string("\\r");
                break;
            case '\n':
                uart1_put_string("\\n");
                break;
            default:
                uart1_put_char(s[i]);
        }
    }
}

void uart0_printMsgText(MSG_BUF* msg) {
    for (int i = 0; msg->mtext[i] && i < 50; i++) {
        uart0_put_char(msg->mtext[i]);
    }
}

int stringToNumAndCount(char* numberStr) {
    int numDigits = 0;
    for (char* charPtr = numberStr; *charPtr >= '0' && *charPtr <= '9'; charPtr++) {
        numDigits++;
    }
    return stringToNum(numberStr, numDigits);
}

int stringToNum(char* numberStr, int numDigits) {
    int result = 0;

    for (int i = 0; i < numDigits; i++) {
        int digit = *numberStr - (int) '0';
        result = result*10 + digit;
        numberStr++;
    }
    
    return result;
}

void skipWhitespace(char** ptr) {
    for (;**ptr == ' ' || **ptr == '\t' || **ptr == '\n' || **ptr == '\r'; (*ptr)++);
}

void sendUARTMsg(char* errText) {
    MSG_BUF* error_msg = (MSG_BUF*) request_memory_block();
    error_msg->mtype = CRT_DISPLAY;
    strcpy(error_msg->mtext, errText);
    send_message(PID_CRT, error_msg);
}
