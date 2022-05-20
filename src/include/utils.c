#include "utils.h"

void error(char *msj){
    perror(msj);
    exit(EXIT_FAILURE);
}

char* get_time(){
    time_t tiempo = time(NULL); 
    if(tiempo < 0) perror("time");

    char *fecha = ctime(&tiempo);
    
    if(fecha == NULL) perror("time");
    else fecha[strlen(fecha)-1] = '\0'; 

    return fecha;
}

void logg(char *file, char *msj, char *var){
    FILE *fptr;

    if((fptr = fopen(file, "a+")) == NULL) error("log file");

    char *tiempo = get_time();
    char *str = calloc((strlen(tiempo) + strlen(msj) + strlen(var)), sizeof(char));
    strcpy(str, tiempo);
    strcat(str, msj);
    strcat(str, var);
    fprintf(fptr, "%s\n", str);
    fclose(fptr);
    free(str);
}

int send_request( struct _u_request * request, struct _u_response * response, char *method, char *url){

    ulfius_init_request(request);
    ulfius_set_request_properties(request,
                                    U_OPT_HTTP_VERB, method,
                                    U_OPT_HTTP_URL, url,
                                    U_OPT_NETWORK_TYPE, U_USE_ALL,
                                   // U_OPT_AUTH_BASIC_USER, , //obtener esto de la request a este servicio
                                   // U_OPT_AUTH_BASIC_PASSWORD, , 
                                    U_OPT_TIMEOUT, 20,
                                    U_OPT_HEADER_PARAMETER, "Content-Type", "application/json",
                                    U_OPT_HEADER_PARAMETER, "Accept", "application/json",
                                    U_OPT_NONE); // Required to close the parameters list

    ulfius_init_response(response);
    
    if(ulfius_send_http_request(request, response) != U_OK){
        fprintf(stderr, "Error in http request, no valido etc etc\n");
    
        ulfius_clean_request(request);
        ulfius_clean_response(response);
        
        return U_CALLBACK_COMPLETE;
        //popen("systemctl restart counterlab")?
    }

    return U_CALLBACK_IGNORE;
}