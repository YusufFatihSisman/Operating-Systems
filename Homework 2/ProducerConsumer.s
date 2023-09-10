.data
empty: .word -1
full: .word -1
data: .word -1
nL: .asciiz "\n"
dataSize: .word 300

.text
.globl main
main:
	la $t0, empty
	la $t1, full
	
	# call MUTEX_INIT_SYSCALL
	li $v0, 22
	syscall
	sw $v0, 0($t0)	
	# call MUTEX_INIT_SYSCALL
	li $v0, 22
	syscall
	sw $v0, 0($t1)	
	
	lw $s0, empty	# s0 = id of empty
	lw $s1, full	# s1 = id of fulll
	
	# empty = 1, full = 1
	
	# call MUTEX_LOCK_SYSCALL
	add $a0, $s1, $zero
	li $v0, 23
	syscall
	
	# empty = 1, full = 0
	
	# call CREATE_THREAD_SYSCALL
	la $a0, consumerThread
	li $v0, 19
	syscall
	add $s3, $v0, $zero
	
	add $t0, $zero, $zero
	lw $t1, dataSize
	
	ProducerWhile:
		bge $t0, $t1, ProducerEnd
		# MUTEX_LOCK(EMPTY)
		add $a0, $s0, $zero
		li $v0, 23
		syscall
		# ADD DATA
		la $t2, data
		sw $t0, 0($t2)
		# MUTEX_UNLOCK(FULL)
		add $a0, $s1, $zero
		li $v0, 24
		syscall
		addi $t0, $t0, 1
		j ProducerWhile
	ProducerEnd:

	# MUTEX_LOCK(EMPTY)
	add $a0, $s0, $zero
	li $v0, 23
	syscall
	# ADD FINISH DATA
	la $t2, data
	addi $t0, $zero, -1
	sw $t0, 0($t2)
	# MUTEX_UNLOCK(FULL)
	add $a0, $s1, $zero
	li $v0, 24
	syscall
	
	# CALL PROCESS EXIT SYSCALL
	li $v0, 25
	syscall
	
	
	
consumerThread:
	lw $s0, empty	# s0 = id of empty
	lw $s1, full	# s1 = id of fulll

	ConsumerWhile:
		# MUTEX_LOCK(FULL)
		add $a0, $s1, $zero
		li $v0, 23
		syscall
		# GET AND PRINT DATA
		lw $a0, data
		beq $a0, -1, ConsumerEnd
		li $v0, 1
		syscall
		la $a0, nL
		li $v0, 4
		syscall
		# MUTEX_UNLOCK(EMPTY)
		add $a0, $s0, $zero
		li $v0, 24
		syscall
		j ConsumerWhile
	ConsumerEnd:
	
	# CALL THREAD EXIT
	li $v0, 21
	syscall
