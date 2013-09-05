init:
	
compiler:
	ln -sf ../../compiler 	
run: 
	./compiler -Ddumpptonly test.ada
tk: ./compiler
	./compiler -Ddumptkonly test.ada
pt: ./compiler
	./compiler -Ddumpptonly test.ada
ast: compiler 
	./compiler -Ddumpastonly test.ada
verify:
#diff output.txt output.diff
clean:
	rm -f *.o compiler
