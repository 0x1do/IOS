bits 64

extern isrHandler

%macro pushAll 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popAll 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro



%macro ISR_NO_ERR 1
global isr%1
isr%1:
    pushAll
    cli
    mov rdi, %1
    call isrHandler
    popAll
    iretq
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    pushAll
    cli
    mov rdi, %1
    call isrHandler
    popAll
    add rsp, 8
.halt:
    hlt
    jmp .halt
%endmacro

ISR_NO_ERR 0
ISR_NO_ERR 1
ISR_NO_ERR 2
ISR_NO_ERR 3
ISR_NO_ERR 4
ISR_NO_ERR 5
ISR_NO_ERR 6
ISR_NO_ERR 7
ISR_ERR 8
ISR_NO_ERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NO_ERR 15
ISR_NO_ERR 16
ISR_ERR 17
ISR_NO_ERR 18
ISR_NO_ERR 19
ISR_NO_ERR 20
ISR_ERR 21
ISR_NO_ERR 22
ISR_NO_ERR 23
ISR_NO_ERR 24
ISR_NO_ERR 25
ISR_NO_ERR 26
ISR_NO_ERR 27
ISR_NO_ERR 28
ISR_ERR 29
ISR_ERR 30
ISR_NO_ERR 31

global isr_handlers
isr_handlers:
%assign i 0 
%rep    32 
    dq isr%+i
%assign i i+1 
%endrep