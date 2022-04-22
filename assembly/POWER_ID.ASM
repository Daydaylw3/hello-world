assume cs:codesg

data segment
  db '1975', '1976', '1977', '1978', '1979', '1980', '1981', '1982', '1983'
  db '1984', '1985', '1986', '1987', '1988', '1989', '1900', '1901', '1902'
  db '1903', '1904', '1905'
  ; 以上是表示21年的21个字符串
  dd 16, 22, 382, 1356, 2390, 8000, 16000, 24486, 50065, 97479, 140417, 197514
  dd 345980, 590827, 803530, 1183000, 1843000, 2759000, 3753000, 4649000, 5937000
  ; 以上表示21年公司总收入的21个dword型数据
  dw 3, 7, 9, 13, 28, 38, 130, 220, 476, 778, 1001, 1442, 2258, 2793, 4037, 5635, 8226
  dw 11542, 14430, 15257, 17800
  ; 以上是表示21年公司雇员人数的21个word型数据
data ends

table segment
  db 21 dup ('year summ ne ?? ')
table ends

; 年份0-53h, 收入54h-0a7h, 雇员数0a8h-0d1h

codesg segment
 start: mov ax, data
     mov ds, ax
     mov ax, table
     mov es, ax
     
     mov bx, 0    ;
     mov si, 0    ;
     mov di, 0    ; 代表目的地址
     mov cx, 21
    s: mov ax, [bx]
       mov es:[di], ax
       mov ax, [bx+2]
       mov es:[di+2], ax
       ; 收入
       mov ax, [bx+54h]
       mov es:[di+5], ax
       mov ax, [bx+56h]
       mov es:[di+7], ax
       ; 雇员数
       mov ax, [si+0a8h]
       mov es:[di+0ah], ax
       
       mov ax, [bx+54h]       ; 计算人均收入
       mov dx, [bx+56h]
       div word ptr ds:[si+0a8h] ; [优化] 可以一行写完
       mov es:[di+0dh], ax
       add bx, 4
       add si, 2
       add di, 10h
     loop s
     
     mov ax, 4c00h
     int 21h
codesg ends
end start