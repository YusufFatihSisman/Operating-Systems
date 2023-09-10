.data
info:	.asciiz "Enter an integer\n"
error:	.asciiz "You have to enter an integer bigger than 0\n"
comma:	.asciiz ", "
nl:	    .asciiz "\n"

.text
.globl main
main:
	# print "Enter an integer\n"
	li $v0, 4
	la $a0, info
	syscall
	# get number from user
	li $v0, 5
	syscall
	add $a0, $v0, $zero
	# call factorize func
	jal factorize
	# print nl
	li $v0, 4
	la $a0, nl
	syscall
	# exit
	li $v0, 10
	syscall
			
factorize:
	# i = 1
	li $t0, 1
	# if num < 1, print error message and return
	blt $a0, $t0, printError
	# half = num / 2
	add $t1, $t0, $t0 
	div $a0, $t1
	mflo $t1
	# temp = num
	add $t3, $a0, $zero
	#while i <= half
	while:
		# if i > half, end loop
		bgt $t0, $t1, end
		# if num % i == 0, print num
		div $t3, $t0
		mfhi $t2 # set result of mod operation
		beq $t2, $zero, printNum
		back:
		# i++
		addi $t0, $t0, 1
		# jump start of loop
		j while
	end:
	# print num
	li $v0, 1
	add $a0, $t3, $zero
	syscall
	return:
	# return from func
	jr $ra
	
printNum:
	li $v0, 1
	add $a0, $t0, $zero
	syscall
	li $v0, 4
	la $a0, comma
	syscall
	j back			
	
printError:
	li $v0, 4
	la $a0, error
	syscall
	j return
