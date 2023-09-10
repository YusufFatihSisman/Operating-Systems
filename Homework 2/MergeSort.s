.data
arr: .word 1, 12, 0, -3, 99, 48, -17, -9, 20, 15, 44, 13, -4, 30, 70, 20, 22, 18, 15, 2
size: .word 20
thread: .word 4
part: .word 0
strNl: .asciiz "\n"
strSpace: .asciiz " "

.text
.globl main
main:
	add $t0, $zero, $zero	# t0 = 0
	la $t1, arr		# t1 = address of arr
	lw $t2, size		# t2 = size of arr
	print_while:
		bge $t0, $t2, end_print_while
		lw $a0, 0($t1)		# a0 = *arr
		li $v0, 1
		syscall
		la $a0, strSpace
		li $v0, 4
		syscall
		addi $t1, $t1, 4	# *(arr++)
		addi $t0, $t0, 1	# t0 = t0 + 1
		j print_while
	end_print_while:
	
	la $a0, strNl
	li $v0, 4
	syscall

	add $t0, $zero, $zero	# t0 = 0
	lw $t1, thread
	add $t2, $t1, $t1
	add $t2, $t2, $t2

	add $a0, $t2, $zero
	li $v0, 9
	syscall
	add $s0, $v0, $zero		# s0 = address of thread id array

	add $t2, $s0, $zero		# t2 = s0

	while:
		bge $t0, $t1, endWhile
		addi $t0, $t0, 1
		la $a0, merge_sort_thread
		li $v0, 19
		syscall
		sw $v0, 0($t2)
		# call CREATE_THREAD_SYSCALL
		addi $t2, $t2, 4
		j while
	endWhile:

	add $t2, $s0, $zero
	add $t0, $zero, $zero

	while2:
		lw $t1, thread
		bge $t0, $t1, endWhile2
		addi $t0, $t0, 1
		# call JOIN_THREAD_SYSCALL
		lw $a0, 0($t2)
		li $v0, 20
		syscall
		addi $t2, $t2, 4
		j while2
	endWhile2:	

	lw $t0, size

	add $a0, $zero, $zero 	# a0 = 0
	sra $a2, $t0, 1 	# a2 = size / 2
	addi $a2, $a2, -1	# a2 = size / 2 - 1	
	sra $a1, $a2, 1 	# a1 = (size / 2 - 1) / 2 
	jal merge

	lw $t0, size

	sra $a0, $t0, 1		# a0 = size / 2
	add $a2, $t0, -1	# a2 = size - 1
	add $a1, $a2, $a0	# a1 = size - 1 - size / 2
	sra $a1, $a1, 1		# a1 = (size - 1 - size / 2) / 2
	jal merge

	lw $t0, size

	add $a0, $zero, $zero	# a0 = 0
	add $a2, $t0, -1	# t0 = size - 1
	sra $a1, $a2, 1		# a1 = (size - 1) / 2
	jal merge
	
	add $t0, $zero, $zero	# t0 = 0
	la $t1, arr		# t1 = address of arr
	lw $t2, size		# t2 = size of arr
	print_sorted_while:
		bge $t0, $t2, end_print_sorted_while
		lw $a0, 0($t1)		# a0 = *arr
		li $v0, 1
		syscall
		la $a0, strSpace
		li $v0, 4
		syscall
		addi $t1, $t1, 4	# *(arr++)
		addi $t0, $t0, 1	# t0 = t0 + 1
		j print_sorted_while
	end_print_sorted_while:
	
	la $a0, strNl
	li $v0, 4
	syscall
	
	# CALL PROCESS_EXIT SYSCALL
	li $v0, 25
	syscall
				
merge:
	sub $t0, $a1, $a0	# t0 = mid - low
	addi $t0, $t0, 1	# t0 = mid - low + 1 (n1)
	sub $t1, $a2, $a1	# t1 = high - mid (n2)
	
	add $t4, $a0, $zero	# t4 = a0

	# initialize the left array
	add $a0, $t0, $t0
	add $a0, $a0, $a0
	li $v0, 9
	syscall
	add $s1, $v0, $zero 	# s1 = address of left array[0]
	
	add $t2, $s1, $zero	# t2 = s1
	
	# initialize the right array
	add $a0, $t1, $t1
	add $a0, $a0, $a0
	li $v0, 9
	syscall
	add $s2, $v0, $zero 	# s2 = address of right array[0]
	
	add $t3, $s2, $zero	# t3 = s2
	
	add $a0, $t4, $zero	# backup a0
	
	add $t4, $zero, $zero	# t4 = 0
	
	la $t5, arr		# t5 = address of arr
	add $t6, $a0, $a0
	add $t6, $t6, $t6	# index += low
	add $t5, $t5, $t6	# t5 = address of arr[low]
	fill_left_while:
		bge $t4, $t0, end_fill_left_while	# if i >= n1, end
		lw $t6, 0($t5)		# t6 = arr[i + low]
		sw $t6, 0($t2)		# left[i] = arr[i + low]		
		addi $t2, $t2, 4	# left++
		addi $t5, $t5, 4	# arr++
		addi $t4, $t4, 1	# t4 += 1
		j fill_left_while
	end_fill_left_while:

	add $t4, $zero, $zero	# t4 = 0
	
	add $t6, $a1, $a1
	add $t6, $t6, $t6	# t6 = index mid
	la $t5, arr		# t5 = address of arr
	add $t5, $t5, $t6	# t5 = address of arr[mid]
	addi $t5, $t5, 4	# t5 = address of arr[mid + 1]
	fill_right_while:
		bge $t4, $t1, end_fill_right_while	# if i >= n2, end
		lw $t6, 0($t5)		# t6 = arr[i + mid + 1]
		sw $t6, 0($t3)		# right[i] = arr[i + mid + 1]
		addi $t3, $t3, 4	# right++
		addi $t5, $t5, 4	# arr++
		addi $t4, $t4, 1	# t4 += 1
		j fill_right_while
	end_fill_right_while:	
	
	la $t6, arr		# t6 = address of arr
	add $t4, $a0, $a0	
	add $t4, $t4, $t4	# t4 = 4 x low
	add $t6, $t6, $t4	# t6 = address of arr[k]
	
	add $t2, $s1, $zero	# t2 = address of left[0]
	add $t3, $s2, $zero	# t3 = address of right[0]
		
	add $t4, $zero, $zero	# t4 = 0
	add $t5, $zero, $zero	# t5 = 0
	merge_while:
		bge $t4, $t0, end_merge_while
		bge $t5, $t1, end_merge_while
		lw $t7, 0($t2)		# t7 = left[i]
		lw $t8, 0($t3)		# t8 = right[j] 
		bgt $t7, $t8, else	# left[i] > right[j]
		if:
			sw $t7, 0($t6)		# arr[k] = left[i]
			addi $t2, $t2, 4	# left++
			addi $t6, $t6, 4	# arr++
			addi $t4, $t4, 1	# i++
			j merge_while
		else:
			sw $t8, 0($t6)		# arr[k] = right[j]
			addi $t3, $t3, 4	# right++
			addi $t6, $t6, 4	# arr++
			addi $t5, $t5, 1	# j++
			j merge_while
	end_merge_while:
	
	last_left_while:
		bge $t4, $t0, end_last_left_while
		lw $t7, 0($t2)		# t7 = left[i]
		sw $t7, 0($t6)		# arr[k] = left[i]
		addi $t2, $t2, 4	# left++
		addi $t6, $t6, 4	# arr++
		addi $t4, $t4, 1	# i++
		j last_left_while
	end_last_left_while:
	
	last_right_while:
		bge $t5, $t1, end_last_right_while
		lw $t8, 0($t3)		# t8 = right[j] 
		sw $t8, 0($t6)		# arr[k] = right[j]
		addi $t3, $t3, 4	# right++
		addi $t6, $t6, 4	# arr++
		addi $t5, $t5, 1	# j++
		j last_right_while
	end_last_right_while:
	
	jr $ra

merge_sort:
	addi $sp, $sp, -16
	sw $a0, 0($sp)		# a0 = low
	sw $a1, 8($sp)		# a1 = high
	sw $ra, 12($sp)

	sub $t0, $a1, $a0	# t0 = high - low
	sra $t0, $t0, 1		# t0 = (high - low) / 2
	add $s3, $t0, $a0	# s3 = mid = low + (high - low) / 2
	
	sw $s3, 4($sp)

	bge $a0, $a1, exitMergeSort	# if low >= high, exit
	
	add $a0, $a0, $zero	# a0 = low
	add $a1, $s3, $zero	# a1 = mid
	jal merge_sort
	
	lw $s3, 4($sp)
	lw $a1, 8($sp)
	addi $a0, $s3, 1	# a0 = mid + 1
	add $a1, $a1, $zero	# a1 = high
	jal merge_sort

	lw $a0, 0($sp)		# a0 = low
	lw $a1, 4($sp)		# a1 = mid
	lw $a2, 8($sp)		# a2 = high
	jal merge
	
	j exitMergeSort

exitMergeSort:
    lw $a0, 0($sp)
    lw $s3, 4($sp)
    lw $a1, 8($sp)
    lw $ra, 12($sp)     	# read registers from stack
    addi $sp, $sp, 16    	# bring back stack pointer
    jr $ra

merge_sort_thread:

	lw $t0, part		# t0 = part
	addi $t1, $t0, 1	# t1 = part + 1
	la $t2, part		# t2 = address of part
	sw $t1, 0($t2)		# part = part + 1
	
	lw $t3, size		# t3 = size	
	sra $t3, $t3, 2		# t3 = size / 4
	
	mult $t0, $t3		# part * (size / 4)
	mflo $s4		# s4 = low
	
	addi $t0, $t0, 1	# t0 = part + 1
	
	mult $t0, $t3		# (part + 1) * (size / 4)
	mflo $t2		# t2 = (part + 1) * (size / 4)
	
	addi $s5, $t2, -1	# s5 = high
	
	sub $t3, $s5, $s4	# t3 = high - low
	sra $t3, $t3, 1		# t3 = (high - low) / 2
	add $s6, $t3, $s4	# s6 = mid
	
	
	bge $s4, $s5, endthread
	
	# merge_sort(low, mid)
	add $a0, $s4, $zero
	add $a1, $s6, $zero
	jal merge_sort
	
	# merge_sort(mid + 1, high)
	addi $a0, $s6, 1
	add $a1, $s5, $zero
	jal merge_sort
	
	# merge(low, mid, high)
	add $a0, $s4, $zero
	add $a1, $s6, $zero
	add $a2, $s5, $zero
	jal merge
	
	endthread:
		# call THREAD_EXIT_SYSCALL
		li $v0, 21
		syscall

