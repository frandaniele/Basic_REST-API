#include "include/utils.h"

#define PORT 8081

int checkBadNames(char *str){
    return strchr(str, '|') || strchr(str, ' ') || strchr(str, '>') || strchr(str, '<');
}

int callback_get(const struct _u_request * request, struct _u_response * response, void * user_data) {
    (void)request;
    (void)user_data;

    struct group *grupo = getgrnam("api_users");
    struct passwd *user = getpwent();

    json_t *json_list = json_array();

    int count = 0;
    while(user != NULL){
        if(user->pw_gid == grupo->gr_gid){
            count++;

            char *username = user->pw_name;
            json_t *json_users = json_object();

            json_object_set_new(json_users, "user_id", json_integer(count));
            json_object_set_new(json_users, "username", json_string(username));
            json_array_append_new(json_list, json_users);
        }
        user = getpwent();
    }
    endpwent();

    json_t *json_response = json_object();
    json_object_set_new(json_response, "data", json_list);
    
    ulfius_set_json_body_response(response, 200, json_response);
    json_decref(json_list);

    char cant[5];
    sprintf(cant, "%i", count);
    logg("/home/francisco/Facultad/2022SOII/practico/laboratorios/soii---2022---laboratorio-vi-frandaniele/log_api_users", ": usuarios creados -> ", cant);
    
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
    if(checkBadNames(username) || checkBadNames(password)){
        json_object_set_new(json_body, "code", json_integer(400));
        json_object_set_new(json_body, "description", json_string("Usuario o contrasenia no permitidos"));
        ulfius_set_json_body_response(response, 400, json_body);
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
            return U_CALLBACK_COMPLETE;
        }
        user = getpwent();
    }
    endpwent();

   // json_tmp = json_object_get(json_req, "client_address");
   // char *ip = (char *)json_string_value(json_tmp);
   // printf("aaa\n");
   // printf("%s\n", ip);

    struct _u_response response_from_counter;
    struct _u_request request_to_counter;

    if(send_request(&request_to_counter , &response_from_counter, "GET", "http://contadordeusuarios.com/contador/value") == U_CALLBACK_COMPLETE){
        json_object_set_new(json_body, "code", json_integer(404));
        json_object_set_new(json_body, "description", json_string("Counter resource not found"));
        ulfius_set_json_body_response(response, 404, json_body);
        return U_CALLBACK_COMPLETE;
    }
    else{
        json_t *json_resp = ulfius_get_json_body_response(&response_from_counter, NULL);
        if(json_resp == NULL){
            fprintf(stderr, "get json body error\n");
            return 1;
        }

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
        free(cmd);

        if(pclose(cmd_pipe) == -1) error("pclose");

        json_tmp = json_object_get(json_resp, "description");
        json_int_t id = json_integer_value(json_tmp);

        json_object_set_new(json_body, "id", json_integer(id));
        json_object_set_new(json_body, "username", json_string(username));

        json_object_set_new(json_body, "created_at", json_string(get_time()));

        ulfius_set_json_body_response(response, 200, json_body);

        char identificacion[5];
        sprintf(identificacion, "%i", (int)id);
        logg("log_api_users", ": usuario creado, id -> ", identificacion);
    }

    ulfius_clean_request(&request_to_counter);
    ulfius_clean_response(&response_from_counter);

    //incremento contador
    struct _u_response response_from_counter_incr;
    struct _u_request request_to_counter_incr;

    if(send_request(&request_to_counter_incr , &response_from_counter_incr, "POST", "http://contadordeusuarios.com/contador/increment") == U_CALLBACK_COMPLETE){
        json_object_set_new(json_body, "code", json_integer(404));
        json_object_set_new(json_body, "description", json_string("Counter resource not found"));
        ulfius_set_json_body_response(response, 404, json_body);
        return U_CALLBACK_COMPLETE;
    }
    ulfius_clean_request(&request_to_counter_incr);
    ulfius_clean_response(&response_from_counter_incr);

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