ecall 11

li t0, 0x8000           # t0 = pointer to ball data
li s0, 0xF000           # s0 = pointer to tilemap base
li ra, 0

j main_loop

main_loop:

lb a0, 0(t0)            # a0 = old x
lb a1, 1(t0)            # a1 = old y

li s1, 0                # s1 = offset = y * 20
li t1, 0
loop_clear_y:
addi s1, s1, 20
addi t1, t1, 1
blt t1, a1, loop_clear_y

add s1, s1, a0          # s1 += x → tile index
add s1, s1, s0          # s1 = address in tilemap
sb ra, 0(s1)          # store 0 (clear old tile)

lb a0, 0(t0)            # x
lb a1, 2(t0)            # dx
add a0, a0, a1
sb a0, 0(t0)            # store new x

lb a0, 1(t0)            # y
lb a1, 3(t0)            # dy
add a0, a0, a1
sb a0, 1(t0)            # store new y

lb a0, 0(t0)            # new x
lb a1, 1(t0)            # new y

li s1, 0
li t1, 0
loop_draw_y:
addi s1, s1, 20
addi t1, t1, 1
blt t1, a1, loop_draw_y

add s1, s1, a0
add s1, s1, s0          # s1 = tile address
li t1, 1                # tile index for ball
sb t1, 0(s1)            # draw ball tile

ecall 11
j main_loop
