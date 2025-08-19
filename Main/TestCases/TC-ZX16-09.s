.text
main:
    li t0, 0          # Clear t0
    li t1, 0          # Clear t1
    ecall  8           # Dump before jump

    jal ra, jump_here # Jump to subroutine (ra = return addr)

 
    ecall 8         # Dump after return from jal

    li t1, 23
    ecall 8             # Dump before jalr

    li16 t0, jump_back
    jalr ra, t0    # Jump via jalr to jump_back

    ecall  8           # Final dump

    ecall 10            # Exit


jump_here:
    li t0, 42         # Set t0 to a value to verify jump succeeded
    ecall 8            # Dump inside jump_here
    ret               # jalr ra, 0(ra)

jump_back:
    li t1, 7         # Set t1 to check if jalr worked
    ret               # Return to caller
