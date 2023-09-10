.data
prime:	.asciiz " Prime\n"
newline:	.asciiz "\n"

.text
.globl main
main:
	# i = 0
	add $t4, $zero, $zero
	# set while limit to 1000
	li $t5, 1000
	
	while:
		# if i == 1000, jump end
		bgt $t4, $t5, end
		# set parameter and call isPrime
		add $a0, $t4, $zero
		jal isPrime
		# print
		beq $v0, $zero, print  # number is not prime
		j printPrime	   #number is prime
		back:
		# i++
		addi $t4, $t4, 1
		# jump start of while
		j while
	end:
	
	#exit
	li $v0, 10
	syscall

printPrime:
 	li $v0, 1
    	add $a0, $t4, $zero
    	syscall
	li $v0, 4
	la $a0, prime
	syscall
	j back
print:
	li $v0, 1
    	add  $a0, $t4, $zero 
    	syscall
    	li $v0, 4
	la $a0, newline
	syscall
    	j back

# use t1, t2, t3
isPrime:
	# i = 2
	li $t1, 2	
	# if num < 2, return 0
	li $t2, 2
	blt $a0, $t2, return0	
	# if num == 2, return 1
	li $t2, 2
	beq $a0, $t2, return1
	# half = num / 2
	div $a0, $t1
	mflo $t2
	# while i <= half
	while2:
		# if i > half, end
		bgt $t1, $t2, end2
		# if num % i == 0, return 0
		div $a0, $t1
		mfhi $t3	# set result of mod operation
		beq $t3, $zero, return0
		# i++
		addi $t1, $t1, 1
		# j while
		j while2
	end2:
	j return1
	
	return0:
		add $v0, $zero, $zero
		jr $ra
	return1:
		li $v0, 1
		jr $ra
		
	
			
