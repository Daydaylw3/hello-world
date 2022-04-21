assume cs:code
a segment
        db 1,2,3,4,5,6,7,8
a ends
b segment
        db 1,2,3,4,5,6,7,8
b ends
c segment
        db 0,0,0,0,0,0,0,0
c ends
code segment
 start: mov ax
        

code ends
end start
