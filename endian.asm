.set noreorder
main: 
andi $t0, $t0, 0 
addiu $t1, $t0, 56 
sw $t1, 0($t0) 
addiu $t2, $t0, 4 
sw $t2, 0($t0) 
lbu $t3, 3($t0) 
lbu $t4, 0($t0) 
lw $t5, 0($t0) 
.word 0xfeedfeed
