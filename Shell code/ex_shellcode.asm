[BITS 64]

section .text
global  main

; You will be writing a program that will open a file, read the contents to 
; memory, write the contents in memory to stdout, and then close the file.  To accomplish this,
; you will need to leverage syscall instructions.  The chromium linux syscall table
; is a fantastic reference to help you with this:
;
; https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md
;
; The basic goal is to open a hardcoded filename, but as a stretch goal, try to accept
; the filename as an argument from the terminal!

main:
    ; mov     eax, mystringend - mystring ; or "offset mystringend - offset mystring"
    ; craft the string "myfile.txt" on the stack
    mov     rax, 0x74
    push    rax
    mov     rax, 0x78742e3167616c66
    push    rax
    push    rsp
    pop     rdi; ** New way **move pointer to "myfile.txt" into rdi - 1st argument


    ; open syscall
    mov     rax, 2     ;Call open
    mov     rsi, 0     ;Read and Write
    mov     rdx, 0
    syscall



    ;read syscall
    mov     rdi, rax
    mov     r10, rax ;Store fd for later
    mov     rax, 0
    mov     rsi, 0x403000;** New way ** buffer address
    mov     rdx, 0xfffff
    syscall

    ;write file
    ;leave rdi, already has fd
    mov     rsi, 0x403000 ;** New way ** buffer address
    mov     rdx, rax
    mov     rax, 1
    mov     rdi, 1
    syscall

    ;close
    ;rdi stays the same
    mov     rax, 3
    mov     rdi, 3 ; get fd
    syscall
    

    ; Now let's exit cleanly by calling the exit syscall
    mov rax, 60    ; exit
    mov rbx, 0     ; exit with error_code 0 to indicate success
    syscall        ; syscall = exit(0)
   

