li t0, 0x8000              # ball data address
li s0, 0xF000              # tile map base address

game_loop:
lb t1, 0(t0)               # t1 = ball_x
lb a0, 2(t0)               # a0 = dx
add t1, t1, a0
sb t1, 0(t0)

lb t1, 1(t0)               # t1 = ball_y
lb a0, 3(t0)               # a0 = dy
add t1, t1, a0
sb t1, 1(t0)

li t1, 0                   # offset
li a0, 0                   # counter

lb a1, 1(t0)               # y
li s1, 20                  # constant 20

y_loop:
beq a0, a1, after_y_loop
add t1, t1, s1
addi a0, a0, 1
j y_loop

after_y_loop:
lb a1, 0(t0)               # x
add t1, t1, a1
add t1, t1, s0             # final address in tile map

li a1, 1                   # tile index = 1
sb a1, 0(t1)               # set tile

j game_loop

