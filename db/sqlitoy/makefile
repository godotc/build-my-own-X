

.PHONY: cfg

cfg:
	xmake f -c
	xmake f -m debug 
	xmake project -k compile_commands

db: *.cc
	g++ *.cc  -ggdb -O0 -o db
	./db
