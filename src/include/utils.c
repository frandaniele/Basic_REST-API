#include "utils.h"

void error(char *msj){
    perror(msj);
    exit(EXIT_FAILURE);
}

char* get_time(){
    time_t tiempo = time(NULL); 
    if(tiempo < 0) perror("time");

    char *fecha = asctime(gmtime(&tiempo));
    
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