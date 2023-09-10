.data
size:	.asciiz "Enter size of array: "
inputs:	.asciiz "Enter array elements: "
nl:	.asciiz "\n"
comma:	.asciiz ", "
sorted:	.asciiz "Sorted array: "
array: .asciiz "Array: "

.text
.globl main
main:
	# print "enter size of array"
	li $v0, 4
	la $a0, size
	syscall
	# get size from user and multiple with 4
	li $v0, 5
	syscall
	add $t0, $v0, $zero
	add $t0, $t0, $t0
	add $t0, $t0, $t0
	# initialize the array
	li $v0, 9
	add $a0, $t0, $zero
	syscall
	add $t1, $v0, $zero # t1 = address of array[0]
	# i = 0
	add $t2, $zero, $zero
	# set t3 to beginning of array adress
	add $t3, $t1, $zero
	# take array elements in loop
	while:
		# if i == size, end loop
		beq $t2, $t0, end
		# get element and assign to array
		li $v0, 5
		syscall
		sw $v0, 0($t3)
		# increase current address
		addi $t3, $t3, 4 
		# increase index
		addi $t2, $t2, 4
		# go back start of loop
		j while
	end: 
	# i = 0
	add $t2, $zero, $zero
	# set t3 to beginning of array adress
	add $t3, $t1, $zero
	# print
	li $v0, 4
	la $a0, array
	syscall
	# print array elements in loop
	while1:
		# if i == size, end loop
		beq $t2, $t0, end1
		# print element
		li $v0, 1
		lw $a0, 0($t3)
		syscall	
		li $v0, 4
		la $a0, comma
		syscall
		# increase current adress
		addi $t3, $t3, 4
		# increase index
		addi $t2, $t2, 4
		# go back start of loop
		j while1
	end1:
	# print new line
	li $v0, 4
	la $a0, nl
	syscall
	# set array parameter of bubble sort
	add $a0, $t1, $zero
	# set length parameter of bubble sort
	add $a1, $t0, $zero
	# call bubble sort
	jal bubbleSort
	# set t3 to beginning of array adress
	add $t3, $t1, $zero
	# i = 0
	add $t2, $zero, $zero
	# print
	li $v0, 4
	la $a0, sorted
	syscall
	# print sorted array
	while2:
		# if i == size, end loop
		beq $t2, $t0, end2
		# print element
		li $v0, 1
		lw $a0, 0($t3)
		syscall	
		li $v0, 4
		la $a0, comma
		syscall
		addi $t3, $t3, 4
		# increase index
		addi $t2, $t2, 4
		# go back start of loop
		j while2
	end2:
	# print new line
	li $v0, 4
	la $a0, nl
	syscall
	#exit
	li $v0, 10
	syscall
	
#dont use t1 and t0
bubbleSort:
	# i = 0
	add $t2, $zero, $zero
	# j = 0
	add $t3, $zero, $zero
	# create temp value for dont lose address of first element
	add $t4, $a0, $zero
	# t5 = length - 1
	addi $t5, $a1, -4
	
	# while i < length - 1
	loop:
		# if i == length - 1, end
		beq $t2, $t5, endloop
		# lim = (length - 1) - i
		sub $t6, $t5, $t2
		# j = 0
		add $t3, $zero, $zero 
		# set array address to first element
		add $t4, $a0, $zero
		# while j < lim
		loop1:
			# if j == lim, end
			beq $t3, $t6, endloop1
			# if arr[j] > arr[j + 1], swap
			lw $t7, 0($t4)	
			lw $t8, 4($t4)
			bgt $t7, $t8, swap
			back:
			# j++
			addi $t3, $t3, 4
			# current address ++
			addi $t4, $t4, 4
			# go start of loop
			j loop1
		endloop1:
		# i++
		addi $t2, $t2, 4
		# go start of loop
		j loop
	endloop:
	jr $ra
	
swap:	
	# arr[j] == arr[j + 1]
	sw $t8, 0($t4)
	# arr[j + 1] = arr [j]
	sw $t7, 4($t4)
	
	j back
