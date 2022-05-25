#include "include/utils.h"

#define PORT 8080 

int callback_print(const struct _u_request * request, struct _u_response * response, void * user_data) {
    (void)request;

    int * counter = (int *) user_data;
   
    char cant[5];
    sprintf(cant, "%i", *counter);
    ulfius_set_json_body_response(response, 200, json_response(200, cant));
    
    return U_CALLBACK_COMPLETE;
}

int callback_increment(const struct _u_request * request, struct _u_response * response, void * user_data){

    json_t *json_req = ulfius_get_json_body_request(request, NULL);
    if(json_req == NULL){ //es decir, le pego desde fuera de laboratorio6.com
        fprintf(stderr, "get json body error\n");

        ulfius_set_json_body_response(response, 403, json_response(403, "Forbidden"));

        logg("/var/log/laboratorio6/counter.log", "Servicio de contador | se intento incrementar desde fuera del servicio lab6", ".");

        return U_CALLBACK_COMPLETE;
    }
    int * counter = (int *) user_data;
    (*counter)++;

    json_t *json_ip = json_object_get(json_req, "ip");
    logg("/var/log/laboratorio6/counter.log", "Servicio de contador | contador incrementado desde -> ip ", (char *)json_string_value(json_ip));
   
    char cant[5];
    sprintf(cant, "%i", *counter);
    ulfius_set_json_body_response(response, 200, json_response(200, cant));

    return U_CALLBACK_COMPLETE;
}

int main(){
    int contador = 0;
    int * ptr = &contador;

    struct group *grupo = getgrnam("api_users");
    struct passwd *user = getpwent();
    while(user != NULL){
        if(user->pw_gid == grupo->gr_gid) contador++;
        user = getpwent();
    }
    endpwent();

    struct _u_instance instance;
    if(ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error ulfius_init_instance, abort\n");
        return(1);
    }

    if(ulfius_add_endpoint_by_val(&instance, "POST", "/contador/increment", NULL, 0, &callback_increment, ptr) != U_OK) error("add endpoint");
    if(ulfius_add_endpoint_by_val(&instance, "GET", "/contador/value", NULL, 0, &callback_print, ptr) != U_OK) error("add endpoint");

    if (ulfius_start_framework(&instance) == U_OK) {
        printf("Se inició el framework en el puerto %d.\n", instance.port);

        while(1);
    } else {
        fprintf(stderr, "Error starting framework\n");
    }

    printf("Saliendo del framework. Adios!\n");

    ulfius_remove_endpoint_by_val(&instance, "GET", "/imprimir", NULL);
    ulfius_remove_endpoint_by_val(&instance, "POST", "/increment", NULL);

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}