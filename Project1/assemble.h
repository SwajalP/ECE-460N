#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>


#define MAX_LINE_LENGTH 255

enum { DONE, OK, EMPTY_LINE };

int toNum(char *pStr);
int readAndParse(FILE *pInfile, char *pLine,
                 char **pLabel, char **pOpcode,
                 char **pArg1, char **pArg2,
                 char **pArg3, char **pArg4);
int isOpcode(char *str);

void firstPass(FILE *infile, FILE *outfile);
void secondPass(FILE *infile, FILE *outfile);
int assembleInstruction(char *opcode, char *arg1, char *arg2,
                        char *arg3, char *arg4);
int getRegister(char *regStr);
uint16_t addandxorInstruction(int opcode, char* arg1, char* arg2, char* arg3);
uint16_t jmpjsrrInstruction(int opcode, char* arg1, char* arg2, char* arg3);
uint16_t jsrInstruction(int opcode, char* arg1);
uint16_t ldbldwstbstwInstruction(int opcode, char* arg1, char* arg2, char* arg3);
uint16_t leaInstruction(int opcode, char* arg1, char* arg2);
uint16_t notInstruction(int opcode, char* arg1, char* arg2);
uint16_t retrtiInstruction(int opcode, char* arg1, char* arg2);
uint16_t lshrshIntruction(int opcode, char* arg1, char* arg2, char* arg3, int idBits);
uint16_t trapInstruction(int opcode, char* arg1);
uint16_t brInstruction(int opcode, char* arg1, char* conditionBits);

#endif