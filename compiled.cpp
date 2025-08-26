#include "virtual_machine.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <iostream>
#include <ostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <cstring>

brainfuck::gen_data_t brainfuck::gen::minimize_bf(const char* filename) {
    gen_data_t ret_val = {NULL, 0};

    if(filename == NULL) {
        return ret_val;
    }

    std::ifstream fd;
    fd.open(filename);
    if(!fd.is_open()) {
        std::cout << "FAILURE TO OPEN FILE\n";
        return ret_val;
    }

    //Set the stream pointer to the end of the stream
    fd.seekg(0, fd.end);
    //Take the position of the stream pointer as an integer, which is the length
    std::size_t file_length = fd.tellg();
    //Set the stream pointer back ot the beginning of the file.
    fd.seekg(0, fd.beg);

    char* file_buf = (char*)std::malloc(file_length);
    fd.read(file_buf, file_length);

    fd.close();

    std::size_t offset = 0;
    std::size_t actual = 0;

    while(actual < file_length) {
        switch (file_buf[actual]) {
            case '+':
            case '-':
            case '.':
            case ',':
            case '<':
            case '>':
            case '[':
            case ']':
            case '~':
            case '(':
            case ')':
            case 'v':
            case '^':
            case '@':
                file_buf[offset] = file_buf[actual];
                offset++;
                break;

            case ';':
                while(file_buf[actual] != '\n' && actual < file_length) {
                    actual++;
                }
            default:
                break;
        }
        actual++;
    }

    ret_val.data = (char*)std::malloc(offset);
    std::memcpy(ret_val.data, file_buf, offset);
    ret_val.length = offset;

    delete file_buf;

    return ret_val;
}

brainfuck::gen_data_t brainfuck::gen::minimize_bf(const char* in, const char* out) {
    gen_data_t data = minimize_bf(in);
    std::ofstream fd(out);
    fd.write(data.data, data.length);
    fd.close();
    return data;
}

void brainfuck::gen::compile_bf(const char* in, const char* out, std::basic_ostream<char>& errout) {
    gen_data_t data = minimize_bf(in, std::string(out).append(".min.bf").c_str());

    // New Language features
    // **   checks for 'v', '^', and '@' may be ignored if there is no '()'
    //      pairS, if there is a lone ')', consider it an early return and
    //      end program execution
    // **   If there is a lone ']', and should requirements be satisfied, then
    //      it should match to the nearest '[' (and for '[', ']') such that
    //          ... [ ... [ ... ] ...
    //              D     E     F
    //      D -> F (if conditions met)
    //      E <-> F (if conditions met)
    // **   'anti-loops': such that
    //          'B' ] ... [ 'A'
    //      where 'A' and 'B' may only contain matched '[]' sets
    //      The above code works the same as a normal loop, but the 
    //      conditions for looping are reversed. Unmatched ']' or '[' 
    //      are permitted in the inner loop such that
    //          'B' ] ... ] ... [ 'A'
    //              D     E     F
    //      D -> F (if conditions met)
    //      E <-> F (if conditions met)
    // **   If there is a lone ')', it should match to the nearest '(' 
    //      (and for '(', ')') such that
    //          ... ( ... ( ... ) ...
    //              D     E     F
    //      D -> F (if conditions met)
    //      E -> F (if conditions met)
    // **   'anti-time': such that
    //          'B' ) ... ( 'A'
    //      where 'A' and 'B' may only contain matched '()' sets
    //      Unmatched ')' or '(' are permitted in the inner loop such that
    //          'B' ) ... ) ... ( 'A'
    //              D     E     F
    //      D -> F (if conditions met)
    //      E -> F (if conditions met)

    // Altered Language Features
    // **   '@' now acts as a 'thread.join()' for 'lower timelines', and will
    //      freeze indefinitely until the 'lower timeline' exits execution.
    //      When a 'lower timeline' completes, n bytes (determined by 'DATA_WIDTH'
    //      of the 'lower timeline') are loaded into the input buffer of the 'parent
    //      timeline' ONLY IF the 'lower timeline' is being waited on with '@'
    // **   '.' now may act as a C-Style function call when the data pointer
    //      resides over 'FUNC_POINTER' memory mapped register, the VM will
    //      attempt to call this function with specified parameters, defined
    //      by the current location of the data pointer(s) on the next 
    //      execution of '.'. '#include <link.h> #include <dlfcn.h>' 
    //      (on POSIX systems) are already preincluded, and it is up to the 
    //      user to use the functions within to include their desired libraries
    // **   After a function call, ',' (and manipulation of 'DATA_WIDTH')
    //      will write 'DATA_WIDTH' bytes into data memory from the function
    //      output, after which, it will resume its normal operation of 
    //      taking user input one character at a time. Successive function
    //      calls will overwrite previous data in the internal buffer
    // **   There is no longer synchronization between the 'timelines', 
    //      synchronization practices and efforts are now the responsibility
    //      of the user. (multi 'timeline' instructions, I.E. '^', 'v', and '@'
    //      will be atomic and still safe to use)
    // **   ',' has the ability to overlap itself, should 'DATA_WIDTH' be large
    //      enough, such that if there was a 5 byte data structure written
    //      to the first byte of data memory and the data memory segment
    //      was only 4 bytes long, then the first byte of data memory would
    //      be overwritten by the 5th byte of the structure.
    // **   any 'v' operation without a 'lower timeline' will discard all 
    //      EXTRA data pointers, but will still retain the 'prime' pointer
    //      the same applies for '^'
    // **   any '^' or 'v' operation will inherit the 'DATA_WIDTH' of the parent
    //      'timeline' even if that timeline expires, this is so parameters of
    //      different lengths may be passed to any called functions

    // New features:
    // **   whole line comments with ';'

    // General optimizations:
    // **   ... ( ... )
    //      where ')' is the last instruction, this '()' set should not create
    //      a new 'timeline', and should instead be optimized out
    // **   single '[', ']', or '(' should be optimized out
    // **   '~' is a compiler directive which, at compile time, will be
    //      translated to the opposite of the most recent data memory
    //      modifying set of instructions
    // **   any matching set of '<' and '>' where both increment and
    //      decrement are equal or '+' and '-' where the net result is 0
    //      Examples:
    //          >>><<<
    //          +++---
    //          >+>+>+-<-<-<
    // **   memory size for new 'dummy' threads which only serve the 'parent'
    //      'timeline' may be shrunk and only select sections of the 'parent'
    //      memory (which are accessed by the program) are available
    // **   seperation into different functions for applicable sections of
    //      code, like 'timelines' which only perform a very small set of
    //      instructions, or a set of several instructions that are repeated 
    //      often
    // **   replace 'noodling' of the 'FUNC_POINTER' register and replace
    //      with final value if possible
    // **   loops optimization is as defined in the 'loop' lambda

    // The following return/input types are prohibited (per SYS-V ABI):
    // **   SSE
    // **   SSEUP
    // **   X87/X87UP/COMPLEX_X87 (FP)
    // The following return/input types are allowed:
    // **   INTEGER
    // **   MEMORY
    auto function = []() {
        //https://wiki.osdev.org/System_V_ABI#x86-64
        /*asm volatile
        *   Type recognition from symbol table
        *   load first 6 parameters into registers:
            if return type INTEGER:
                rdi, rsi, rdx, rcx, r8, and r9: INTEGER parameters
            if return type MEMORY:
                rdi: address of the function output buffer
                rsi, rdx, rcx, r8, and r9: INTEGER parameters
            and 'prime' data pointer will point to the starting location of the
            data memory which should be copied to the stack for additional 
            function parameters (the amount to be copied to be determined by
            'DATA_WIDTH' of the calling function)
        *   call function (from '%r11')
        *   check '%rax' for the return value (hopefully not '%rdx')
        */
    };

    auto loop = [&]() {
        // for a loop to be optimized to a constant expression:
        // **   the general program should not have any '()' pairs, 'v' or , '^'
        //      such that '[]' pairs are affected
        // **   The loop in question's continuation should not depend
        //      on user input nor any value from memory mapped registers
        // **   The loop in question has no effect on any of the 
        //      memory cells or memory pointers:
        //          loops such as:
        //              [@] (without any '()', 'v', '^' instructions present)
        //              [<< X >>] : such that any set of instructions 'X' has 
        //                  no net effect on the data pointer position or the
        //                  final value of the 'determining' data cell
        //              [++ 'X' --] : same reasoning as above
        //              [++ 'X' ~~] : same reasoning as above
        //              [~] : when executed before any ',', '+', or '-'
        // **   If at any point a nested loop cannot be optimized for
        //      any of the reasons listed above, then the current loop
        //      may not be either

        auto inner_loop = [&](auto&& inner) -> bool {
            std::size_t cell_inc;
            std::size_t cell_dec;

            std::size_t net_effect;

            bool optimized = false;

            for(std::size_t i = 0; i < data.length && data.data[i] != ']'; i++) {
                // until end of loop
                // if nested loop
                if(data.data[i] == ',') {
                    return false;
                }

                if(!inner(inner)) {
                    //return false;
                }
                // if nested loop may be optimized,
                // do so
            }
            //if this loop may be optimized, do so
            return true;
        };

        inner_loop(inner_loop);
    };
}

void brainfuck::vm_t::compil_vm() {
    //mmap and jump in
}

void brainfuck::vm_t::operator()() {
    if(!(prog_mem.data && data_mem.data)) {
        return;
    }
    if(*((uint32_t*)prog_mem.data) == 0x7f454c46) { //Compiled
        compil_vm();
    } else { //Interpreted
        interp_vm();
    }
};

void brainfuck::vm_t::operator()(gen_data_t* instr_mem, gen_data_t* data_mem) {
    std::memcpy(this->prog_mem.data, instr_mem->data, std::min(instr_mem->length, this->prog_mem.length));
    std::memcpy(this->data_mem.data, data_mem->data, std::min(data_mem->length, this->data_mem.length));
    this->operator()();
};

//static const char* debug_symbols = "+,-.<>[]~()v^@";