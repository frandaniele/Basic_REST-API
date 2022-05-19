#include "include/utils.h"

#define PORT 8080 

int callback_print(const struct _u_request * request, struct _u_response * response, void * user_data) {
    (void)request;
    (void)response;

    int * counter = (int *) user_data;
   
    json_t * json_body = json_object();
    json_object_set_new(json_body, "code", json_integer(200));
    json_object_set_new(json_body, "description", json_integer(*counter));
    ulfius_set_json_body_response(response, 200, json_body);
    
    return U_CALLBACK_COMPLETE;
}

int callback_increment(const struct _u_request * request, struct _u_response * response, void * user_data) {
    (void)request;
    (void)response;
    
    int * counter = (int *) user_data;
    (*counter)++;

    logg("log_user_count", ": contador incrementado desde -> ip ", "127.0.0.1");

    json_t * json_body = json_object();
    json_object_set_new(json_body, "code", json_integer(200));
    json_object_set_new(json_body, "description", json_integer(*counter));
    ulfius_set_json_body_response(response, 200, json_body);

    return U_CALLBACK_COMPLETE;
}

int main()
{
    struct _u_instance instance;

    int contador = 0;
    int * ptr = &contador;

    if(ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error ulfius_init_instance, abort\n");
        return(1);
    }

    if(ulfius_add_endpoint_by_val(&instance, "POST", "/contador/increment", NULL, 0, &callback_increment, ptr) != U_OK) error("add endpoint");
    if(ulfius_add_endpoint_by_val(&instance, "GET", "/contador/value", NULL, 0, &callback_print, ptr) != U_OK) error("add endpoint");

    if (ulfius_start_framework(&instance) == U_OK) {
        printf("Se inició el framework en el puerto %d.\n", instance.port);
        printf("Pulsa enter para salir.\n");

        getchar();
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