#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_LINE_LENGTH 255

enum { DONE, OK, EMPTY_LINE };

int toNum(char *pStr);
int readAndParse(FILE *pInfile, char *pLine,
                 char **pLabel, char **pOpcode,
                 char **pArg1, char **pArg2,
                 char **pArg3, char **pArg4);
int isOpcode(char *str);

void firstPass(FILE *infile);
void secondPass(FILE *infile, FILE *outfile);
int assembleInstruction(char *opcode, char *arg1, char *arg2,
                        char *arg3, char *arg4);

#endif