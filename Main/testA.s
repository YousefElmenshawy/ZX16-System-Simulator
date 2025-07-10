# RISC-V Assembly - Tile-Based Pong Game
# This program implements a simple Pong game using tile-based graphics

# Memory Map Constants
.equ STACK_TOP, 0xFF34              # Top of stack memory
.equ TILE_MAP_BUFFER_ADDR, 0xF000   # Start of tile map (20x15 grid)
.equ TILE_MAP_END, 0xF12B           # End of tile map buffer
.equ TILE_DEFINITIONS_ADDR, 0xF200  # Start of tile sprite definitions
.equ COLOR_PALLETE_ADDR, 0xFA00     # Start of color palette data

.text
.org 0x0000
j main                              # Jump to main program

.org 0x0020
main:
    # Initialize the game
    li16 sp, STACK_TOP              # Initialize stack pointer
    
    # Calculate initial positions for paddles and ball
    li16 s0, TILE_MAP_BUFFER_ADDR   # s0 = base tile map address
    addi s0, 63                     # s0 += 63 (row 3, column 3)
    addi s0, 63                     # s0 += 63 (row 6, column 6) 
    addi s0, 14                     # s0 += 14 (row 7, column 0) - Player 1 paddle position
    
    mv t0, s0                       # t0 = ball position (copy from s0)
    addi t0, 10                     # t0 += 10 (move ball 10 tiles right from paddle)
    
    mv s1, s0                       # s1 = Player 2 paddle base position
    addi s1, 19                     # s1 += 19 (move to right side of screen)

    # Main game loop
    loop:
        call move_ball              # Update ball position
        call read_input             # Check for player input
        j loop                      # Repeat forever
    
    ecall 0xA                       # Exit system call (never reached)

# Function: read_input
# Purpose: Check for keyboard input and handle player controls
# Registers used: t0, t1, a0, a1, ra
read_input:
    # Save registers on stack
    addi sp, -4                     # Allocate 4 bytes on stack
    sw t0, 0(sp)                    # Save ball position
    sw ra, 2(sp)                    # Save return address

    ecall 0x7                       # System call to read keyboard input
                                    # Returns: a0 = key code, a1 = key pressed flag
    
    li t0, 0                        # Load 0 for comparison
    bne a1, t0, key                 # If key was pressed (a1 != 0), handle it

    # No key pressed - restore and return
    lw ra, 2(sp)                    # Restore return address
    lw t0, 0(sp)                    # Restore ball position
    addi sp, 4                      # Deallocate stack space
    ret                             # Return to caller

key:
    # Key was pressed - check which key and call appropriate handler
    li t0, 22                       # Key code 22 (Player 1 up)
    li t1, 63                       # Base for other key codes
    addi t1, 10                     # t1 = 73 (Player 2 up)
    bne a0, t0, L1                  # If not P1 up, check next key
    call p1up                       # Handle Player 1 up movement
    
    L1:
    bne a0, t1, L2                  # If not P2 up, check next key
    call p2up                       # Handle Player 2 up movement
    
    L2:
    li t1, 63                       # Reset base
    addi t1, 11                     # t1 = 74 (Player 2 down)
    bne a0, t1, L3                  # If not P2 down, check next key
    call p2down                     # Handle Player 2 down movement
    
    L3:
    li t1, 18                       # Key code 18 (Player 1 down)
    bne a0, t1, L4                  # If not P1 down, skip
    call p1down                     # Handle Player 1 down movement
    
    L4:
    # Restore registers and return
    lw ra, 2(sp)                    # Restore return address
    lw t0, 0(sp)                    # Restore ball position
    addi sp, 4                      # Deallocate stack space
    ret                             # Return to caller

# Function: p2up
# Purpose: Move Player 2 paddle up (remove bottom tile, add top tile)
# Registers used: t0, t1, s1
p2up:
    # Check boundary - don't move if at top
    li16 t0, TILE_MAP_BUFFER_ADDR   # Load tile map start
    addi t0, 40                     # t0 = minimum allowed position (2 rows down)
    blt s1, t0, B2                  # If paddle already at top, skip movement
    
    # Move paddle up by removing bottom tile and adding top tile
    addi s1, 20                     # Move to bottom of current paddle
    li t1, 0                        # Tile 0 (empty/background)
    sb t1, 0(s1)                    # Clear bottom tile
    addi s1, -60                    # Move to new top position (3 rows up)
    addi t1, 3                      # Tile 3 (Player 2 paddle sprite)
    sb t1, 0(s1)                    # Place paddle tile at new top
    addi s1, 20                     # Adjust s1 back to middle of paddle
    
    B2:
    ret                             # Return to caller

# Function: p1up  
# Purpose: Move Player 1 paddle up (remove bottom tile, add top tile)
# Registers used: t0, s0
p1up:
    # Check boundary - don't move if at top of screen
    li16 t0, TILE_MAP_BUFFER_ADDR   # Load tile map start address
    beq s0, t0, B1                  # If at very top, skip movement
    
    # Move paddle up by removing bottom tile and adding top tile
    addi s0, 40                     # Move to bottom of current paddle
    li t0, 0                        # Tile 0 (empty/background)
    sb t0, 0(s0)                    # Clear bottom tile
    addi s0, -60                    # Move to new top position (3 rows up)
    addi t0, 2                      # Tile 2 (Player 1 paddle sprite)
    sb t0, 0(s0)                    # Place paddle tile at new top
    
    B1:
    ret                             # Return to caller

# Function: p1down
# Purpose: Move Player 1 paddle down (remove top tile, add bottom tile)
# Registers used: t0, t1, s0
p1down:
    # Check boundary - don't move if at bottom
    li16 t0, TILE_MAP_END           # Load tile map end address
    addi t0, -40                    # t0 = maximum allowed position (2 rows up from bottom)
    bge s0, t0, B3                  # If paddle already at bottom, skip movement
    
    # Move paddle down by removing top tile and adding bottom tile
    addi s0, -20                    # Move to top of current paddle
    li t1, 0                        # Tile 0 (empty/background)
    sb t1, 0(s0)                    # Clear top tile
    addi s0, 60                     # Move to new bottom position (3 rows down)
    addi t1, 2                      # Tile 2 (Player 1 paddle sprite)
    sb t1, 0(s0)                    # Place paddle tile at new bottom
    addi s0, -20                    # Adjust s0 back to middle of paddle
    
    B3:
    ret                             # Return to caller

# Function: p2down
# Purpose: Move Player 2 paddle down (remove top tile, add bottom tile)
# Registers used: t0, t1, s1
p2down:
    # Check boundary - don't move if at bottom
    li16 t0, TILE_MAP_END           # Load tile map end address
    addi t0, -21                    # t0 = maximum allowed position
    bge s1, t0, B4                  # If paddle already at bottom, skip movement
    
    # Move paddle down by removing top tile and adding bottom tile
    addi s1, -20                    # Move to top of current paddle
    li t1, 0                        # Tile 0 (empty/background)
    sb t1, 0(s1)                    # Clear top tile
    addi s1, 60                     # Move to new bottom position (3 rows down)
    addi t1, 3                      # Tile 3 (Player 2 paddle sprite)
    sb t1, 0(s1)                    # Place paddle tile at new bottom
    addi s1, -20                    # Adjust s1 back to middle of paddle
    
    B4:
    ret                             # Return to caller

# Function: move_ball
# Purpose: Move the ball up or down based on current direction
# Registers used: t0, a0, ra
move_ball:
    # Save registers on stack
    addi sp, -4                     # Allocate 4 bytes on stack
    sw ra, 0(sp)                    # Store return address
    sw t0, 2(sp)                    # Store current ball position
    
    # Check ball direction (stored in a0)
    li t0, 0                        # Load 0 for comparison
    beq a0, t0, Up                  # If a0 == 0, ball moves up
    
    # Ball moves down
    lw t0, 2(sp)                    # Restore ball position
    Down:
        call move_down              # Move ball down
        lw ra, 0(sp)                # Restore return address
        addi sp, 4                  # Restore stack
        ret                         # Return to caller
    
    # Ball moves up
    Up:
        lw t0, 2(sp)                # Restore ball position
        call move_up                # Move ball up
        lw ra, 0(sp)                # Restore return address
        addi sp, 4                  # Restore stack
        ret                         # Return to caller

# Function: move_up
# Purpose: Move ball up one row, bounce if hitting top boundary
# Registers used: t0, t1, a0, ra
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

# Function: move_down
# Purpose: Move ball down one row, bounce if hitting bottom boundary
# Registers used: t0, t1, a0, ra
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

# Function: move_left
# Purpose: Move ball left one column (currently unused)
# Registers used: t0, t1
move_left:
    addi t1, 0                      # Initialize t1 (seems redundant)
    li t1, 0                        # Tile 0 (empty/background)
    sb t1, 0(t0)                    # Clear current ball position
    addi t0, -1                     # Move ball left one column
    addi t1, 1                      # Tile 1 (ball sprite)
    sb t1, 0(t0)                    # Place ball at new position
    ret                             # Return to caller

# Function: move_right
# Purpose: Move ball right one column (currently unused)
# Registers used: t0, t1
move_right:
    addi t1, 0                      # Initialize t1 (seems redundant)
    li t1, 0                        # Tile 0 (empty/background)
    sb t1, 0(t0)                    # Clear current ball position
    addi t0, 1                      # Move ball right one column
    addi t1, 1                      # Tile 1 (ball sprite)
    sb t1, 0(t0)                    # Place ball at new position
    ret                             # Return to caller

# =============================================================================
# DATA SECTION - Tile Map and Graphics Data
# =============================================================================

.data
.org 0xF000
# Tile Map Buffer (20x15 grid = 300 tiles)
# Each byte represents one tile on screen
# Tile IDs: 0=empty, 1=ball, 2=player1_paddle, 3=player2_paddle
tiles:
    # Rows 0-5: Empty space (top area)
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    
    # Row 6: Left paddle, empty space, right paddle
    .byte 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3
    
    # Row 7: Left paddle, ball in middle, right paddle
    .byte 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3
    
    # Row 8: Left paddle, empty space, right paddle
    .byte 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3
    
    # Rows 9-14: Empty space (bottom area)
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

# =============================================================================
# TILE DEFINITIONS - Sprite Data (8x16 pixels each, 128 bytes per tile)
# =============================================================================

.org 0xF200

# Tile 0: Empty/Background tile (all transparent)
tile0_data:    .fill 128, 0x00

# Tile 1: Ball sprite (circular shape)
tile1_data:
    .byte 0x00, 0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x00  # Row 0: Small top
    .byte 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00  # Row 1: Wider
    .byte 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00  # Row 2: Same
    .byte 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00  # Row 3: Even wider
    .byte 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00  # Row 4: Same
    .byte 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11  # Row 5: Full width
    .byte 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11  # Row 6: Full width
    .byte 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11  # Row 7: Full width
    .byte 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11  # Row 8: Full width
    .byte 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11  # Row 9: Full width
    .byte 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00  # Row 10: Narrower
    .byte 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00  # Row 11: Same
    .byte 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00  # Row 12: Same
    .byte 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00  # Row 13: Even narrower
    .byte 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00  # Row 14: Same
    .byte 0x00, 0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x00  # Row 15: Small bottom

# Tile 2: Player 1 paddle sprite (right-aligned rectangle)
tile2_data:
    .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11  # Right half filled
    .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11  # (Pattern repeats
    .byte 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11  #  for all 16 rows)
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

# Tile 3: Player 2 paddle sprite (left-aligned rectangle)
tile3_data:
    .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00  # Left half filled
    .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00  # (Pattern repeats
    .byte 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00  #  for all 16 rows)
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

# Tiles 4-15: Additional tile definitions (solid colors)
# These appear to be placeholder tiles filled with solid colors
tile4_data:    .fill 128, 0x44   # Solid color 0x44
tile5_data:    .fill 128, 0x55   # Solid color 0x55
tile6_data:    .fill 128, 0x66   # Solid color 0x66
tile7_data:    .fill 128, 0x77   # Solid color 0x77
tile8_data:    .fill 128, 0x88   # Solid color 0x88
tile9_data:    .fill 128, 0x99   # Solid color 0x99
tile10_data:   .fill 128, 0xAA   # Solid color 0xAA
tile11_data:   .fill 128, 0xBB   # Solid color 0xBB
tile12_data:   .fill 128, 0xCC   # Solid color 0xCC
tile13_data:   .fill 128, 0xDD   # Solid color 0xDD
tile14_data:   .fill 128, 0xEE   # Solid color 0xEE
tile15_data:   .fill 128, 0xFF   # Solid color 0xFF

# =============================================================================
# COLOR PALETTE - 16 color palette for tile graphics
# =============================================================================

.org 0xFA00
palette_data:
    # 16 colors, each color is 1 byte (palette index format)
    .byte 0x00,  0xff,  0xdd,  0xdd,  0xB6,  0x92,  0x91,  0x6D
    .byte 0x6D,  0x49,  0x49,  0x24,  0x24,  0x00,  0x00,  0xFF
