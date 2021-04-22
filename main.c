#include <stdio.h>
#include "main.h"
#include "log/log.h"
#include "keyValStore.h"



int main() {
    char hallo[10];

    put("12b3","Irgenein1");
    get("12b3", *hallo);
    printf("Hallo");
    printf("%s", hallo);
    return 0;
}
