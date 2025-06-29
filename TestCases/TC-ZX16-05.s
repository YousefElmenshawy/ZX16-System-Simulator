li a0, 5          # a0 = 5
jal x0, label1    # jump to label1 (does not save return address)

li a0, 999        # this should be skipped

label1:
li a1, 10         # a1 = 10
blt a0, a1, label2  # if a0 < a1 → jump to label2

li a1, 999        # this should be skipped if jump happens

label2:
add a0, a1        # a0 = a0 + a1 → should be 5 + 10 = 15
ecall 8           # dump registers

ecall 10          # exit
