#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "assemble.h"
#include <cstdint>

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
    bool startedCode = false;

    while(1){
         ret = readAndParse(infile, line, &label, &opcode, &arg1, &arg2, &arg3, &arg4);
        if(ret == DONE) break;
        if(ret == EMPTY_LINE) continue;

        if(strcmp(opcode, ".orig") == 0){
            PC = toNum(arg1);
            startedCode = true;
            continue;
        }

        if(strcmp(opcode, ".end") == 0){
            break;
        }

        if(label[0] != '\0' && startedCode){
            addSymbol(label, PC);
        }


        // if (strcmp(opcode, ".blkw") == 0) {
        //     PC += toNum(arg1);
        // } else if (strcmp(opcode, ".stringz") == 0) {
        //     PC += (int)strlen(arg1) + 1;  
        // } else {
        //     PC++;
        // }
    }
}

void secondPass(FILE *infile, FILE *outfile) {
    bool startedCode = false;
    char line[MAX_LINE_LENGTH + 1];
    char *label, *opcode, *arg1, *arg2, *arg3, *arg4;
    int ret; 
     while(1){
         ret = readAndParse(infile, line, &label, &opcode, &arg1, &arg2, &arg3, &arg4);
        if(ret == DONE) break;
        if(ret == EMPTY_LINE) continue;

        if(strcmp(opcode, ".orig") == 0){
            startedCode = true;
            continue;
        }

        if(strcmp(opcode, ".end") == 0){
            break;
        }

        if(startedCode && opcode != NULL){
            uint16_t address1 = findSymbol(arg1);
            uint16_t address2 = findSymbol(arg2);
            uint16_t address3 = findSymbol(arg3);
            uint16_t address4 = findSymbol(arg4);

            char placeholderNum1[16];
            char placeholderNum2[16];
            char placeholderNum3[16];
            char placeholderNum4[16];

            //Filling in the labels with addresses if we could find one
            if(address1 != -1){
                sprintf(placeholderNum1, "%d", address1);
                arg1 = placeholderNum1;
            }
            if(address2 != -1){
                sprintf(placeholderNum2, "%d", address2);
                arg2 = placeholderNum2;
            }
            if(address3 != -1){
                sprintf(placeholderNum3, "%d", address3);
                arg3 = placeholderNum3;
            }
            if(address4 != -1){
                sprintf(placeholderNum4, "%d", address4);
                arg4 = placeholderNum4;
            }

            //Calling all the opcodes now
            uint16_t printedCode = 0;

            if(strcmp(opcode, "add") == 0) {
                printedCode = addandxorInstruction(0x1, arg1, arg2, arg3);
            } else if(strcmp(opcode, "and") == 0) {
                printedCode = addandxorInstruction(0x1, arg1, arg2, arg3);
            } else if(strstr(opcode, "br") == 0) {
                
            } else if(strcmp(opcode, "jmp") == 0) {

            } else if(strcmp(opcode, "jsr") == 0) {

            } else if(strcmp(opcode, "jsrr") == 0) {

            } else if(strcmp(opcode, "ldb") == 0) {

            } else if(strcmp(opcode, "ldw") == 0) {

            } else if(strcmp(opcode, "lea") == 0) {

            } else if(strcmp(opcode, "not") == 0) {

            } else if(strcmp(opcode, "ret") == 0) {

            } else if(strcmp(opcode, "rti") == 0) {

            } else if(strcmp(opcode, "lshf") == 0) {

            } else if(strcmp(opcode, "rshfl") == 0) {

            } else if(strcmp(opcode, "rshfa") == 0) {

            } else if(strcmp(opcode, "stb") == 0) {

            } else if(strcmp(opcode, "stw") == 0) {

            } else if(strcmp(opcode, "trap") == 0) {

            } else if(strcmp(opcode, "xor") == 0) {

            } else {
                // invalid opcode
                //Delete this later
                printf("Invalid Opcode!");
                exit(4);
            }

        }
    }

}

    uint16_t addandxorInstruction(int opcode, char* arg1, char* arg2, char* arg3){
         uint16_t num = 0;
         num += opcode << 12;
         num += toNum(++arg1) << 9;
         num += toNum(++arg2) << 6;

         if(arg3[0] == 'r'){
            num += toNum(++arg3);
         }else{
            num += 1 << 5;
            num += toNum(arg3);
         }
      }

    uint16_t jmpjsrrInstruction(int opcode, char* arg1, char* arg2, char* arg3){
        uint16_t num = 0;
        num += opcode << 12;
        num += toNum(++arg1) << 6;
    }

    uint16_t jsrInstruction(int opcode, char* arg1){
        uint16_t num = 0;
        num += opcode << 12;
        num += 1 << 11;
        num += toNum(arg1);
    }

    uint16_t ldbldwstbstwInstruction(int opcode, char* arg1, char* arg2, char* arg3){
         uint16_t num = 0;
         num += opcode << 12;
         num += toNum(++arg1) << 9;
         num += toNum(++arg2) << 6;
         num += toNum(arg3);
      }

      uint16_t leaInstruction(int opcode, char* arg1, char* arg2){
         uint16_t num = 0;
         num += opcode << 12;
         num += toNum(++arg1) << 9;
         num += toNum(arg2);
      }

      uint16_t notInstruction(int opcode, char* arg1, char* arg2){
         uint16_t num = 0;
         num += opcode << 12;
         num += toNum(++arg1) << 9;
         num += toNum(++arg2) << 6;
         num += 1 << 5;
         num += 0x1F;
      }

      uint16_t retrtiInstruction(int opcode, char* arg1, char* arg2){
         uint16_t num = 0;
         num += opcode << 12;
         if(opcode == 0xC){
            num += 0x7 << 6;
         }
      }

      uint16_t lshrshIntruction(int opcode, char* arg1, char* arg2, char* arg3, int idBits){
         uint16_t num = 0;
         num += opcode << 12;
         num += toNum(++arg1) << 9;
         num += toNum(++arg2) << 6;
         num += idBits << 4;
         num += toNum(arg3);
      }

      uint16_t trapInstruction(int opcode, char* arg1){
        uint16_t num = 0;
        num += opcode << 12;
        num += toNum(arg1);
      }

      uint16_t brInstruction(int opcode, char* arg1, char* conditionBits){
        uint16_t num = 0;
        int bits = ((strchr(conditionBits, 'n') ? 1 : 0) << 2) + ((strchr(conditionBits, 'z') ? 1 : 0) << 1) + ((strchr(conditionBits, 'p') ? 1 : 0) << 0);
        num += opcode << 12;
        num += bits << 9;
        num += toNum(arg1);
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