assume cs:code

data segment
				db 'Welcome to masm!', 0
data ends

code segment
 start: mov ax, data
 				mov ds, ax
 				mov si, 0
 				mov dh, 8
 				mov dl, 3
 				mov cl, 2
 				call show_str
 				
 				mov ax, 4c00h
 				int 21h

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
    			jcxz ok
          mov es:[di], cl   ; �ַ�
          mov es:[di+1], bl ; ��ɫ
          inc si
          add di, 2
          jmp short show
          
      ok: ret
code ends
end start