#include "exec_context.hpp"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

    using namespace brainfuck;

    gen::gen_data_t prog_mem = minimize_bf("./main.bf");
    gen::gen_data_t data_mem = {new void*[32,768], 1<<16};
    vm_t vm = vm();
    vm(&prog_mem, &data_mem);
    free(prog_mem.data);
    free(data_mem.data);

    return 0;
}