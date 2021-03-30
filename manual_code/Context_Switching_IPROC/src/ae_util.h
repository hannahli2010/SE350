 /**************************************************************************//**
 * @file        ae_util.h
 * @brief       ae test utils header file
 *              
 * @version     V1.2021.01
 * @authors     Group 10
 * @date        2021 JAN
 *****************************************************************************/

#ifndef AE_UTIL
#define AE_UTIL

void printUart1(char *, char *);

int assertTest(char *, int, int, char*);

void printNumber(int);

void printFraction(int, int);

void printSummary(char *, int, int);

void strcpy(char*, char*);

void uart0_printMsgText(MSG_BUF*);

int stringToNum(char*, int);

void skipWhitespace(char**);

void sendUARTMsg(char*);

#endif
