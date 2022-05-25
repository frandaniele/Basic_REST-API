### Lab6 Sistemas Operativos II - Francisco Daniele
## Ingeniería en Compuatación - FCEFyN - UNC
# Sistemas Embebidos

## Desarrollo
```text
├── log                 -> archivo de configuracion de logrotate
├── nginx               -> archivos para configuracion de nginx
├── servicios           -> archivos de configuracion de systemD para los servicios
├── src
│    ├── bin            -> archivos binarios
│    ├── include        -> libreria con funciones utilizadas en el laboratorio
│    ├── obj            -> archivo objeto
│    ├── tp6.c          -> source code del servicio de usuarios
│    └── user_count.c   -> source code del servicio de contador de usuarios
├── .gitignore
├── LICENSE
├── makefile            -> genera todo lo necesario para el desarrollo del trabajo
└── README.md           -> informe del trabajo realizado
```

## Setup
Para preparar el entorno del sistema se deben seguir los siguientes pasos:
-   creación del usuario _admin_users_, perteneciente al grupo _adm_, quien se encargará de correr los servicios
``` Bash
sudo useradd -g adm admin_users -p $(openssl passwd -1 <contrasenia>)
```
-   modifico el archivo _/etc/sudoers_ para dar permisos de ejecución de los comandos necesarios al usuario _admin_users agregando las siguientes líneas:
``` Bash
Cmnd_Alias USERAPI_COMMANDS = /usr/sbin/useradd, /usr/bin/htpasswd, /usr/bin/openssl

admin_users	ALL=(ALL) NOPASSWD: USERAPI_COMMANDS
```
-   make para generar los directorios y binarios necesarios, y
-   sudo make setup_server para crear los directorios y archivos pertinentes a la configuracion del server: logs, nginx y de los servicios de usuario y contador. **WARNING**: si ya existe una configuración de _nginx_, será sobreescrita.
``` Bash
make
sudo make setup_server
```
-   asociamos la IP del host con el nombre del servidor, incluyendo las siguientes líneas en el archivo /etc/hosts
``` Bash
<IP local>	contadordeusuarios.com
<IP local>	laboratorio6.com
```

## Ejecucion
Para la ejecución de los servicios seguimos los siguientes pasos:
-   Habilitamos los servicios
``` Bash
sudo make enable_server
#sudo make disable_server para deshabilitar
```
-   hacemos un reload de los servicios
``` Bash
sudo make reload_server
```
-   los iniciamos
``` Bash
sudo make start_server
```

Además podemos hacer un seguimiento de los servicios con los siguientes comandos:
``` Bash
journalctl -f -o cat _SYSTEMD_UNIT=lab6.service
journalctl -f -o cat _SYSTEMD_UNIT=counterlab.service
```
## Consultas
### Manuales

-   Nuevo usuario

curl --request POST \
            --url http://laboratorio6.com/api/users \
            -u admin:fran \
            --header 'accept: application/json' \
            --header 'content-type: application/json' \
            --data '{"username": "myuser10", "password": "mypassword"}'

-   Listar usuarios

curl --request GET \
        --url http://laboratorio6.com/api/users \
        -u admin:fran \
        --header 'accept: application/json' \
        --header 'content-type: application/json'

-   Incrementar contador

curl --request POST \
            --url http://contadordeusuarios.com/contador/increment \
            -u admin:fran \
            --header 'accept: application/json' \
            --header 'content-type: application/json' 

-   Obtener contador

curl --request GET \
            --url http://contadordeusuarios.com/contador/value \
            -u admin:fran \
            --header 'accept: application/json' \
            --header 'content-type: application/json' 

### Postman
Para facilitar la interacción con el servidor, se realizó un documento de consultas en _Postman_ para probar las distintas funcionalidades del mismo. Una vez configurado e iniciado el servidor, se puede cargar dicho archivo en _Postman Desktop_ para poder trabajar con _localhost_ y realizar las consultas que se quieran.