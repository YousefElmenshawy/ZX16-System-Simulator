.data
prompt1: .string "Enter first number: \0"
prompt2: .string "Enter second number: \0"
add_msg: .string "\nAddition complete. Dumping registers:\0"
sub_msg: .string "\nSubtraction complete. Dumping registers:\0"

.text
.org 0x0020
main:
    # read the first number
    la a0, prompt1       # Address of prompt1
    ecall 3             # Print string
    ecall 2             # Read integer
    mv s0, a0           # s0 = a0 (user's first number)

    # read the second number 
    la a0, prompt2       # Address of prompt2
    ecall 3             # Print string
    ecall 2             # Read integer
    mv s1, a0           # s1 = a0 (user's second number)

    # Calculate and Display Addition
    la a0, add_msg       # Address of add_msg
    ecall 3             # Print string
    mv t0, s0           # t0 = s0
    add t0, s1          # t0 = t0 + s1
    ecall 8             # Dump registers to show result in t0

    # Calculate and Display Subtraction 
    la a0, sub_msg       # Address of sub_msg
    ecall 3             # Print string
    mv t1, s0           # t1 = s0
    sub t1, s1          # t1 = t1 - s1
    ecall 8             # Dump registers to show result in t1

end_program:
    # Test: Program Exit (ECALL 10) 
    ecall 10            # Halt the simulator

