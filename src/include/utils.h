#include <stdio.h>
#include <ulfius.h>
#include <jansson.h>
#include <sys/types.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <grp.h>
#include <netdb.h>
#include <arpa/inet.h>

void error(char *msj);

char* get_time();

void logg(char *file, char *msj, char *var);

int send_request( struct _u_request * request, struct _u_response * response, char *method, char *url, char* ab_user, char* ab_pass, char* ip);

void exec_cmd(char *cmd);

void *get_in_addr(struct sockaddr *sa);

json_t *json_response(int code, char *description);