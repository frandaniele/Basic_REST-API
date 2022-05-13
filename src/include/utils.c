#include "utils.h"

void error(char *msj){
    perror(msj);
    exit(EXIT_FAILURE);
}