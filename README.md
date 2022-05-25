### Lab6 Sistemas Operativos II - Francisco Daniele
## Ingeniería en Compuatación - FCEFyN - UNC
# Sistemas Embebidos

## Desarrollo
```text
├── log                 -> archivo de configuracion de logrotate
├── nginx               -> archivos para configuracion de nginx
├── postman             -> coleccion de consultas para Postman
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

## Funcionamiento de los servicios
### Usuarios
Este servicio cuenta con los dos endpoints pedidos por la consigna e inicia el framework en el puerto 8081.

Cuando se le consulta para listar los usuarios creados, actúa de la siguiente manera:

-      Le hace una request al servicio de contador mediante _send_request_ que es una funcion propia, wrapper de _ulfius_send_http_request_ con control de errores y seteo de las propiedades de la request. Si dicho servicio no está disponible se envia el mensaje de error y se lo loguea, de lo contrario se obtiene la cantidad de usuarios que se han creado.
-      A través de _getpwent_ obtiene los usuarios creados por el servicio y los mete en una lista JSON
-      Prepara la respuesta en formato JSON
-      Escribe en el log del servicio y retorna con exito.

Para responder al pedido de crear un usuario procede así:

-       Obtiene el nombre de usuario y la contrasenia del body de la request
-       Si alguno de los dos está vacío, contiene menos de 8 caracteres o mas de 24 o caracteres prohibidos, se retorna un error y se loguea la situacion
-       Se chequea que no exista el usuario, de lo contario se envia el error pertinente y se loguea
-       Se obtiene la ip del cliente y se la envia al servicio del contador con la request para incrementarlo. Si no se encuentra disponible se retorna el error y se loguea.
-       Se prepara el comando _useradd_ y se encripta la contrasenia mediante _openssl_
-       Se ejecuta el comando mediante _exec_cmd_ que es un wrapper de _popen_ con control de errores
-       Se prepara el comando _htpasswd_ para poder autorizarlo al uso de estos servicios mediante nginx
-       Se obtiene el _uid_ del usuario mediante _getpwnam_
-       Se setea la respuesta pertinente en formato JSON
-       Se loguea la respuesta y se retorna correctamente

### Contador
Este servicio cuenta con los dos endpoints pedidos por la consigna e inicia el framework en el puerto 8080.
Al iniciar obtiene la cantidad de usuarios creados mediante _getpwent_ e inicia el contador del servicio en ese valor.

Responde a la consulta de obtener contador de la siguiente manera:

-       Mediante un puntero al contador obtiene su valor 
-       Setea la respuesta en formato JSON
-       Retorna con exito

Cuando debe incrementar el contador realiza lo siguiente:

-       Chequea que ha sido consultado desde el servicio _lab6_, de lo contrario responde con un error y loguea la situacion
-       Obtiene la ip del cliente que crea el usuario desde la request del servicio lab6
-       Incrementa el contador
-       Escribe el log del servicio
-       Setea la respuesta en formato JSON y retorna correctamente

## Consultas
### Manuales

-   Nuevo usuario
``` Bash
curl --request POST \
            --url http://laboratorio6.com/api/users \
            -u admin:fran \
            --header 'accept: application/json' \
            --header 'content-type: application/json' \
            --data '{"username": "myuser10", "password": "mypassword"}'
```
-   Listar usuarios
``` Bash
curl --request GET \
        --url http://laboratorio6.com/api/users \
        -u admin:fran \
        --header 'accept: application/json' \
        --header 'content-type: application/json'
```
-   Incrementar contador
``` Bash
curl --request POST \
            --url http://contadordeusuarios.com/contador/increment \
            -u admin:fran \
            --header 'accept: application/json' \
            --header 'content-type: application/json' 
```
-   Obtener contador
``` Bash
curl --request GET \
            --url http://contadordeusuarios.com/contador/value \
            -u admin:fran \
            --header 'accept: application/json' \
            --header 'content-type: application/json' 
```
### Postman
Para facilitar la interacción con el servidor, se realizó un documento de consultas en _Postman_ para probar las distintas funcionalidades del mismo. Una vez configurado e iniciado el servidor, se puede cargar dicho archivo en _Postman Desktop_ para poder trabajar con _localhost_ y realizar las consultas que se quieran.