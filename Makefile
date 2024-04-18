
all: 
	gcc -fno-common *.c -o emul

clean:
	rm emul
