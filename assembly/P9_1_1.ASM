assume cs:code

data segment
        db 00, 00, 00
data ends

code segment
 start: mov ax, data
        mov ds, ax
        mov bx, 0
        jmp word ptr [bx+1]
code ends
end start