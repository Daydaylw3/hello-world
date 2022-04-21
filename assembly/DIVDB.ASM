assume cs:code

code segment
 start: mov ax, 0
 		mov dx, 317ah
 		mov bl, 10
 		mov cx, 256
 		call divdb

 		mov ax, 4c00h
 		int 21h

 		; x/n = int(h/n)*256+[rem(h/n)*256+l]/n
 		; ah/bl*256 + (rem(ah/bl)*256+al)/bl
 divdb: mov ah, dh
 		div dl
 		; 余数在ah, 
 		mov ax, ah
 		mov ah, 0
 		mul cx
 		ret
code ends
end start