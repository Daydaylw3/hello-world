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
 				; ִ�������ģ���
 				; 076a:0000 7B 00 7A 31 01 00 08 00-03 00 26 00 00 00 31 32   {.z1......&...12
 				; 076a:0010 33 31 32 36 36 36 31 38-33 33 38 00 00 00 00 00   31266618338.....
 				; ���ֻҪ���ַ�����ַָ�� 076a:00e0 ��ִ��show_strģ�鼴��
 				
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
 				push di       ; �洢Ŀ���ַ��ʼƫ�Ƶ�ַ, ����reverse��Ҫ��
 				mov dx, 0
 		 		mov ax, cx
 		 s: mov bx, 0ah
 				call divdb
 				add bl, 30h
 				mov [di], bl  ; ��ת�������ַ��洢��Ŀ���ַ
 				inc di        ; ����Ŀ��ƫ�Ƶ�ַ
 				inc dx        ; ��¼����
 				mov cx, ax
 				jcxz reverse  ; ��Ϊ0, ��˵���Ѿ�����, ��ת����
 				jmp short s
 				
; ��������һ������, �����ǵ����, ��Ҫ����Ū��
reverse:pop bx        ; ����ʼ��ַ�õ�, ��Ӧs1��[push di]
				mov ax, bx    ; ��ax�ݴ�ס��ʼ��ַ
				
 				mov cl, dl    ; �����ȸ���cx
 		 h:	mov dh, [bx]
 		 		push dx
 		 		inc bx
 		 		loop h
 		 		
 		 		mov bx, ax    ; ����Ŀ���ַ����ʼ��ַ�õ�
 		 		
 		 		mov cl, dl
 		h1: pop ax
 				mov [bx], ah
 				inc bx
 				loop h1
 				jmp short s1
 				
 		ok:	ret

 ; ax: ������, bl: ����
 ; ax: ��, bl: ����
 divdb: push cx       ; ��Ϊ���������õ�cx, dx, �������ݴ�, ֮��ָ�
 				push dx
 				push ax       ; �ݴ�סal��ֵ
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
 				
show_str: mov bl, cl        ; ����ɫ��bl��, ����cl��
					mov al, dh        ; ��������еĻ���ַ
          mov ah, 0a0h
          mul ah
          mov cl, 10h
          div cl      ; ��������Ϊ֪��һ��Ϊ16�ı���������ô��
          add ax, 0b800h
          mov es, ax
          mov al, dl        ; ������е�ƫ�Ƶ�ַ
          mov ah, 2
          mul ah
          mov di, ax
          mov cx, 0
    show: mov cl, [si]
    			jcxz ok1
          mov es:[di], cl   ; �ַ�
          mov es:[di+1], bl ; ��ɫ
          inc si
          add di, 2
          jmp short show
          
     ok1: ret
code ends
end start