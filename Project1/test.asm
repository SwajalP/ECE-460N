        .ORIG x3000

        ; Simple math
        ADD R1, R0, #5        ; R1 = R0 + 5
        AND R2, R1, #3        ; R2 = R1 & 3
        XOR R3, R1, R2        ; R3 = R1 ^ R2
        NOT R4, R3            ; R4 = ~R3
Hello
        ; Memory ops
        ADD R5, R5, #1        ; increment it

        ; Branching
        BRz SKIP              ; branch if zero
        ADD R6, R6, #1        ; skipped if branch taken

SKIP    JSR FUNC              ; jump to subroutine FUNC

        ; Trap call (HALT)
        TRAP x25

; Subroutine
FUNC    ADD R7, R7, #2        ; modify R7
        RET                   ; return

;VAL     .FILL #10             ; memory location containing decimal 10

        .END
