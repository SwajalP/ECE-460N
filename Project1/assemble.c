#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 255
enum { DONE, OK, EMPTY_LINE };


#define OK 0
#define EMPTY_LINE 1
#define DONE 2

int addandxorInstruction(int opcode, char* arg1, char* arg2, char* arg3);
int jmpjsrrInstruction(int opcode, char* arg1, char* arg2, char* arg3);
int jsrInstruction(int opcode, char* arg1, int currAddr);
int ldbldwstbstwInstruction(int opcode, char* arg1, char* arg2, char* arg3);
int leaInstruction(int opcode, char* arg1, char* arg2, int currAddr);
int notInstruction(int opcode, char* arg1, char* arg2);
int retrtiInstruction(int opcode, char* arg1, char* arg2);
int lshrshInstruction(int opcode, char* arg1, char* arg2, char* arg3, int idBits);
int trapInstruction(int opcode, char* arg1);
int brInstruction(int opcode, char* arg1, char* conditionBits, int currAddr);
int getRegister(char *regStr);
int getImmediate(char *arg, int currAddr, const char *opcode);

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
        "brnzp", "jmp", "jsr", "jsrr", "lea", "not",
        "ret", "rti", "trap", "xor", "lshf", "rshfl", "rshfa",
        "ldb", "ldw", "stb", "stw", "nop", "halt",
        ".orig", ".end", ".fill"
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
int PC = 0; 

int addSymbol(char *label, int address) {
    if (label == NULL || strlen(label) == 0) return 0;
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

void firstPass(FILE *infile, FILE *outfile) {
    char line[MAX_LINE_LENGTH + 1];
    char *label, *opcode, *arg1, *arg2, *arg3, *arg4;
    int ret; 
    bool startedCode = false;

    while(1){
        ret = readAndParse(infile, line, &label, &opcode, &arg1, &arg2, &arg3, &arg4);
        if(ret == DONE) break;
        if(ret == EMPTY_LINE) continue;

        if(strcmp(opcode, ".orig") == 0){
            PC = toNum(arg1);
            fprintf(outfile, "0x%04X\n", PC);
            startedCode = true;
            continue;
        }

        if(strcmp(opcode, ".end") == 0){
            break;
        }

        if(startedCode){
            if(strlen(label) > 0){
                addSymbol(label, PC);
            }
            PC += 2;
        }
    }
}

void secondPass(FILE *infile, FILE *outfile) {
    bool startedCode = false;
    char line[MAX_LINE_LENGTH + 1];
    char *label, *opcode, *arg1, *arg2, *arg3, *arg4;
    int ret;
    PC = 0;  
    
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

        if(startedCode && strlen(opcode) > 0){
            uint16_t printedCode = 0;

            if(strcmp(opcode, "add") == 0) {
                printedCode = addandxorInstruction(0x1, arg1, arg2, arg3);
            } else if(strcmp(opcode, "and") == 0) {
                printedCode = addandxorInstruction(0x5, arg1, arg2, arg3);
            } else if(strcmp(opcode, "br") == 0 || strcmp(opcode, "brnzp") == 0) {
                printedCode = brInstruction(0x0, arg1, "nzp", PC);
            } else if(strcmp(opcode, "brn") == 0) {
                printedCode = brInstruction(0x0, arg1, "n", PC);
            } else if(strcmp(opcode, "brz") == 0) {
                printedCode = brInstruction(0x0, arg1, "z", PC);
            } else if(strcmp(opcode, "brp") == 0) {
                printedCode = brInstruction(0x0, arg1, "p", PC);
            } else if(strcmp(opcode, "brnz") == 0) {
                printedCode = brInstruction(0x0, arg1, "nz", PC);
            } else if(strcmp(opcode, "brnp") == 0) {
                printedCode = brInstruction(0x0, arg1, "np", PC);
            } else if(strcmp(opcode, "brzp") == 0) {
                printedCode = brInstruction(0x0, arg1, "zp", PC);
            } else if(strcmp(opcode, "jmp") == 0) {
                printedCode = jmpjsrrInstruction(0xC, arg1, arg2, arg3);
            } else if(strcmp(opcode, "jsr") == 0) {
                printedCode = jsrInstruction(0x4, arg1, PC);
            } else if(strcmp(opcode, "jsrr") == 0) {
                printedCode = jmpjsrrInstruction(0x4, arg1, arg2, arg3);
            } else if(strcmp(opcode, "ldb") == 0) {
                printedCode = ldbldwstbstwInstruction(0x2, arg1, arg2, arg3);
            } else if(strcmp(opcode, "ldw") == 0) {
                printedCode = ldbldwstbstwInstruction(0x6, arg1, arg2, arg3);
            } else if(strcmp(opcode, "lea") == 0) {
                printedCode = leaInstruction(0xE, arg1, arg2, PC);
            } else if(strcmp(opcode, "not") == 0) {
                printedCode = notInstruction(0x9, arg1, arg2);
            } else if(strcmp(opcode, "ret") == 0) {
                printedCode = retrtiInstruction(0xC, arg1, arg2);
            } else if(strcmp(opcode, "rti") == 0) {
                printedCode = retrtiInstruction(0x8, arg1, arg2);
            } else if(strcmp(opcode, "lshf") == 0) {
                printedCode = lshrshInstruction(0xD, arg1, arg2, arg3, 0x0);
            } else if(strcmp(opcode, "rshfl") == 0) {
                printedCode = lshrshInstruction(0xD, arg1, arg2, arg3, 0x1);
            } else if(strcmp(opcode, "rshfa") == 0) {
                printedCode = lshrshInstruction(0xD, arg1, arg2, arg3, 0x3);
            } else if(strcmp(opcode, "stb") == 0) {
                printedCode = ldbldwstbstwInstruction(0x3, arg1, arg2, arg3);
            } else if(strcmp(opcode, "stw") == 0) {
                printedCode = ldbldwstbstwInstruction(0x7, arg1, arg2, arg3);
            } else if(strcmp(opcode, "trap") == 0) {
                int trapVec = toNum(arg1);
                if(trapVec < 0 || trapVec > 0xFF) {
                    printf("Error: TRAP vector out of range: %d\n", trapVec);
                    exit(4);
                }
                printedCode = trapInstruction(0xF, arg1);
            } else if(strcmp(opcode, "halt") == 0) {
                printedCode = 0xF025;  // TRAP x25
            } else if(strcmp(opcode, "xor") == 0) {
    printedCode = addandxorInstruction(0x9, arg1, arg2, arg3);
} else if (strcmp(opcode, "nop") == 0) {
                printedCode = 0x0000;
            } else if(strcmp(opcode, ".fill") == 0){
                printedCode = toNum(arg1) & 0xFFFF;
            }

            fprintf(outfile, "0x%04X\n", printedCode);
            PC += 2;
        }
    }
}


int addandxorInstruction(int opcode, char* arg1, char* arg2, char* arg3){
    int num = 0;
    num |= (opcode & 0xF) << 12;     
    num |= (getRegister(arg1) & 0x7) << 9; 
    num |= (getRegister(arg2) & 0x7) << 6;

    if(arg3[0] == 'r' || arg3[0] == 'R'){ 
        num |= (getRegister(arg3) & 0x7); 
    } else {                                 
        num |= 1 << 5;                     
        num |= (toNum(arg3) & 0x1F);    
    }
    return num;
}

int jmpjsrrInstruction(int opcode, char* arg1, char* arg2, char* arg3){
    int num = 0;
    num += opcode << 12;
    
    if(opcode == 0xC) {
        num += getRegister(arg1) << 6;
    } else {
        num += getRegister(arg1) << 6;
    }
    
    return num;
}

int jsrInstruction(int opcode, char* arg1, int currAddr){
    int num = 0;
    num += opcode << 12;
    num += 1 << 11;

    if(arg1[0] == '#' || arg1[0] == 'x') {
        num += toNum(arg1) & 0x7FF;
    } else {

        int symAddr = findSymbol(arg1);
        if(symAddr == -1) {
            printf("Error: undefined symbol '%s'\n", arg1);
            exit(4);
        }
        int offset = (symAddr - (currAddr + 2)) / 2;
        if(offset < -1024 || offset > 1023) {
            printf("Error: JSR offset out of range\n");
            exit(4);
        }
        num += offset & 0x7FF;
    }
    
    return num;
}

int ldbldwstbstwInstruction(int opcode, char* arg1, char* arg2, char* arg3){
    int num = 0;
    num += opcode << 12;
    
    if(opcode == 0x3 || opcode == 0x7) {

        num += getRegister(arg1) << 9;
    } else {

        num += getRegister(arg1) << 9;
    }
    
    num += getRegister(arg2) << 6;
    

        int offset6 = toNum(arg3);
    if (offset6 < -32 || offset6 > 31) { printf("Error: offset6 out of range\n"); exit(4); }
    num |= offset6 & 0x3F;

    
    return num;
}

int leaInstruction(int opcode, char* arg1, char* arg2, int currAddr){
    int num = 0;
    num += opcode << 12;
    num += getRegister(arg1) << 9;
    

    if(arg2[0] == '#' || arg2[0] == 'x') {
        num += toNum(arg2) & 0x1FF;
    } else {

        int symAddr = findSymbol(arg2);
        if(symAddr == -1) {
            printf("Error: undefined symbol '%s'\n", arg2);
            exit(4);
        }

        int offset = (symAddr - (currAddr + 2)) / 2;

        if(offset < -256 || offset > 255) {
            printf("Error: LEA offset out of range\n");
            exit(4);
        }
        num += offset & 0x1FF;
    }
    
    return num;
}

int notInstruction(int opcode, char* arg1, char* arg2){
    int num = 0;
    num += opcode << 12;
    num += getRegister(arg1) << 9;
    num += getRegister(arg2) << 6;
    num += 0x3F; 
    return num;
}

int retrtiInstruction(int opcode, char* arg1, char* arg2){
    int num = 0;
    num += opcode << 12;
    if(opcode == 0xC) num += 0x7 << 6; 
    return num;
}

int lshrshInstruction(int opcode, char* arg1, char* arg2, char* arg3, int idBits){
    int num = 0;
    num |= (opcode & 0xF) << 12;  
    num |= (getRegister(arg1) & 0x7) << 9; 
    num |= (getRegister(arg2) & 0x7) << 6;
    num |= (idBits & 0x3) << 4;        
    num |= (toNum(arg3) & 0xF); 
    return num;
}

int trapInstruction(int opcode, char* arg1){
    int num = 0;
    num += opcode << 12;
    num += toNum(arg1) & 0xFF;
    return num;
}

int brInstruction(int opcode, char* arg1, char* conditionBits, int currAddr){
    int num = 0;
    int bits = ((strchr(conditionBits, 'n') ? 1 : 0) << 2) +
               ((strchr(conditionBits, 'z') ? 1 : 0) << 1) +
               ((strchr(conditionBits, 'p') ? 1 : 0) << 0);
    num += 0x0 << 12;
    num += bits << 9;

    if(arg1[0] == '#' || arg1[0] == 'x') {
        num += toNum(arg1) & 0x1FF;
    } else {

        int symAddr = findSymbol(arg1);
        if(symAddr == -1) {
            printf("Error: undefined symbol '%s'\n", arg1);
            exit(4);
        }
        int offset = (symAddr - (currAddr + 2)) / 2;

        if(offset < -256 || offset > 255) {
            printf("Error: BR offset out of range\n");
            exit(4);
        }
        num += offset & 0x1FF;
    }
    
    return num;
}

int getRegister(char *reg) {
    if(reg[0] != 'r' && reg[0] != 'R') {
        printf("Error: invalid register %s\n", reg);
        exit(4);
    }
    int r = atoi(reg + 1);
    if(r < 0 || r > 7) {
        printf("Error: invalid register %s\n", reg);
        exit(4);
    }
    return r;

}

int main(int argc, char* argv[]) {
    if(argc != 3) {
        printf("Error: usage: %s <input file> <output file>\n", argv[0]);
        exit(4);
    }
    
    FILE *infile = fopen(argv[1], "r");
    FILE *outfile = fopen(argv[2], "w");
    
    if (!infile) {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
    }
    if (!outfile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }
    
    firstPass(infile, outfile);
    
    rewind(infile);
    
    secondPass(infile, outfile);
    
    fclose(infile);
    fclose(outfile);
    
    return 0;
}