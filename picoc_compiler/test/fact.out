#Picoc09 2011-12-13
__start__:
	call	main
	halt
#組込み関数 read()
read:
	enter
	rd
	storel	2
	leave
	ret
#組込み関数 write()
write:
	enter
	pushl	3
	wr
	leave
	ret
#組込み関数 writeln()
writeln:
	enter
	wrln
	leave
	ret
main:
	enter
	mvsp	-2
	mvsp	-1
	call	read
	storel	-1
	mvsp	1
	pushl	-1
	mvsp	-1
	call	fact
	storet	1
	mvsp	1
	storel	-2
	mvsp	1
	pushl	-2
	mvsp	-1
	call	write
	storet	1
	mvsp	1
	mvsp	1
	mvsp	-1
	call	writeln
	mvsp	1
	mvsp	2
	leave
	ret
fact:
	enter
	pushl	3
	pushi	0
	lt
	jf	.L0
	pushi	0
	storel	2
	leave
	ret
	jp	.L1
.L0:
	pushl	3
	pushi	1
	le
	jf	.L2
	pushi	1
	storel	2
	leave
	ret
	jp	.L3
.L2:
	pushl	3
	pushl	3
	pushi	1
	sub
	mvsp	-1
	call	fact
	storet	1
	mvsp	1
	mul
	storel	2
	leave
	ret
.L3:
.L1:
	leave
	ret
