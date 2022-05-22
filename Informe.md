creacion de admin_users, agregado a grupo adm

------------------------------------------------

/etc/sudoers
Cmnd_Alias USERAPI_COMMANDS = /usr/sbin/useradd, /usr/bin/htpasswd, /usr/bin/openssl

# User privilege specification
root	ALL=(ALL:ALL) ALL
admin_users	ALL=(ALL) NOPASSWD: USERAPI_COMMANDS

------------------------------------------------
/etc/systemd/system/counterlab.service

/etc/systemd/system/lab6.service 
-----------------------------------------------------------------------
inspeccionar servicios
journalctl -f -o cat _SYSTEMD_UNIT=lab6.service
journalctl -f -o cat _SYSTEMD_UNIT=counterlab.service

------------------------------------------
crear el archivo .htpasswd y el usuario admin

-----------------------------------------------