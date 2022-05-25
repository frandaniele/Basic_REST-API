#include "include/utils.h"

#define PORT 8081

int checkBadNames(char *str){
    int chars = strchr(str, '|') || strchr(str, ' ') || strchr(str, '>') || strchr(str, '<'); //check bad chars

    int len = (strlen(str) < 8) || (strlen(str) > 24); //check length beetwen 8 and 24

    return chars || len;
}

int callback_get(const struct _u_request * request, struct _u_response * response, void * user_data) {
    (void)user_data;

    json_t *json_response = json_object();

    //obtengo contador
    struct _u_response response_from_counter;
    struct _u_request request_to_counter;

    if(send_request(&request_to_counter , &response_from_counter, "GET", "http://contadordeusuarios.com/contador/value", request->auth_basic_user, request->auth_basic_password, NULL) == U_CALLBACK_ERROR){
        json_object_set_new(json_response, "code", json_integer(404));
        json_object_set_new(json_response, "description", json_string("Counter resource not found"));
        ulfius_set_json_body_response(response, 404, json_response);

        ulfius_clean_request(&request_to_counter);
        ulfius_clean_response(&response_from_counter);

        logg("/var/log/laboratorio6/users.log", "Servicio de usuarios | servicio de contador no disponible ", ".");

        return U_CALLBACK_COMPLETE;
    }

    struct group *grupo = getgrnam("api_users");
    struct passwd *user = getpwent();

    json_t *json_list = json_array();

    while(user != NULL){
        if(user->pw_gid == grupo->gr_gid){
            char *username = user->pw_name;
            unsigned int id = user->pw_uid;
            json_t *json_users = json_object();

            json_object_set_new(json_users, "user_id", json_integer(id));
            json_object_set_new(json_users, "username", json_string(username));
            json_array_append_new(json_list, json_users);
        }
        user = getpwent();
    }
    endpwent();

    json_object_set_new(json_response, "data", json_list);
    
    ulfius_set_json_body_response(response, 200, json_response);
    json_decref(json_list);

    json_t *json_resp = ulfius_get_json_body_response(&response_from_counter, NULL);
    if(json_resp == NULL){
        fprintf(stderr, "get json body error\n");
        return 1;
    }

    //todo ok, preparo la respuesta
    json_int_t count = json_integer_value(json_object_get(json_resp, "description"));
    char cant[5];
    sprintf(cant, "%lli", count);
    logg("/var/log/laboratorio6/users.log", "Servicio de usuarios | usuarios creados -> ", cant);

    ulfius_clean_request(&request_to_counter);
    ulfius_clean_response(&response_from_counter);
    
    return U_CALLBACK_COMPLETE;
}

int callback_post(const struct _u_request * request, struct _u_response * response, void * user_data) {
    (void)user_data;

    json_t * json_body = json_object();
    json_t *json_tmp = NULL;
    json_t *json_req = ulfius_get_json_body_request(request, NULL);
    if(json_req == NULL){
        fprintf(stderr, "get json body error\n");
        return 1;
    }

    json_tmp = json_object_get(json_req, "username");
    char *username = (char *)json_string_value(json_tmp);
    json_tmp = json_object_get(json_req, "password");
    char *password = (char *)json_string_value(json_tmp);

    //CHEQUEAR BADNAMES + USUARIO REPETIDO
    if(username == NULL || password == NULL || checkBadNames(username) || checkBadNames(password)){
        json_object_set_new(json_body, "code", json_integer(400));
        json_object_set_new(json_body, "description", json_string("Usuario o contrasenia no permitidos"));
        ulfius_set_json_body_response(response, 400, json_body);
        
        logg("/var/log/laboratorio6/users.log", "Servicio de usuarios | Usuario o contrasenia no permitidos", ".");

        return U_CALLBACK_COMPLETE;
    }

    struct group *grupo = getgrnam("api_users");
    struct passwd *user = getpwent();
    while(user != NULL){
        if (user->pw_gid == grupo->gr_gid && strcmp(username, user->pw_name) == 0){
            endpwent();

            json_object_set_new(json_body, "code", json_integer(409));
            json_object_set_new(json_body, "description", json_string("El usuario ya existe"));
            ulfius_set_json_body_response(response, 409, json_body);

            logg("/var/log/laboratorio6/users.log", "Servicio de usuarios | Usuario repetido", ".");

            return U_CALLBACK_COMPLETE;
        }
        user = getpwent();
    }
    endpwent();

   //incremento contador
    char ip[INET6_ADDRSTRLEN];
    inet_ntop(request->client_address->sa_family, get_in_addr(request->client_address), ip, sizeof(ip));

    struct _u_response resp_from_incr;
    struct _u_request req_to_incr;

    if(send_request(&req_to_incr , &resp_from_incr, "POST", "http://contadordeusuarios.com/contador/increment", request->auth_basic_user, request->auth_basic_password, ip) == U_CALLBACK_ERROR){
        json_object_set_new(json_body, "code", json_integer(404));
        json_object_set_new(json_body, "description", json_string("Counter resource not found"));
        ulfius_set_json_body_response(response, 404, json_body);

        ulfius_clean_request(&req_to_incr);
        ulfius_clean_response(&resp_from_incr);

        logg("/var/log/laboratorio6/users.log", "Servicio de usuarios | Servicio de contador no disponible", ".");

        return U_CALLBACK_COMPLETE;
    }

    //agrego user
    char *uadd = "sudo useradd -g api_users ";
    char *encrypt = " -p $(openssl passwd -1 ";
    char *close = ")";

    size_t str_size = strlen(uadd) + strlen(encrypt) + strlen(close) + strlen(username) + strlen(password);
    char *cmd = malloc(str_size);
    strcpy(cmd, uadd);
    strcat(cmd, username);
    strcat(cmd, encrypt);
    strcat(cmd, password);
    strcat(cmd, close);

    exec_cmd(cmd);
    free(cmd);

    //lo agrego para que pueda usar estos servicios
    char *echo = "echo ";
    char *htpasswd = "sudo htpasswd -i /etc/nginx/.htpasswd ";
    char *pipe = " | ";

    char *pass = malloc(strlen(echo) + strlen(password) + strlen(pipe) + strlen(htpasswd) + strlen(username));
    strcpy(pass, echo);
    strcat(pass, password);
    strcat(pass, pipe);
    strcat(pass, htpasswd);
    strcat(pass, username);

    exec_cmd(pass);
    free(pass);

    struct passwd *p;
    if((p = getpwnam(username)) == NULL) {
        perror("getpwnam");
        return EXIT_FAILURE;
    }
    
    json_object_set_new(json_body, "id", json_integer(p->pw_uid));
    json_object_set_new(json_body, "username", json_string(username));
    json_object_set_new(json_body, "created_at", json_string(get_time()));

    ulfius_set_json_body_response(response, 200, json_body);

    ulfius_clean_request(&req_to_incr);
    ulfius_clean_response(&resp_from_incr);

    char identificacion[5];
    sprintf(identificacion, "%i", (int) p->pw_uid);
    logg("/var/log/laboratorio6/users.log", "Servicio de usuarios | usuario creado, id -> ", identificacion);

    return U_CALLBACK_COMPLETE;
}

int main(){
    struct _u_instance instance;

    if(ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error ulfius_init_instance, abort\n");
        return(1);
    }

    if(ulfius_add_endpoint_by_val(&instance, "POST", "/api/users", NULL, 0, &callback_post, NULL) != U_OK) error("add endpoint");
    if(ulfius_add_endpoint_by_val(&instance, "GET", "/api/users", NULL, 0, &callback_get, NULL) != U_OK) error("add endpoint");

    if (ulfius_start_framework(&instance) == U_OK) {
        printf("Se inició el framework en el puerto %d.\n", instance.port);
        
        while(1);
    } else {
        fprintf(stderr, "Error starting framework\n");
    }

    printf("Saliendo del framework. Adios!\n");

    ulfius_remove_endpoint_by_val(&instance, "GET", "/api/users", NULL);
    ulfius_remove_endpoint_by_val(&instance, "POST", "/api/users", NULL);

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}