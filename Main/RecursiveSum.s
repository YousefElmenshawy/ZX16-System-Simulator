
j main
main:
li a0, 50
call addsum
ecall 0xA

addsum:
li t1, 0
bne t1, a0, recurse
jr ra

recurse:
addi sp, -8
sw a0, 4(sp)
sw ra, 0(sp)
addi a0, -1
call addsum
lw t0, 4(sp)
lw ra, 0(sp)
add a0, t0
addi sp , 8
jr ra







