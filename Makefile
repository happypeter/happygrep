all:
	gcc happygrep.c -o happygrep -lncursesw  -Wall

install:
	mv happygrep /bin
	
