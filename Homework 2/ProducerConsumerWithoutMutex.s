.data
data: .word -1
nL: .asciiz "\n"
dataSize: .word 1000

.text
.globl main
main:	
	# call CREATE_THREAD_SYSCALL
	la $a0, consumerThread
	li $v0, 19
	syscall
	add $s3, $v0, $zero
	
	add $t0, $zero, $zero
	lw $t1, dataSize
	
	ProducerWhile:
		bge $t0, $t1, ProducerEnd
		# ADD DATA
		la $t2, data
		sw $t0, 0($t2)
		addi $t0, $t0, 1
		j ProducerWhile
	ProducerEnd:

	# ADD FINISH DATA
	la $t2, data
	addi $t0, $zero, -1
	sw $t0, 0($t2)
	
	# CALL PROCESS EXIT SYSCALL
	li $v0, 25
	syscall
	
	
consumerThread:
	add $t0, $zero, $zero
	ConsumerWhile:
		# GET AND PRINT DATA
		lw $a0, data
		beq $a0, -1, ConsumerEnd
		li $v0, 1
		syscall
		la $a0, nL
		li $v0, 4
		syscall
		j ConsumerWhile
	ConsumerEnd:
	
	# CALL THREAD EXIT
	li $v0, 21
	syscall
