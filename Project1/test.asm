;===================================================
; Demo program using all LC-3b instructions
;===================================================
        .ORIG x3000

; --- Arithmetic and Logic ---
        ADD   R1, R2, R3        ; R1 = R2 + R3
        ADD   R4, R1, #5        ; R4 = R1 + 5
        AND   R5, R4, R3        ; R5 = R4 & R3
        AND   R6, R5, #7        ; R6 = R5 & 7
        XOR   R7, R6, R1        ; R7 = R6 ^ R1
        XOR   R0, R7, #15       ; R0 = R7 ^ 15
        NOT   R0, R0            ; R0 = ~R0

; --- Shifts ---
        LSHF  R1, R1, #1        ; R1 = R1 << 1
        RSHFL R2, R2, #1        ; R2 = logical right shift
        RSHFA R3, R3, #1        ; R3 = arithmetic right shift

; --- Branch / Jumps ---
        BRnzp SKIP               ; unconditional branch
        ADD   R0, R0, #1        ; skipped
SKIP    JMP   R1                ; jump to address in R1 (just demo)
        JSR   SUBROUTINE        ; jump to subroutine using PC offset
        JSRR  R4                ; jump to subroutine using register
        RET                     ; return from subroutine
        RTI                     ; return from interrupt (special case)

; --- Loads and Stores ---
        LDB   R1, R2, #2        ; load byte from mem[R2 + 2]
        LDW   R2, R3, #4        ; load word from mem[R3 + 4]
        LEA   R3, LABEL         ; R3 = address of LABEL
        STB   R4, R5, #3        ; store byte at mem[R5 + 3]
        STW   R6, R7, #1        ; store word at mem[R7 + 1]

; --- Trap ---
        TRAP  x21               ; output a character (PUTS etc.)

; --- Subroutine example ---
SUBROUTINE
        ADD   R0, R0, #1
        RET


        .END
