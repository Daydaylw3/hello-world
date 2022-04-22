assume cs:code

stack segment
				db 16 dup (0)
stack ends

; 从内存 1000:0000 开始执行命令
code segment
 start: mov ax, stack
				mov ss, ax
				mov sp, 16
				mov ax, 0
				push ax
				mov ax, 1000h
				push ax
				retf
code ends
end start