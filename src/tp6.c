#include "include/utils.h"

#define PORT 8081

int checkBadNames(char *str){
    return strchr(str, '|') || strchr(str, ' ') || strchr(str, '>') || strchr(str, '<');
}

int callback_get(const struct _u_request * request, struct _u_response * response, void * user_data) {
    (void)request;
    (void)response;
    (void)user_data;

    json_t * json_body = json_object();
    json_object_set_new(json_body, "id", json_integer(2));
    json_object_set_new(json_body, "username", json_string("fran"));
    json_object_set_new(json_body, "created_at", json_string("hoy"));
    json_object_set_new(json_body, "id2", json_integer(1));
    json_object_set_new(json_body, "username2", json_string("asd"));
    json_object_set_new(json_body, "created_at2", json_string("mañana"));
    ulfius_set_json_body_response(response, 200, json_body);

    logg("log_api_users", ": usuarios creados -> ", "5");
    
    return U_CALLBACK_COMPLETE;
}

int callback_post(const struct _u_request * request, struct _u_response * response, void * user_data) {
    (void)response;
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
    if(checkBadNames(username) || checkBadNames(password)){
        json_object_set_new(json_body, "code", json_integer(200));
        json_object_set_new(json_body, "description", json_string("Usuario o contrasenia no permitidos"));
        ulfius_set_json_body_response(response, 200, json_body);
        return U_CALLBACK_COMPLETE;
    }

    struct group *grupo = getgrnam("api_users");
    struct passwd *user = getpwent();

    while(user != NULL){
        if (user->pw_gid == grupo->gr_gid && strcmp(username, user->pw_name) == 0){
            endpwent();
            json_object_set_new(json_body, "code", json_integer(200));
            json_object_set_new(json_body, "description", json_string("El usuario ya existe"));
            ulfius_set_json_body_response(response, 200, json_body);
            return U_CALLBACK_COMPLETE;
        }
        user = getpwent();
    }
    endpwent();

   // json_tmp = json_object_get(json_req, "client_address");
   // char *ip = (char *)json_string_value(json_tmp);
   // printf("aaa\n");
   // printf("%s\n", ip);

    char *uadd = "useradd -g api_users ";
    char *encrypt = " -p $(openssl passwd -1 ";
    char *close = ")";

    size_t str_size = strlen(uadd) + strlen(encrypt) + strlen(close) + strlen(username) + strlen(password);
    char *cmd = malloc(str_size);
    strcpy(cmd, uadd);
    strcat(cmd, username);
    strcat(cmd, encrypt);
    strcat(cmd, password);
    strcat(cmd, close);

    FILE *cmd_pipe = popen(cmd, "w");
    if(cmd_pipe == NULL) error("popen");

    if(pclose(cmd_pipe) == -1) error("pclose");

   // ulfius_send_http_request(const struct _u_request * request, struct _u_response * response);
    //mando el request, recibo la respuesta y decodifico el json
    json_object_set_new(json_body, "id", json_integer(2));
    json_object_set_new(json_body, "username", json_string(username));

    time_t cr_at = time(NULL);
    json_object_set_new(json_body, "created_at", json_string(ctime(&cr_at)));

    ulfius_set_json_body_response(response, 200, json_body);

    logg("log_api_users", ": usuario creado, id -> ", "4");

    free(cmd);

    return U_CALLBACK_COMPLETE;
}

int main()
{
    struct _u_instance instance;

    if(ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error ulfius_init_instance, abort\n");
        return(1);
    }

    if(ulfius_add_endpoint_by_val(&instance, "POST", "/api/users", NULL, 0, &callback_post, NULL) != U_OK) error("add endpoint");
    if(ulfius_add_endpoint_by_val(&instance, "GET", "/api/users", NULL, 0, &callback_get, NULL) != U_OK) error("add endpoint");

    if (ulfius_start_framework(&instance) == U_OK) {
        printf("Se inició el framework en el puerto %d.\n", instance.port);
        printf("Pulsa enter para salir.\n");

        getchar();
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