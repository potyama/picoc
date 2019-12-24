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

	pushl	-1
	pushi	1
	eq
	jf	.L0
	pushi	1
	storel	-2
	mvsp	1
	jp	.L1
.L0:
.L1:
	pushl	-2
	storel	2
	leave
	ret
	mvsp	2
	leave
	ret
