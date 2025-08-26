; Alexander Spicer
; '5DBFWMTT-WIOC' HTTP Server
; '5 Dimensional Brainfuck with Multiverse Time Travel-With Input Output Capabilities' Hyper Text Transfer Protocol Server
; Started 8/26/2025

; First, initialize the entire data memory with ones
; The memory pointer starts at 0, so set that value to
; -1, so that when adding +1 to every cell, the first cell
; will be set to zero, and the loop will stop

-           ; set first cell to -1
    [>+]    ; iterate over memory and add +1 until the present cell is zero

; This is done to allow 'jumping' to the next block of memory
; via a simple '[<]' or '[>]', which allows for easy access to
; data memory without the hassle of manually counting out the
; address

; Secondly, print out a starting message so that we know
; the program has at least loaded successfully, presently
; the data pointer is at 0

>           ; increment the data pointer to not overwrite
            ; the 'block start'

; Multiply 8x8 by repetition, which is 64, Uppercase letters start at 65
; because the output cell is already initialized to 1, the addition of 64
; will cause the output cell to be 65

+++++++
>[-]<
[
    ->
    ++++++++
    [
        ->+<
    ]
    <
]

>>
++++++++.       ; Print 'I'
+++++.          ; Print 'N'
-----.          ; Print 'I'
+++++++++++.    ; Print 'T'
[-]             ; Reset cell to 0
++++++++++.     ; Print '\n'
[-]+            ; Reset cell to 1

; ============================================================

; Now we get ugly, we must now interface with the POSIX system
; through the System V ABI and some dynamic linking to create
; the TCP socket with which to host the webserver to accept
; POST/GET, database, and UDP requests, but also to interface
; with the host file system.

<+<+
[-]>[-]>[-]<<

; initialize a field to 48 such that 

[<]<>>      ; Set the data pointer to the 'FUNC_POINTER' register

            ; Set 'FUNC_POINTER' to the address of symbol 'getstart'
            ; to get the starting address of data memory

.           ; Call the function

<++++++++   ; Set 'DATA_WIDTH' to 8 bytes
<<<<<<<<-,  ; Move 8 bytes from the end of memory and load the starting address

; When creating the socket...