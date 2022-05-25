CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wconversion -pedantic -std=gnu11
ULFIUS = -lulfius
JANSSON = -ljansson

main: build_folders regs counter

setup_server:
#	nginx config
	cp ./nginx/nginx.conf /etc/nginx/
	cp ./nginx/.htpasswd /etc/nginx/
	chown admin_users /etc/nginx/.htpasswd
	service nginx reload
	service nginx start

#	creacion de servicios
	cp ./servicios/counterlab.service /etc/systemd/system/
	cp ./servicios/lab6.service /etc/systemd/system/
	
#	creacion de logs y permisos para modificarlos
	mkdir /var/log/laboratorio6
	chown root:adm laboratorio6/
	touch /var/log/laboratorio6/users.log
	touch /var/log/laboratorio6/counter.log
	chown admin_users:adm /var/log/laboratorio6/users.log 
	chown admin_users:adm /var/log/laboratorio6/counter.log 
	chmod 640 /var/log/laboratorio6/users.log 
	chmod 640 /var/log/laboratorio6/counter.log 

#	configuracion de logrotate
	cp ./log/laboratorio6 /etc/logrotate.d/laboratorio6/

	systemctl daemon-reload

enable_server:
	systemctl enable counterlab.service
	systemctl enable lab6.service

disable_server:
	systemctl disable counterlab.service
	systemctl disable lab6.service

reload_server:
	service counterlab reload
	service lab6 reload

restart_server:
	service counterlab restart
	service lab6 restart
	
start_server:
	service counterlab start
	service lab6 start	
	
stop_server:
	service counterlab stop
	service lab6 stop	

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
	stop_server
	disable_server
	rm -f -r /src/bin/* /src/obj/* ./src/bin ./src/obj  
	rm -f -r /var/log/laboratorio6/* ./var/log/laboratorio6
	rm -f /etc/logrotate.d/laboratorio6  
	rm -f /etc/systemd/system/counterlab.service /etc/systemd/system/lab6.service
	rm -f /etc/nginx/.htpasswd /etc/nginx/nginx.conf
	systemctl daemon-reload