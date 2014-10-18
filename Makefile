all:
	gcc happygrep.c -o happygrep -lncursesw

install:
	mv happygrep /bin
	
