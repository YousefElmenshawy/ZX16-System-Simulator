li s0, 5         # counter = 5
li s1, 0         # sum = 0
loop:
mv a0, s0       # a0 = s0
add s1, a0       # s1 += a0
addi s0, -1      # s0--
li t0,0
bne s0, t0, loop  # loop if not zero
mv a0, s1        # move sum to a0
ecall 0x08       # print
ecall 0x0A       # exit
