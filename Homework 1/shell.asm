.data
pr:		.asciiz "--> "


.text
.globl main
getCommand:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        sw      $5,12($fp)
        li $v0, 8
        syscall

        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        j       $31
isExit:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        lw      $2,8($fp)
        lb      $3,0($2)
        li      $2,101                  # 0x65
        bne     $3,$2,$L3
        lw      $2,8($fp)
        addiu   $2,$2,1
        lb      $3,0($2)
        li      $2,120                  # 0x78
        bne     $3,$2,$L3
        lw      $2,8($fp)
        addiu   $2,$2,2
        lb      $3,0($2)
        li      $2,105                  # 0x69
        bne     $3,$2,$L3
        lw      $2,8($fp)
        addiu   $2,$2,3
        lb      $3,0($2)
        li      $2,116                  # 0x74
        bne     $3,$2,$L3
        lw      $2,8($fp)
        addiu   $2,$2,4
        lb      $3,0($2)
        li      $2,10                 # 0xa
        bne     $3,$2,$L3
        lw      $2,8($fp)
        addiu   $2,$2,5
        lb      $2,0($2)
        bne     $2,$0,$L3
        li      $2,1                        # 0x1
        b       $L4
$L3:
        move    $2,$0
$L4:
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        j       $31
exitProgram:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        li $v0, 10
        syscall

        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        j       $31
run:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        li $v0, 18
        syscall

        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        j       $31

main:
        addiu   $sp,$sp,-296
        sw      $31,292($sp)
        sw      $fp,288($sp)
        move    $fp,$sp
        sw      $0,24($fp)
        b       $L8
$L11:
		li		$v0, 4
		la		$a0, pr
		syscall
        addiu   $2,$fp,28
        li      $5,1024           # 0x400
        move    $4,$2
        jal     getCommand
        addiu   $2,$fp,28
        move    $4,$2
        jal     isExit
        move    $3,$2
        li      $2,1                        # 0x1
        bne     $3,$2,$L9
        li      $2,1                        # 0x1
        sw      $2,24($fp)
        b       $L8
$L9:
        addiu   $2,$fp,28
        move    $4,$2
        jal     run
$L8:
        lw      $2,24($fp)
        beq     $2,$0,$L11
        jal     exitProgram
        move    $2,$0
        move    $sp,$fp
        lw      $31,292($sp)
        lw      $fp,288($sp)
        addiu   $sp,$sp,296
        j       $31