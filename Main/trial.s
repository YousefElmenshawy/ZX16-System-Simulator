.text
.org 0x0000

start:
    # Initialize Palette at 0xFA00
    li t0, 0xFA00       # base address
    li t1, 0x00         # black
    li a0, 0
    add s1, t0
    add s1, a0
    sb t1, 0(s1)        # color 0 = black

    li t1, 0x0F         # green
    li a0, 1
    add s1, t0
    add s1, a0
    sb t1, 0(s1)        # color 1 = green

    li t1, 0x08         # blue
    li a0, 2
    add s1, t0
    add s1, a0
    sb t1, 0(s1)        # color 2 = blue

    # Initialize Tile 1 at 0xF200 (make it green square)
    li t0, 0xF200
    li t1, 0x11         # pattern: color1 | color1
    li a0, 0
    li a1, 128          # 128 bytes for 16x16 tile
init_tile_loop:
    add s2, t0
    add s2, a0
    sb t1, 0(s2)
    addi a0, 1
    blt a0, a1, init_tile_loop

    # Initialize tile map at 0xF000 to tile 0 (black)
    li t0, 0xF000
    li t1, 0            # tile index 0
    li a0, 0
    li a1, 300          # 20x15 = 300 tiles
init_tilemap_loop:
    add s2, t0
    add s2, a0
    sb t1, 0(s2)
    addi a0, 1
    blt a0, a1, init_tilemap_loop

    # Place ball at (5,5) in tile map (tile 1)
    li t0, 0xF000
    li t1, 5            # x = 5
    li t2, 5            # y = 5
    li t3, 20           # row stride
    mul t4, t2, t3      # t4 = y * 20
    add t4, t4, t1      # t4 = y*20 + x
    add t4, t4, t0      # t4 = address
    li t5, 1            # tile 1
    sb t5, 0(t4)

    # Initialize Ball Data at 0x8000
    li t0, 0x8000
    li t1, 5            # x = 5
    sb t1, 0(t0)
    li t1, 5            # y = 5
    sb t1, 1(t0)
    li t1, 1            # dx = 1
    sb t1, 2(t0)
    li t1, 1            # dy = 1
    sb t1, 3(t0)

    # Prepare
    li s0, 0xF000       # s0 = tilemap base
    li ra, 0

j main_loop

main_loop:
    # Clear old tile
    li t0, 0x8000
    lb a0, 0(t0)        # a0 = x
    lb a1, 1(t0)        # a1 = y

    li s1, 0
    li t1, 0
loop_clear_y:
    addi s1, 20
    addi t1, 1
    blt t1, a1, loop_clear_y

    add s1, s1
    add s1, a0
    add s1, s0
    sb ra, 0(s1)        # clear old tile

    # Update x
    lb a0, 0(t0)
    lb a1, 2(t0)
    add a0, a1
    sb a0, 0(t0)

    # Update y
    lb a0, 1(t0)
    lb a1, 3(t0)
    add a0, a1
    sb a0, 1(t0)

    # Draw new tile
    lb a0, 0(t0)
    lb a1, 1(t0)

    li s1, 0
    li t1, 0
loop_draw_y:
    addi s1, 20
    addi t1, 1
    blt t1, a1, loop_draw_y

    add s1, s1
    add s1, a0
    add s1, s0
    li t1, 1
    sb t1, 0(s1)        # draw ball

    ecall 11
    j main_loop
