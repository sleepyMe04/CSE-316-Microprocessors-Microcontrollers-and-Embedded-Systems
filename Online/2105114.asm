.model small
.stack 100h
.code
main proc
    mov ax,23126
    push ax
    call sum
    call print_number
    mov ah,4ch
    int 21h
    
    main endp 


sum proc near:
    push bp
    mov bp,sp
    
    
    mov ax,[bp+4]
    cmp ax,0
    je zero
    xor bx,bx
    xor dx,dx
    mov bx,10
    div bx
    push dx
    ;mov ax,dx
    push ax
    call sum
    pop dx
    add ax,dx
    jmp return
    
    
    zero:
    mov ax,0
    jmp return
    
    return:
    pop bp
    ret 2
sum endp


  
print_number proc
push ax
push bx
push cx
push dx

mov bx, 10 
xor cx, cx 

next_digit:
xor dx, dx
div bx 
push dx 
inc cx
cmp ax, 0
jne next_digit

print_digits:
pop dx 
add dl, '0' 
mov ah, 02h
int 21h
loop print_digits

pop dx
pop cx
pop bx
pop ax
ret
print_number endp
    


end main