#ifndef EXEC_CONTEXT_H
#define EXEC_CONTEXT_H

#include <iostream>
#include <ostream>

namespace brainfuck {
    struct gen_data_t {
        char* data;
        std::size_t length;
    };
    
    namespace gen {
        gen_data_t  minimize_bf(const char* in);
        void        minimize_bf(const char* in, const char* out);

        void        compile_bf (const char* in, const char* out, std::basic_ostream<char>& errout);
    }

    class vm_t {
        gen_data_t prog_mem = {nullptr, 0};
        gen_data_t data_mem = {nullptr, 0};

        void interp_vm();
        void compil_vm();

        public:
            vm_t() = default;

            void operator()();
            void operator()(gen_data_t* instr_mem, gen_data_t* data_mem);
    };
}

//https://en.wikipedia.org/wiki/Brainfuck
//https://esolangs.org/wiki/Brainfuck#Language_overview

// add memory mapped registers that are above some 2^n number such that writes may 
// overflow and write over themselves

// memory mapped registers include
// DATA_LENGTH (32bit) : the number of bytes for any IO operation (',', '.')
// FUNC_CALL   (sizeof(void*) bytes) : number of the callback (stored in the VM) to call
#endif