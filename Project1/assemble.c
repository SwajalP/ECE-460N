#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "assemble.h"

#define MAX_LINE_LENGTH 255
enum { DONE, OK, EMPTY_LINE };

int toNum(char *pStr) {
    char *t_ptr;
    char *orig_pStr;
    int t_length, k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if (*pStr == '#') { 
        pStr++;
        if (*pStr == '-') {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++) {
            if (!isdigit(*t_ptr)) {
                printf("Error: invalid decimal operand, %s\n", orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg) lNum = -lNum;
        return lNum;
    } else if (*pStr == 'x') { 
        pStr++;
        if (*pStr == '-') {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++) {
            if (!isxdigit(*t_ptr)) {
                printf("Error: invalid hex operand, %s\n", orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16);
        lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong;
        if (lNeg) lNum = -lNum;
        return lNum;
    } else {
        printf("Error: invalid operand, %s\n", orig_pStr);
        exit(4);
    }
}

int isOpcode(char *str) {
    const char *opcodes[] = {
        "add", "and", "br", "brn", "brz", "brp", "brzp", "brnp", "brnz",
        "brnzp", "jmp", "jsr", "jsrr", "ld", "ldi", "ldr", "lea", "not",
        "ret", "rti", "st", "sti", "str", "trap",
        ".orig", ".end", ".fill", ".blkw", ".stringz"
    };

    const int number_of_opcodes = sizeof(opcodes) / sizeof(opcodes[0]);
    for (int i = 0; i < number_of_opcodes; i++) {
        if (strcmp(str, opcodes[i]) == 0) {
            return i;
        }
    }
    return -1;
}

int readAndParse(FILE *pInfile, char *pLine,
                 char **pLabel, char **pOpcode,
                 char **pArg1, char **pArg2,
                 char **pArg3, char **pArg4) {
    char *lPtr;
    int i;
    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))
        return DONE;

    for (i = 0; i < strlen(pLine); i++)
        pLine[i] = tolower(pLine[i]);

    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    lPtr = pLine;
    while (*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n')
        lPtr++;
    *lPtr = '\0';

    if (!(lPtr = strtok(pLine, "\t\n ,")))
        return EMPTY_LINE;

    if (isOpcode(lPtr) == -1 && lPtr[0] != '.') {
        *pLabel = lPtr;
        if (!(lPtr = strtok(NULL, "\t\n ,")))
            return OK;
    }

    *pOpcode = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,"))) return OK;
    *pArg1 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,"))) return OK;
    *pArg2 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,"))) return OK;
    *pArg3 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,"))) return OK;
    *pArg4 = lPtr;

    return OK;
}

typedef struct {
    char label[MAX_LINE_LENGTH];
    int address;
} Symbol;

Symbol symbolTable[255];
int symbolCount = 0;

int addSymbol(char *label, int address) {
    if (label == NULL) return 0;
    if (symbolCount > 254) {
        printf("Error: symbol table overflow\n");
        exit(4);
    }

    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].label, label) == 0) {
            printf("Error: duplicate label %s\n", label);
            exit(4);
        }
    }

    strcpy(symbolTable[symbolCount].label, label);
    symbolTable[symbolCount].address = address;
    symbolCount++;
    return 1;
}


int findSymbol(char *label) {
    for(int i = 0; i < symbolCount; i++){
        if (strcmp(symbolTable[i].label, label) == 0){
            return symbolTable[i].address;
        }
    }
    return -1;
}

void firstPass(FILE *infile) {
    char line[MAX_LINE_LENGTH + 1];
    char *label, *opcode, *arg1, *arg2, *arg3, *arg4;
    int ret; 
    int PC = 0;

    while(1){
         ret = readAndParse(infile, line, &label, &opcode, &arg1, &arg2, &arg3, &arg4);
        if(ret == DONE) break;
        if(ret == EMPTY_LINE) continue;

        if(strcmp(opcode, ".orig") == 0){
            PC = toNum(arg1);
            continue;
        }

        if(strcmp(opcode, ".end") == 0){
            break;
        }

        if(label[0] != '\0'){
            addSymbol(label, PC);
        }

        if (strcmp(opcode, ".blkw") == 0) {
            PC += toNum(arg1);
        } else if (strcmp(opcode, ".stringz") == 0) {
            PC += (int)strlen(arg1) + 1;  
        } else {
            PC++;
        }
    }
}

void secondPass(FILE *infile, FILE *outfile) {

}

int assembleInstruction(char *opcode, char *arg1, char *arg2,
                        char *arg3, char *arg4, int location) {
        if (strcmp(opcode, "\0") == 0 || opcode == NULL){
            exit(4);
        }
    
}

int getRegister(char *regStr) {
    if (regStr == NULL) return -1;

    if ((regStr[0] == 'R' || regStr[0] == 'r') && isdigit(regStr[1])) {
        int regNum = regStr[1] - '0';
        if (regNum >= 0 && regNum < 8)
            return regNum;
    }

    printf("Error: invalid register %s\n", regStr);
    exit(4); 
}

int getImmediate(char *immStr, int bitCount) {
    int val = toNum(immStr);
    int minVal = -(1 << (bitCount - 1));
    int maxVal =  (1 << (bitCount - 1)) - 1;

    if(val < minVal || val > maxVal){
        printf("Error: immediate value %d out of range for %d-bit field\n", val, bitCount);
        exit(4);
    }
    return val & ((1 << bitCount) - 1);
}

int main(int argc, char *argv[]) {
    FILE *infile = NULL, *outfile = NULL;

    if (argc < 3) {
        printf("Usage: %s <input file> <output file>\n", argv[0]);
        exit(4);
    }

    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");
    if (!infile) {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
    }
    if (!outfile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }

    firstPass(infile);
    rewind(infile);
    secondPass(infile, outfile);

    fclose(infile);
    fclose(outfile);
    return 0;
}