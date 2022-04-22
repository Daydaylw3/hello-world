assume cs:code, ds:data, ss:stack

data segment
 				db 'welcome to masm!'
 				; 绿、绿底红色、白底蓝色
 				; 0x02、0x24、0x71
 				dw 2, 36, 113
data ends

stack segment
 				dw 8 dup (0)
stack ends

; 12行为 B800:0x06E0~0x077F
; 13行为 B800:0x0780~0x081F
; 14行为 B800:0x0820~0x08BF
; 偏移40h~5fh
 				
code segment
 start: mov ax, data
 				mov ds, ax
 				mov ax, 0b86Eh ; 从第12行的首地址开始
 				mov es, ax
 				mov ax, stack
 				mov ss, ax
 				mov sp, 16
 				mov si, 16
 				mov di, 0
 				mov cx, 3
		 s: mov bx, 0
 				push cx
 				
 				mov cx, 16
 		s1: mov al, [bx]
 				mov es:[di+40h], al
 				mov al, [si]
 				mov es:[di+41h], al
 				add bx, 1
 				add di, 2
 				loop s1
 				
 				add si, 2
 				add di, 80h
 				pop cx
 				loop s

 				mov ax, 4c00h
 				int 21h
code ends
end start