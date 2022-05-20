/etc/nginx/nginx.conf

include /etc/nginx/modules-enabled/*.conf;

events {
  worker_connections  1024;
}
http {
 server {
    listen 80;
    server_name laboratorio6.com www.laboratorio6.com;

    location / {
        auth_basic 		"Restricted Content";
        auth_basic_user_file	/etc/nginx/.htpasswd;
    }
    
    location /api/users {
        auth_basic 		"Restricted Content";
        auth_basic_user_file	/etc/nginx/.htpasswd;
        proxy_set_header 	X-Real-IP $remote_addr;
        proxy_pass         	http://localhost:8081/api/users;
    }
}

server {
    listen 80;
    server_name contadordeusuarios.com www.contadordeusuarios.com;

    location /contador/increment {
        auth_basic "Restricted Content";
        auth_basic_user_file /etc/nginx/.htpasswd;
        proxy_pass         http://localhost:8080/contador/increment;   
    }
    
    location /contador/value {
        auth_basic "Restricted Content";
        auth_basic_user_file /etc/nginx/.htpasswd;
        proxy_pass         http://localhost:8080/contador/value;   
    }
}

server {
  listen      80 default_server;
  server_name _;
  
  return 404 '{"error": {"status_code": 404,"status": "Not Found"}}';
}
	access_log /var/log/nginx/access.log;
	error_log /var/log/nginx/error.log;
}

------------------------------------------------

/etc/sudoers.d/admin_users
User_Alias  USERADMIN = admin_users

USERADMIN ALL = NOPASSWD: /sbin/useradd

------------------------------------------------

/etc/systemd/system/counterlab.service

[Unit]
Description=servicio de contador de usuarios

Wants=network.target
After=syslog.target network-online.target

[Service]
Type=simple
User=admin_users
ExecStart=/home/francisco/Facultad/2022SOII/practico/laboratorios/soii---2022---laboratorio-vi-frandaniele/src/bin/counter
Restart=on-failure
RestartSec=10
KillMode=process

[Install]
WantedBy=multi-user.target

------------------------------------------------

/etc/systemd/system/lab6.service 

[Unit]
Description=creador de usuarios

Wants=network.target
After=syslog.target network-online.target

[Service]
Type=simple
User=admin_users
ExecStart=/home/francisco/Facultad/2022SOII/practico/laboratorios/soii---2022---laboratorio-vi-frandaniele/src/bin/regs
Restart=on-failure
RestartSec=10
KillMode=process

[Install]
WantedBy=multi-user.target