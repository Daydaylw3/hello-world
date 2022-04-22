assume cs:code

data segment
        dd 12345678h
data ends

code segment
 start: mov ax, data
        mov ds, ax
        mov bx, 0
        mov [bx], bx    ; (ip)=0
        mov [bx+2], cs  ; (cs)=(cs)
        jmp dword ptr ds:[0]
code ends
end start