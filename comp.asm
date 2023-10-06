# Tests comparisons like slt, slti sltu, sltiu
main: addi    $t0, $t0, -50     # unsigned 4294967246
      addi    $t1, $t1, -25     # unsigned 4294967271

      slt     $s7, $t0, $t1     # TRUE --> $s7 = 1
      add     $s0, $s0, $s7

      sltu    $s7, $t0, $t1     # TRUE 
      add     $s0, $s0, $s7

      sltu    $s7, $t0, $zero     # FALSE
      add     $s1, $s1, $s7

      slt     $s7, $t0, $zero   # TRUE --> s7 = 1
      add     $s0, $s0, $s7

      addi    $t1, $zero, 2300  
      slt     $s7, $t0, $t1     # TRUE --> s7 = 1
      add     $s0, $s0, $s7

      sltu    $s7, $t0, $t1     # FALSE
      add     $s1, $s1, $s7    

      addi    $t1, $zero, -50   
      slt     $s7, $t0, $t1     # FALSE --> S7 = 0
      add     $s1, $s1, $s7

      sltu    $s7, $t0, $t1     # FALSE --> S7 = 0
      add     $s1, $s1, $s7

      addi    $t2, $zero, 0x8000 
      sll     $t2, $t2, 16      # $t2 = 0x8000000
      slt     $s7, $t2, $t0     # TRUE --> S7 = 1
      add     $s0, $s0, $s7

      sltu    $s7, $t2, $t0     # TRUE
      add     $s0, $s0, $s7

      slt     $s7, $t0, $t2     # FALSE
      add     $s1, $s1, $s7     

      addi    $t3, $zero, 10
      addi    $t4, $zero, 12
      slt     $s7, $t3, $t4     # TRUE
      add     $s0, $s0, $s7
      sltu    $s7, $t3, $t4     # TRUE
      add     $s0, $s0, $s7
      
      ## slti sltiu tests
      slti    $s7, $t3, 0x0f00  # TRUE
      add     $s0, $s0, $s7
      
      slti    $s7, $t3, 0x8f00  # FALSE
      add     $s1, $s1, $s7

      sltiu   $s7, $t3, 0x8f00  # TRUE
      add     $s0, $s0, $s7

      addi    $t3, $zero, -2
      slti    $s7, $t3, 0x8000  # FALSE
      add     $s1, $s1, $s7

      sltiu   $s7, $t3, 0x8000  # FALSE
      add     $s1, $s1, $s7

      slti    $s7, $t3, -1      # TRUE
      add     $s0, $s0, $s7

      slti    $s7, $t3, 0      # TRUE
      add     $s0, $s0, $s7

      slti    $s7, $t3, -3      # FALSE
      add     $s1, $s1, $s7
      
      sltiu   $s7, $t3, -2000   # FALSE
      add     $s1, $s1, $s7

      addi    $t5, $zero, 5
      sltiu   $s7, $t5, -1      # TRUE
      add     $s0, $s0, $s7

      slti    $s7, $t5, -1      # FALSE
      add     $s1, $s1, $s7

      .word 0xfeedfeed          # s0 = 13 i.e. 0xd and s1 = 0

