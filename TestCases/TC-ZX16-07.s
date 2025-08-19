.data
array: .word 8, 1, 6, 3, 4

.text
main:
    li16 s0, array        # s0 = base address of array
    li s1, 5            # s1 = number of elements

    li t0, 0            # Outer loop index i = 0
outer_loop:
    mv ra, s1
    addi ra, -1     # ra = n - 1
    bne t0, ra, outer_continue
    j end_sort

outer_continue:
    li t1, 0            # Inner loop index j = 0
inner_loop:
    mv ra, s1
    sub ra, t0      # ra = n - i
    addi ra, -1     # ra = n - i - 1
    bne t1, ra, inner_continue
    j end_inner_loop

inner_continue:
    # Calculate address of array[j]
    mv ra, t1
    slli ra, 2      # ra = j * 4 (word-aligned)
    add ra, s0      # ra = &array[j]

    lw a0, 0(ra)        # a0 = array[j]
    lw a1, 4(ra)        # a1 = array[j+1]

    blt a1, a0, swap
    j no_swap

swap:
    sw a1, 0(ra)        # array[j] = a1
    sw a0, 4(ra)        # array[j+1] = a0

no_swap:
    addi t1, 1      # j++
    j inner_loop

end_inner_loop:
    addi t0, 1      # i++
    j outer_loop

end_sort:
    li16 a0, array        # memory base
    li a1, 20           # 5 words * 4 = 20 bytes
    ecall 9             # dump memory
    ecall 10            # exit
