# Test delay slot with all branching instructions (conditional and unconditional)

init:           .set noreorder                  # Stops assembler from messing up the tests
                li      $t0, 0                  # Loading in constants for testing
                li      $t1, 1
                li      $t2, 2
                li      $t3, end

                li      $s1, 0                  # All of these should eventually be set to 1;
                li      $s2, 0                  # if they are still 0, the delay slot fails to run;
                li      $s3, 0                  # if they are set to 2, the branch sets the PC counter wrong
                li      $s4, 0                  # everything else is undefined behavior
                li      $s5, 0
                li      $s6, 0
                li      $s7, 0

branch1:        blez    $zero, branch2          # BLEZ: if ($zero <= 0) goto branch2 (should be true)
                li      $s1, 1                  # $s1 = 1
                li      $s1, 2                  # $s1 = 2; should not run

branch2:        bgtz    $t1, branch3            # BGTZ: if ($t1 > 0) goto branch3 (should be true)
                li      $s2, 1                  # $s2 = 1
                li      $s2, 2                  # $s2 = 2; should not run

branch3:        beq     $t1, $t1, branch4       # BEQ: if ($t1 == $t1) goto branch4 (should be true)
                li      $s3, 1                  # $s3 = 1
                li      $s3, 2                  # $s3 = 2; should not run

branch4:        bne     $t1, $zero, branch5     # BNE: if ($t1 != $zero) goto branch5 (should be true)
                li      $s4, 1                  # $s4 = 1
                li      $s4, 2                  # $s4 = 2; should not run

branch5:        j       branch6                 # J: goto branch6
                li      $s5, 1                  # $s5 = 1
                li      $s5, 2                  # $s5 = 2; should not run

branch6:        jal     branch7                 # JAL: goto branch7
                li      $s6, 1                  # $s6 = 1
                li      $s6, 2                  # $s6 = 2; should not run

branch7:        jr      $t3                     # JR: goto $t3 (should go to end)
                li      $s7, 1                  # $s1 = 1
                li      $s7, 2                  # $s1 = 2; should not run

end:            .word   0xfeedfeed
                .word   0x0                     # size of "array"
                .word   0x0                     # size of "array"
                .word   0x0                     # size of "array"
                .word   0xc                     # size of "array"
