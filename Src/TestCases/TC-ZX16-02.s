lui a0, 0x0001       # a0 = 1<<7 =128
slli a0, 0x1       # a0 = 256


srli a0,  2       # a0 = 64
            # print a0 = 128

slli a0,  3       # a0 = 512

ecall 0x008            # dump registers a0 = 512

ecall 0x00A          # terminate
