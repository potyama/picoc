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
	pushi	1
	mvsp	1
	leave
	ret
