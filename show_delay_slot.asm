.set noreorder
main: beq  $t0, $t7, end    # should branch
      addiu $t0, $t0, 16    # should still affect $t0
      addiu $t1, $t1, 20    # should not affect $t1
      nop
      nop
      nop
      nop
end:  .word 0xfeedfeed
