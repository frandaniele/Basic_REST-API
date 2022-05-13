#include "include/utils.h"

int main(int argc, char* argv[])
{
    if(argc != 2) error("Usage: main <port>\n");

    printf("%s\n", argv[0]);

    return 0;
}