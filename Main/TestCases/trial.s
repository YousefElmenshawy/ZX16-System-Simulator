ecall 11

li t0, 0x8000      # Pointer to ball data
li s0, 0xF000      # Pointer to graphics memory
li ra, 0           # Used for negation

j main_loop

main_loop:
lb a0, 0(t0)       # Load x
lb a1, 2(t0)       # Load dx
add a0, a0, a1
sb a0, 0(t0)

li t1, 0
blt a0, t1, flip_dx
li t1, 19
blt t1, a0, flip_dx
j after_x_check

flip_dx:
lb a1, 2(t0)
sub a1, ra, a1
sb a1, 2(t0)

after_x_check:
lb a0, 1(t0)       # y
lb a1, 3(t0)       # dy
add a0, a0, a1
sb a0, 1(t0)

li t1, 0
blt a0, t1, flip_dy
li t1, 14
blt t1, a0, flip_dy
j after_y_check

flip_dy:
lb a1, 3(t0)
sub a1, ra, a1
sb a1, 3(t0)

after_y_check:
lb a0, 1(t0)       # y
li a1, 0
li t1, 0

loop_y:
addi a1, a1, 20
addi t1, t1, 1
blt t1, a0, loop_y

lb t1, 0(t0)       # x
add a1, a1, t1
add a1, a1, s0
li t1, 1           # ball tile index
sb t1, 0(a1)

# Call ecall to trigger screen render
ecall 11

j main_loop
