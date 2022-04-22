assume cs:code, ss:stack

data segment
				dw 123, 12666, 1, 8, 3, 38
				; 076a:0000 7B 00 7A 31 01 00 08 00-03 00 26 00
				db 15 dup (0)
data ends

stack segment
				dw 8 dup (0)
stack ends

code segment
 start: mov ax, stack
 				mov ss, ax
 				mov sp, 16
 				mov ax, data
 				mov ds, ax
 				mov si, 0
 				call dtoc       
 				; 执行完这个模块后
 				; 076a:0000 7B 00 7A 31 01 00 08 00-03 00 26 00 00 00 31 32   {.z1......&...12
 				; 076a:0010 33 31 32 36 36 36 31 38-33 33 38 00 00 00 00 00   31266618338.....
 				; 因此只要将字符串地址指向 076a:00e0 再执行show_str模块即可
 				
 				mov si, 0eh
 				mov dh, 8
 				mov dl, 3
 				mov cl, 2
 				call show_str
 				
 				mov ax, 4c00h
 				int 21h
 				
 	dtoc: mov di, 0eh
 		s1:	mov cx, [si]
 				jcxz ok
 				add si, 2
 				push di       ; 存储目标地址起始偏移地址, 后面reverse需要用
 				mov dx, 0
 		 		mov ax, cx
 		 s: mov bx, 0ah
 				call divdb
 				add bl, 30h
 				mov [di], bl  ; 将转换过的字符存储到目标地址
 				inc di        ; 递增目标偏移地址
 				inc dx        ; 记录长度
 				mov cx, ax
 				jcxz reverse  ; 商为0, 则说明已经除尽, 跳转处理
 				jmp short s
 				
; 处理完了一个数字, 但是是倒序的, 需要将其弄正
reverse:pop bx        ; 将起始地址拿到, 对应s1处[push di]
				mov ax, bx    ; 由ax暂存住起始地址
				
 				mov cl, dl    ; 将长度赋给cx
 		 h:	mov dh, [bx]
 		 		push dx
 		 		inc bx
 		 		loop h
 		 		
 		 		mov bx, ax    ; 重新目标地址将起始地址拿到
 		 		
 		 		mov cl, dl
 		h1: pop ax
 				mov [bx], ah
 				inc bx
 				loop h1
 				jmp short s1
 				
 		ok:	ret

 ; ax: 被除数, bl: 除数
 ; ax: 商, bl: 余数
 divdb: push cx       ; 因为接下来会用到cx, dx, 所以先暂存, 之后恢复
 				push dx
 				push ax       ; 暂存住al的值
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
 				
show_str: mov bl, cl        ; 将颜色用bl存, 不用cl存
					mov al, dh        ; 计算出该行的基地址
          mov ah, 0a0h
          mul ah
          mov cl, 10h
          div cl      ; 这里是因为知道一行为16的倍数才能这么做
          add ax, 0b800h
          mov es, ax
          mov al, dl        ; 计算该列的偏移地址
          mov ah, 2
          mul ah
          mov di, ax
          mov cx, 0
    show: mov cl, [si]
    			jcxz ok1
          mov es:[di], cl   ; 字符
          mov es:[di+1], bl ; 颜色
          inc si
          add di, 2
          jmp short show
          
     ok1: ret
code ends
end start