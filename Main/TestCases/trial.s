ecall 11
li t0, 0x8000      # Pointer to ball data (x, y, dx, dy)
li s0, 0xF000      # Pointer to graphics memory

j main_loop

main_loop:
# Update x position
lb a0, 0(t0)       # x
lb a1, 2(t0)       # dx
add a0, a0, a1
sb a0, 0(t0)       # store new x

# Update y position
lb a0, 1(t0)       # y
lb a1, 3(t0)       # dy
add a0, a0, a1
sb a0, 1(t0)       # store new y

# Compute tile index: (y * 20 + x)
lb a0, 1(t0)       # y
li a1, 0
li t1, 0

loop_y:
addi a1, a1, 20
addi t1, t1, 1
blt t1, a0, loop_y

lb t1, 0(t0)       # x
add a1, a1, t1
add a1, a1, s0     # a1 now points to tile memory

li t1, 1           # ball tile index
sb t1, 0(a1)

li ra, 0
ecall 11

j main_loop
