.equ STACK_TOP, 0xFF34
.equ TILE_MAP_BUFFER_ADDR, 0xF000
.equ TILE_MAP_END, 0xF12B
.equ TILE_DEFINITIONS_ADDR, 0xF200
.equ COLOR_PALLETE_ADDR, 0xFA00
.equ Center, 0xF096


.text
.org 0x0000
j main
.org 0x0020
main:
  li16 sp, STACK_TOP # Initialize stack pointer
  li16 s0, TILE_MAP_BUFFER_ADDR
  addi s0, 63
  addi s0, 63
  addi s0, 14
  mv s1, s0
  addi s1, 19
  mv t0, s0
  addi t0, 10
  li t1, 10 # x posotion of the ball
  li a1, 0
  li a0, 0
  call move_left
  loop:
  call read_input
  call move_ball
  j loop
  ecall 0xA




read_input:
  addi sp,-8
  sw t0, 0(sp)# store address of ball
  sw t1, 2(sp) #store old x posotion of ball
  sw a0, 4(sp) # store a0
  sw a1, 6(sp) # store a1
  ecall 0x7
  li t0, 0
  bne a1, t0, key
  lw t0, 0(sp)
  lw t1, 2(sp)
  lw a0, 4(sp)
  lw a1, 6(sp)
  addi sp, 8
  ret
  key:
  li t0, 22
  li t1, 63
  addi t1, 10
  bne a0, t0, L1
  addi sp, -2
  sw ra, 0(sp)
  call p1up
  lw ra, 0(sp)
  addi sp, 2
  L1:
  bne a0, t1, L2
  addi sp, -2
  sw ra, 0(sp)
  call p2up
  lw ra, 0(sp)
  addi sp, 2
  L2:
  li t1, 63
  addi t1, 11
  bne a0, t1, L3
  addi sp, -2
  sw ra, 0(sp)
  call p2down
  lw ra, 0(sp)
  addi sp, 2
  L3:
  li t1, 18
  bne a0, t1, L4
  addi sp, -2
  sw ra, 0(sp)
  call p1down
  lw ra, 0(sp)
  addi sp, 2
  L4:
  lw t0, 0(sp)
  lw t1, 2(sp)
  lw a0, 4(sp) # restore a0
  lw a1, 6(sp) # restore key a1
  addi sp, 8
  ret



p2up:
  li16 t0, TILE_MAP_BUFFER_ADDR
  addi t0, 40
  blt s1, t0, B2
  addi s1, 20
  li t1, 0
  sb t1, 0(s1)
  addi s1, -60
  addi t1, 3
  sb t1, 0(s1)
  addi s1, 20
  B2:
      ret




p1up:
  li16 t0, TILE_MAP_BUFFER_ADDR
  beq s0, t0, B1
  addi s0, 40
  li t0, 0
  sb t0, 0(s0)
  addi s0, -60
  addi t0, 2
  sb t0, 0(s0)
  B1:

  ret                  # Jump back to loop


p1down:
  li16 t0, TILE_MAP_END
  addi t0, -40  # Equivalent boundary check for bottom
  bge s0, t0, B3
  addi s0, -20
  li t1, 0
  sb t1, 0(s0)
  addi s0, 60
  addi t1, 2
  sb t1, 0(s0)
  addi s0, -20
  B3:
  ret


p2down:
  li16 t0, TILE_MAP_END
  addi t0, -21  # Equivalent boundary check for bottom
  bge s1, t0, B4
  addi s1, -20
  li t1, 0
  sb t1, 0(s1)
  addi s1, 60
  addi t1, 3
  sb t1, 0(s1)
  addi s1, -20
  B4:
  ret

move_ball:
    addi sp, -4                    # Allocate 4 bytes
    sw ra, 0(sp)                    # Store return address
    sw t0, 2(sp)                    # Save t0 temporarily

    # Check X direction using t0 for comparison
    li t0, 0                        # Use t0 for comparison value
    beq a1, t0, restore_and_left    # Compare X direction

    # Move right - restore t0 first
    lw t0, 2(sp)                    # Restore ball position
    call move_right                 #  CHANGED: move_right updates t0 and t1
    sw t0, 2(sp)                    #  ADDED: Save updated t0 back to stack
    j check_y_direction

restore_and_left:
    lw t0, 2(sp)                    # Restore ball position
    call move_left                  #  CHANGED: move_left updates t0 and t1
    sw t0, 2(sp)                    #  ADDED: Save updated t0 back to stack

check_y_direction:
    # Get the updated t0 from stack (could be Center if ball reset)
    lw t0, 2(sp)                    #  CHANGED: Get potentially updated position
    sw t0, 2(sp)                    # Save it back
    li t0, 0                        # Use t0 for comparison
    beq a0, t0, restore_and_up      # Compare Y direction

    # Move down - restore t0 first
    lw t0, 2(sp)                    #  CHANGED: Get updated ball position
    call move_down
    sw t0, 2(sp)
    j move_ball_end

restore_and_up:
    lw t0, 2(sp)                    #  CHANGED: Get updated ball position
    call move_up
    sw t0, 2(sp)

move_ball_end:
    lw ra, 0(sp)                    # Restore return address
    addi sp, 4                     # FIXED: Restore 4 bytes (not 8)
    ret

move_up:
    # Check if ball is at top boundary
    li16 t1, TILE_MAP_BUFFER_ADDR   # Load tile map start
    addi t1, 20                     # t1 = one row down from top
    blt t0, t1, B1M                 # If ball at top, bounce

    # Move ball up one row
    li t1, 0                        # Tile 0 (empty/background)
    sb t1, 0(t0)                    # Clear current ball position
    addi t0, -20                    # Move ball up one row (20 tiles per row)
    addi t1, 1                      # Tile 1 (ball sprite)
    sb t1, 0(t0)                    # Place ball at new position
    li a0, 0                        # Set direction flag (0 = moving up)
    ret                             # Return to caller

    B1M:
        # Ball hit top boundary - bounce down
        addi sp, -2                 # Allocate stack space
        sw ra, 0(sp)                # Save return address
        call move_down              # Change direction to down
        lw ra, 0(sp)                # Restore return address
        addi sp, 2                  # Deallocate stack space
        ret                         # Return to caller

move_down:
    # Check if ball is at bottom boundary
    li16 t1, TILE_MAP_END           # Load tile map end
    addi t1, -21                    # t1 = one row up from bottom
    bge t0, t1, B2M                 # If ball at bottom, bounce

    # Move ball down one row
    li t1, 0                        # Tile 0 (empty/background)
    sb t1, 0(t0)                    # Clear current ball position
    addi t0, 20                     # Move ball down one row (20 tiles per row)
    addi t1, 1                      # Tile 1 (ball sprite)
    sb t1, 0(t0)                    # Place ball at new position
    li a0, 1                        # Set direction flag (1 = moving down)
    ret                             # Return to caller

    B2M:
        # Ball hit bottom boundary - bounce up
        addi sp, -2                 # Allocate stack space
        sw ra, 0(sp)                # Save return address
        call move_up                # Change direction to up
        lw ra, 0(sp)                # Restore return address
        addi sp, 2                  # Deallocate stack space
        ret                         # Return to caller



move_left:
   addi sp, -4
   sw t0, 0(sp)    # store ball position
   sw t1, 2(sp)    # store ball x position


   # Check if ball is at left edge (x = 0, goal for player 2)
   li t0, 0
   bne t1, t0, L_check_paddle
   j L_goal_p2


   L_check_paddle:
   # Check collision with player 1 paddle
   lw t0, 0(sp)    # get ball position
   addi t0, -1     # position to the left
   lb t0, 0(t0)    # get tile at left position
   li t1, 2        # player 1 paddle tile
   bne t0, t1, L_normal_move
   j L_bounce_right


   L_normal_move:
   # Normal move left - no collision
   lw t0, 0(sp)    # get current ball position
   li t1, 0
   sb t1, 0(t0)    # clear current position
   addi t0, -1     # move left
   li t1, 1        # ball tile
   sb t1, 0(t0)    # place ball at new position


   # Update x position
   lw t1, 2(sp)
   addi t1, -1
   sw t0, 0(sp)    # store new ball position
   sw t1, 2(sp)    # store new x position
   addi sp, 4
   ret


L_bounce_right:
   # Ball hits player 1 paddle, bounce right
   li a1, 1        # set direction to right
   lw t0, 0(sp)
   lw t1, 2(sp)
   addi sp, 4
   ret


L_goal_p2:
   # Ball reached left edge - goal for player 2
   # Reset ball to center
   lw t0, 0(sp)
   li t1, 0
   sb t1, 0(t0)        # clear current ball position


   li16 t0, Center     # center position
   li t1, 1            # ball tile
   sb t1, 0(t0)        # place ball at center


   li t1, 10           # reset x position to center
   li a1, 0            # reset direction to left (or change to 1 for right)


   sw t0, 0(sp)        # store center position
   sw t1, 2(sp)        # store center x position
   addi sp, 4
   ret


move_right:
    addi sp, -4
    sw t0, 0(sp)    # store ball position
    sw t1, 2(sp)    # store ball x position


    # Check if ball is at right edge (x = 19, goal for player 1)
    li t0, 19
    bne t1, t0, R_check_paddle
    j R_goal_p1


    R_check_paddle:
    # Check collision with player 2 paddle
    lw t0, 0(sp)    # get ball position
    addi t0, 1      # position to the right
    lb t0, 0(t0)    # get tile at right position
    li t1, 3        # player 2 paddle tile
    bne t0, t1, R_normal_move
    j R_bounce_left


    R_normal_move:
    # Normal move right - no collision
    lw t0, 0(sp)    # get current ball position
    li t1, 0
    sb t1, 0(t0)    # clear current position
    addi t0, 1      # move right
    li t1, 1        # ball tile
    sb t1, 0(t0)    # place ball at new position


    # Update x position
    lw t1, 2(sp)
    addi t1, 1
    sw t0, 0(sp)    # store new ball position
    sw t1, 2(sp)    # store new x position
    addi sp, 4
    ret


R_bounce_left:
    # Ball hits player 2 paddle, bounce left
    li a1, 0        # set direction to left
    lw t0, 0(sp)
    lw t1, 2(sp)
    addi sp, 4
    ret


R_goal_p1:
    # Ball reached right edge - goal for player 1
    # Reset ball to center
    lw t0, 0(sp)
    li t1, 0
    sb t1, 0(t0)        # clear current ball position


    li16 t0, Center     # center position
    li t1, 1            # ball tile
    sb t1, 0(t0)        # place ball at center


    li t1, 10           # reset x position to center
    li a1, 1            # reset direction to right (or change to 0 for left)


    sw t0, 0(sp)        # store center position
    sw t1, 2(sp)        # store center x position
    addi sp, 4
    ret
.data
.org 0xF000
tiles:
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3
  .byte 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3
  .byte 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0








.org 0xF200








tile0_data:    .fill 128, 0x00




# Ball tiles
tile1_data:
  .byte 0x00, 0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x00
  .byte 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00
  .byte 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00
  .byte 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00
  .byte 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
  .byte 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
  .byte 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
  .byte 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
  .byte 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00
  .byte 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00
  .byte 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00
  .byte 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00
  .byte 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00
  .byte 0x00, 0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x00




# Player 1 tiles
tile2_data:
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
  .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11




# Player 2 tiles
tile3_data:
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00
  .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00




tile4_data:    .fill 128, 0x44
tile5_data:    .fill 128, 0x55
tile6_data:    .fill 128, 0x66
tile7_data:    .fill 128, 0x77
tile8_data:    .fill 128, 0x88
tile9_data:    .fill 128, 0x99
tile10_data:   .fill 128, 0xAA
tile11_data:   .fill 128, 0xBB
tile12_data:   .fill 128, 0xCC
tile13_data:   .fill 128, 0xDD
tile14_data:   .fill 128, 0xEE
tile15_data:   .fill 128, 0xFF




.org 0xFA00
palette_data:
  .byte 0x00,  0xff,  0xdd,  0xdd,  0xB6,  0x92,  0x91,  0x6D
  .byte 0x6D,  0x49,  0x49,  0x24,  0x24,  0x00,  0x00,  0xFF


