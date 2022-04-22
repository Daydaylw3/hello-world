assume cs:code, ss:stack

stack segment
				dw 8 dup (0)
stack ends

code segment
 start: mov ax, stack
 				mov ss, ax
 				mov sp, 16
 				
 				mov ax, 317ah
 				mov bl, 0ah
 				call divdb
 				
 				; ax=04f2, bx=0006
 				mov ax, 4c00h
 				int 21h
 				
 divdb: push cx
 				push dx
 				push ax
 				mov al, ah
 				mov ah, 0
 				div bl
 				mov cl, al
 				pop dx
 				mov al, dl
 				div bl
 				mov bl, ah
 				mov ah, cl
 				pop dx
 				pop cx
 				ret
code ends
end start