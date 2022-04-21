assume cs:code, ds:data

data segment
        db 'welcome to masm!'
	; 绿、绿底红色、白底蓝色
	; 0x02、0x24、0x71
	dw 2, 36, 113
data ends

code segment
 start: mov ax, data
	mov ds, ax
	mov ax, 0b8aah
	mov es, ax
	; 12行为 0x0AA0~0x0B3F
	; 13行为 0x0B40~0x0BDF
	; 14行为 0x0BE0~0x0C7F
	; 偏移40h~5fh
	mov si, 16
	mov di, 0
	mov cx, 3
     s: mov bx, 0
	mov dx, cx
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
	mov cx, dx
	loop s

	mov ax, 4c00h
	int 21h
code ends
end start