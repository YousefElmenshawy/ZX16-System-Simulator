.text

main:
  
    li a0, 35       # frequency
    li a1, 20       # duration
    ecall 4

    add t0, t0
    add t0, t0
    add t0, t0


    # Test 2: Lose Sound
    li a0, 60
    li a1, 20
    ecall 4


    
    add t0, t0
    add t0, t0
    add t0, t0

    j  main 

    ecall 10
