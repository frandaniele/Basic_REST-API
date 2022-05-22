CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wconversion -pedantic -std=gnu11
ULFIUS = -lulfius
JANSSON = -ljansson

main: build_folders regs counter

start_server:
#	nginx config
	cp /nginx/nginx.conf /etc/nginx/

#	creacion de servicios
	cp /servicios/counterlab.service /etc/systemd/system/
	cp /servicios/lab6.service /etc/systemd/system/
	
#	creacion de logs y permisos para modificarlos
	sudo mkdir /var/log/laboratorio6
	sudo chown root:adm laboratorio6/
	sudo touch /var/log/laboratorio6/users.log
	sudo touch /var/log/laboratorio6/counter.log
	sudo chown admin_users:adm /var/log/laboratorio6/users.log 
	sudo chown admin_users:adm /var/log/laboratorio6/counter.log 
	sudo chmod 775 /var/log/laboratorio6/users.log 
	sudo chmod 775 /var/log/laboratorio6/counter.log 
	cp /log/laboratorio6 /etc/logrotate.d/laboratorio6/

regs: regs.o utils.o
	$(CC) $(CFLAGS) -o src/bin/regs src/obj/regs.o src/obj/utils.o $(ULFIUS) $(JANSSON)

regs.o: src/tp6.c 
	$(CC) $(CFLAGS) -c src/tp6.c -o src/obj/regs.o

counter: counter.o utils.o 
	$(CC) $(CFLAGS) -o src/bin/counter src/obj/counter.o src/obj/utils.o $(ULFIUS) $(JANSSON)

counter.o: src/user_count.c 
	$(CC) $(CFLAGS) -c src/user_count.c -o src/obj/counter.o

utils.o: src/include/utils.c
	$(CC) $(CFLAGS) -c src/include/utils.c -o src/obj/utils.o 

build_folders:
	mkdir -p ./src/obj ./src/bin 

cppcheck:
	cppcheck --enable=all --suppress=missingIncludeSystem src/ 2>err.txt

clean:
	rm -f -r /src/bin/* /src/obj/* ./src/bin ./src/obj