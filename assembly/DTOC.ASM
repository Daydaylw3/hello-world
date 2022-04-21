assume cs:code

data segment
		dw 123, 12666, 1, 8, 3, 38
		; 076a:0000 7B 00 7A 31 01 00 08 00 03 00 26 00
		db 16 dup (0)
data ends

code segment
 start: mov ax, data
 		mov ds, ax
 		mov si, 0
 		call dtoc

 		mov ax, 4c00h
 		int 21h

  dtoc: mov ax, data  ; 声明目标地址
  		mov es, ax
  		mov di, 0eh

  	 s: mov bx, 0
  		mov cx, 0
  	    mov ax, [si]
  	s1: mov bl, 10
  		div bl
  		; 将余数=》bl, 商=》cl
  		; 若cx=0, 则说明已除尽
  		mov bl, ah
  		add bl, 30h   ; 转换成数字字符
  		inc bh        ; 用bh记录数字长度
  		mov ah, 0     ; 余数归0
  		mov cl, al
  		mov es:[di], bl
  		inc di
  		jcxz s2       ; 处理下一个数
  		jmp short s1

  		; 处理倒序的数字
    s2: mov cl, bh

    	add si, 2
    	mov cx, [si]
    	jcxz s3
    	jmp short s

  	s3:	ret
code ends
end start