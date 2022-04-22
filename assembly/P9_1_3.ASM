assume cs:code

code segment
 start: mov ax, 2000h
        mov es, ax
        jmp dword ptr es:[1000h]
        
        mov ax, 4c00h
        int 21h
code ends
end start