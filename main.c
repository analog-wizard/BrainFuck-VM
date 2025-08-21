#include "exec_context.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

    gen_data_t prog_mem = prog_from_file("./main.min.bf");
    gen_data_t data_mem = {malloc(1<<16), 1<<16};
    vm_instance(&prog_mem, &data_mem, 0);
    free(data_mem.data);

    return 0;
}