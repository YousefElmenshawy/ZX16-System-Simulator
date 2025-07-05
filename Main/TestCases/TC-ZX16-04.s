li a0, -1         # a0 = -1 (0xFFFF in 16-bit signed)
li a1, 60         # a1 = base address for memory ops

# Store Byte
sb a0, 0(a1)      # store signed byte (-1) into memory[60]

# Load Byte (signed)
lb t0, 0(a1)      # should load -1 (0xFF sign-extended to 0xFFFF)

# Load Byte (unsigned)
lbu t1, 0(a1)     # should load 255 (0x00FF)

ecall 8           # Dump registers

# Store Word (signed 16-bit)
li a0, -12345     # a0 = -12345
sw a0, 2(a1)      # store to memory[62]

# Load Word
lw s0, 2(a1)      # should load -12345 into s0

ecall 8           # Dump again
li a0, 60 	  # address=60
li a1, 4 	  # 4 bytes
ecall 9 	  #Dump Memory

# Exit
ecall 10
