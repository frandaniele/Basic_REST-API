CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wconversion -pedantic -std=gnu11
ULFIUS = -lulfius

all: build_folders regs counter

regs: regs.o utils.o
	$(CC) $(CFLAGS) -o src/bin/regs src/obj/regs.o src/obj/utils.o $(ULFIUS)

regs.o: src/tp6.c 
	$(CC) $(CFLAGS) -c src/tp6.c -o src/obj/regs.o

counter: counter.o utils.o 
	$(CC) $(CFLAGS) -o src/bin/counter src/obj/counter.o src/obj/utils.o $(ULFIUS)

counter.o: src/user_count.c 
	$(CC) $(CFLAGS) -c src/user_count.c -o src/obj/counter.o

utils.o: src/include/utils.c
	$(CC) $(CFLAGS) -c src/include/utils.c -o src/obj/utils.o

build_folders:
	mkdir -p ./src/obj ./src/bin 

cppcheck:
	cppcheck --enable=all --suppress=missingIncludeSystem src/ 2>err.txt

clean:
	rm -f -r ./src/bin ./src/obj /src/bin/* /src/obj/*