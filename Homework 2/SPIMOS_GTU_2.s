.data
filename: .asciiz "ProducerConsumer.s "
filename2: .asciiz "ProducerConsumerWithoutMutex.s "

.text
.globl main
main:
	li $v0, 18
	la $a0, filename
	syscall
