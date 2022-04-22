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
    			jcxz ok
          mov es:[di], cl   ; 字符
          mov es:[di+1], bl ; 颜色
          inc si
          add di, 2
          jmp short show
          
      ok: ret
code ends
end start