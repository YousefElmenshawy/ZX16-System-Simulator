.equ STACK_TOP, 0xFF34
.equ TILE_MAP_BUFFER_ADDR, 0xF000
.equ TILE_MAP_END, 0xF12B
.equ TILE_DEFINITIONS_ADDR, 0xF200
.equ COLOR_PALLETE_ADDR, 0xFA00
.equ Center, 0xF096
.equ pauseBig, 0x0200
.equ pauseSmall, 0x0020


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
  li t1, 10 # x position of the ball
  li a1, 0
  li a0, 0
  loop:
  call read_input
  call move_ball
  addi sp, -4
    sw a0, 0(sp) # store a0 (direction)
    sw a1, 2(sp) # store a1 (key)
    li a0, 0
    li16 a1, pauseSmall   # some arbitrary delay loop count
        delay_loop:
          addi a1, -1
          bne a1, a0, delay_loop
    lw a0, 0(sp) # restore a0 (direction)
    lw a1, 2(sp) # restore a1 (key)
    addi sp, 4
  j loop
  ecall 0xA


read_input:
  addi sp, -8
  sw t0, 0(sp)  # store address of ball
  sw t1, 2(sp) # store old x position of ball
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
  addi sp, -6
  sw t0, 0(sp)  # save t0
  sw t1, 2(sp)  # save t1 (ball x-position)
  sw ra, 4(sp)  # save ra

  li16 t0, TILE_MAP_BUFFER_ADDR
  addi t0, 40
  blt s1, t0, B2
  addi s1, 20
  li t1, 0
  sb t1, 0(s1)
  addi s1, -60
  li t1, 3
  sb t1, 0(s1)
  addi s1, 20
  B2:
  lw t0, 0(sp)  # restore t0
  lw t1, 2(sp)  # restore t1
  lw ra, 4(sp)  # restore ra
  addi sp, 6
  ret


p1up:
  addi sp, -6
  sw t0, 0(sp)  # save t0
  sw t1, 2(sp)  # save t1 (ball x-position)
  sw ra, 4(sp)  # save ra

  li16 t0, TILE_MAP_BUFFER_ADDR
  beq s0, t0, B1
  addi s0, 40
  li t0, 0
  sb t0, 0(s0)
  addi s0, -60
  li t0, 2
  sb t0, 0(s0)
  B1:
  lw t0, 0(sp)  # restore t0
  lw t1, 2(sp)  # restore t1
  lw ra, 4(sp)  # restore ra
  addi sp, 6
  ret


p1down:
  addi sp, -6
  sw t0, 0(sp)  # save t0
  sw t1, 2(sp)  # save t1 (ball x-position)
  sw ra, 4(sp)  # save ra

  li16 t0, TILE_MAP_END
  addi t0, -40  # Equivalent boundary check for bottom
  bge s0, t0, B3
  addi s0, -20
  li t1, 0
  sb t1, 0(s0)
  addi s0, 60
  li t1, 2
  sb t1, 0(s0)
  addi s0, -20
  B3:
  lw t0, 0(sp)  # restore t0
  lw t1, 2(sp)  # restore t1
  lw ra, 4(sp)  # restore ra
  addi sp, 6
  ret


p2down:
  addi sp, -6
  sw t0, 0(sp)  # save t0
  sw t1, 2(sp)  # save t1 (ball x-position)
  sw ra, 4(sp)  # save ra

  li16 t0, TILE_MAP_END
  addi t0, -21  # Equivalent boundary check for bottom
  bge s1, t0, B4
  addi s1, -20
  li t1, 0
  sb t1, 0(s1)
  addi s1, 60
  li t1, 3
  sb t1, 0(s1)
  addi s1, -20
  B4:
  lw t0, 0(sp)  # restore t0
  lw t1, 2(sp)  # restore t1
  lw ra, 4(sp)  # restore ra
  addi sp, 6
  ret


move_ball:
  addi sp, -6
  sw ra, 0(sp)    # Store return address
  sw t0, 2(sp)    # Save t0 (ball position)
  sw t1, 4(sp)    # Save t1 (ball x-position)

  # Check X direction
  li t0, 0
  bne a1, t0, move_right_call

  # Move left
  lw t0, 2(sp)    # Restore ball position
  lw t1, 4(sp)    # Restore ball x-position
  call move_left
  sw t0, 2(sp)    # Save updated ball position
  sw t1, 4(sp)    # Save updated ball x-position
  j check_y_direction

move_right_call:
  lw t0, 2(sp)    # Restore ball position
  lw t1, 4(sp)    # Restore ball x-position
  call move_right

  sw t0, 2(sp)    # Save updated ball position
  sw t1, 4(sp)    # Save updated ball x-position

check_y_direction:
  li t0, 0
  bne a0, t0, move_down_call

  # Move up
  lw t0, 2(sp)    # Get updated ball position
  lw t1, 4(sp)    # Get updated ball x-position
  call move_up
  sw t0, 2(sp)    # Save updated ball position
  sw t1, 4(sp)    # Save updated ball x-position
  j move_ball_end

move_down_call:
  lw t0, 2(sp)    # Get updated ball position
  lw t1, 4(sp)    # Get updated ball x-position
  call move_down
  sw t0, 2(sp)    # Save updated ball position
  sw t1, 4(sp)    # Save updated ball x-position

move_ball_end:
  lw ra, 0(sp)    # Restore return address
  lw t0, 2(sp)    # Restore ball position
  lw t1, 4(sp)    # Restore ball x-position
  addi sp, 6
  ret


move_up:
  addi sp, -6
  sw ra, 0(sp)    # save ra
  sw t0, 2(sp)    # save t0 (for boundary check)
  sw t1, 4(sp)    # save t1 (ball x-position)

  # Check if ball is at top boundary
  li16 t1, TILE_MAP_BUFFER_ADDR
  addi t1, 20
  bge t0, t1, move_up_normal  # If ball NOT at top, move normally

  # Ball hit top boundary - bounce (change direction only)
  li a0, 1        # Set direction flag to down
  lw ra, 0(sp)    # restore ra
  lw t0, 2(sp)    # restore t0
  lw t1, 4(sp)    # restore t1
  addi sp, 6
  ret

move_up_normal:
  # Move ball up one row
  li t1, 0
  sb t1, 0(t0)    # Clear current ball position
  addi t0, -20    # Move ball up one row
  li t1, 1
  sb t1, 0(t0)    # Place ball at new position
  li a0, 0        # Set direction flag (0 = moving up)

  lw ra, 0(sp)    # restore ra
  lw t1, 4(sp)    # restore t1 (ball x-position)
  addi sp, 6
  ret


move_down:
  addi sp, -6
  sw ra, 0(sp)    # save ra
  sw t0, 2(sp)    # save t0 (for boundary check)
  sw t1, 4(sp)    # save t1 (ball x-position)

  # Check if ball is at bottom boundary
  li16 t1, TILE_MAP_END
  addi t1, -21
  blt t0, t1, move_down_normal  # If ball NOT at bottom, move normally

  # Ball hit bottom boundary - bounce (change direction only)
  li a0, 0        # Set direction flag to up
  lw ra, 0(sp)    # restore ra
  lw t0, 2(sp)    # restore t0
  lw t1, 4(sp)    # restore t1
  addi sp, 6
  ret

move_down_normal:
  # Move ball down one row
  li t1, 0
  sb t1, 0(t0)    # Clear current ball position
  addi t0, 20     # Move ball down one row
  li t1, 1
  sb t1, 0(t0)    # Place ball at new position
  li a0, 1        # Set direction flag (1 = moving down)

  lw ra, 0(sp)    # restore ra
  lw t1, 4(sp)    # restore t1 (ball x-position)
  addi sp, 6
  ret
move_left:
  addi sp, -6
  sw ra, 0(sp)    # save ra
  sw t0, 2(sp)    # save t0 (ball position)
  sw t1, 4(sp)    # save t1 (ball x position)

  # Check if ball is at left edge (x = 0, goal for player 2)
  li t0, 0
  bne t1, t0, L_check_paddle
  j L_goal_p2

L_check_paddle:
  # Check collision with player 1 paddle
  lw t0, 2(sp)    # get ball position
  addi t0, -1     # position to the left
  lb t0, 0(t0)    # get tile at left position
  li t1, 2        # player 1 paddle tile
  bne t0, t1, L_normal_move
  j L_bounce_right

L_normal_move:
  # Normal move left - no collision
  lw t0, 2(sp)    # get current ball position
  li t1, 0
  sb t1, 0(t0)    # clear current position
  addi t0, -1     # move left
  li t1, 1        # ball tile
  sb t1, 0(t0)    # place ball at new position

  # Update x position
  lw t1, 4(sp)    # get current x position
  addi t1, -1     # decrement x position

  lw ra, 0(sp)    # restore ra
  addi sp, 6
  ret

L_bounce_right:
  # Ball hits player 1 paddle, bounce right
  li a1, 1        # set direction to right
  lw ra, 0(sp)    # restore ra
  lw t0, 2(sp)    # restore t0
  lw t1, 4(sp)    # restore t1
  addi sp, 6
  ret

L_goal_p2:
  # Ball reached left edge - goal for player 2
  # Reset ball to center
  lw t0, 2(sp)    # get current ball position
  li t1, 0
  sb t1, 0(t0)    # clear current ball position

  li16 t0, Center # center position
  li t1, 1        # ball tile
  sb t1, 0(t0)    # place ball at center


  li t1, 0
  li16 a1, pauseBig   # some arbitrary delay loop count
  pause_loop:
    addi a1, -1
    bne a1, t1, pause_loop

    li t1, 10       # reset x position to center
      li a1, 0        # reset direction to left

  lw ra, 0(sp)    # restore ra
  addi sp, 6
  ret


move_right:
  addi sp, -6
  sw ra, 0(sp)    # save ra
  sw t0, 2(sp)    # save t0 (ball position)
  sw t1, 4(sp)    # save t1 (ball x position)

  # Check if ball is at right edge (x = 19, goal for player 1)
  li t0, 19
  bne t1, t0, R_check_paddle
  j R_goal_p1

R_check_paddle:
  # Check collision with player 2 paddle
  lw t0, 2(sp)    # get ball position
  addi t0, 1      # position to the right
  lb t0, 0(t0)    # get tile at right position
  li t1, 3        # player 2 paddle tile
  bne t0, t1, R_normal_move
  j R_bounce_left

R_normal_move:
  # Normal move right - no collision
  lw t0, 2(sp)    # get current ball position
  li t1, 0
  sb t1, 0(t0)    # clear current position
  addi t0, 1      # move right
  li t1, 1        # ball tile
  sb t1, 0(t0)    # place ball at new position

  # Update x position
  lw t1, 4(sp)    # get current x position
  addi t1, 1      # increment x position

  lw ra, 0(sp)    # restore ra
  addi sp, 6
  ret

R_bounce_left:
  # Ball hits player 2 paddle, bounce left
  li a1, 0        # set direction to left
  lw ra, 0(sp)    # restore ra
  lw t0, 2(sp)    # restore t0
  lw t1, 4(sp)    # restore t1
  addi sp, 6
  ret

R_goal_p1:
  # Ball reached right edge - goal for player 1
  # Reset ball to center
  lw t0, 2(sp)    # get current ball position
  li t1, 0
  sb t1, 0(t0)    # clear current ball position

  li16 t0, Center # center position
  li t1, 1        # ball tile
  sb t1, 0(t0)    # place ball at center

  li t1, 10       # reset x position to center
  li a1, 1        # reset direction to right

  lw ra, 0(sp)    # restore ra
  addi sp, 6
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