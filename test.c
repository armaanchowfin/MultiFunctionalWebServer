// Online C compiler to run C program online
#include <stdio.h>
#include <string.h>

int main() {

    char s[100] = {0};
    fgets(s, sizeof(s), stdin);
    char strtype[100] = {0};
    char args[100] = {0};
    sscanf(s, "%s %[^\n]", strtype,args);
    printf("%s\n", strtype);
    printf("%sx\n", args); //
    printf("hi\n");

    return 0;
}