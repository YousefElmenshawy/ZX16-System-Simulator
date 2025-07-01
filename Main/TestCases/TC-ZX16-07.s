li t0, 5             # t0 = 5
jal x0, jump_one     # Jump to jump_one

li a0, 999           # Should be skipped!

jump_one:
li a0, 10            # a0 = 10
blt t0, a0, jump_two # if 5 < 10 → jump

li a1, 999           # Should be skipped if jump happens

jump_two:
add s0, t0, a0       # s0 = 5 + 10 = 15
ecall 8              # dump registers: expect s0 = 15

ecall 10             # exit
