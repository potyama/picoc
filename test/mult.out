#Picoc09 2011-12-13
__start__:
	call	main
	halt
#組込み関数 read()
#ここに read() の機械語を書く
#組込み関数 write()
#ここに write() の機械語を書く
#組込み関数 writeln()
writeln:
	enter
	wrln
	leave
	ret
main:
	enter
	mvsp	-3
	mvsp	-1
	call	read
	storel	-1
	mvsp	1
	mvsp	-1
	call	read
	storel	-2
	mvsp	1
	mul
	storel	-3
	mvsp	1
	mvsp	-1
	call	write
	storet	1
	mvsp	1
	mvsp	1
	mvsp	-1
	call	writeln
	mvsp	1
	mvsp	3
	leave
	ret
