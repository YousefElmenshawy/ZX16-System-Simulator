.data
mystr: .string "Here we go, keep believing."

.text
main:
    li16 a0, mystr     # a0 ← address of string
    li t0, 0         # t0 = count of 'e'
    li t1, 63       # t1 = ASCII of 'e'
    addi t1, 38

loop:
    lb a1, 0(a0)         # load character into a1
    li s0, 0       # compare to null terminator manually
    beq a1, s0, done   # if a1 == 0, end of string
    bne a1, t1, skip     # if not 'e', skip counting
    addi t0, 1       # increment counter

skip:
    addi a0, 1       # next char
    j loop

done:
    ecall 8              # dump registers (check t0)
    ecall 10             # exit
