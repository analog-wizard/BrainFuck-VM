#ifndef EXEC_CONTEXT_H
#define EXEC_CONTEXT_H

typedef struct {
    char instr1 : 4;
    char instr2 : 4;    
} instr_pair_t;

typedef struct {
    char* data;
    unsigned length;
} gen_data_t;

//KMAP: bring it back
//+ : 00101011
//, : 00101100
//- : 00101101
//. : 00101110

//< : 00111100
//> : 00111110

//[ : 01011011
//] : 01011101

//https://en.wikipedia.org/wiki/Brainfuck
//https://esolangs.org/wiki/Brainfuck#Language_overview
/*uint8_t convert_list[8][2] = {
    {'+', 0}, //Increment the byte at the data pointer by 1
    {',', 1}, //Accept DATA_LENGTH bytes of input, stored at the data pointer*
    {'-', 2}, //Decrement the byte at the data pointer by 1
    {'.', 3}, //Output DATA_LENGTH bytes at the data pointer*

    {'<', 4}, //Decrement the data pointer by 1
    {'>', 5}, //Increment the data pointer by 1

    {'[', 6}, //If the byte at the data pointer is zero, jump to next ']', otherwise execute next command
    {']', 7}, //If the byte at the data pointer is NON zero, jump to prev '[', otherwise execute next command
}*/

// Specifications require that each '[' have a ']', but as of present
// I have decided to forgo that formality

// add memory mapped registers that are above some 2^n number such that writes may 
// overflow and write over themselves

// memory mapped registers include
// DATA_LENGTH (32bit) : the number of bytes for any IO operation (',', '.')
// FUNC_CALL   (8-bit) : number of the callback (stored in the VM) to call

void minimize_bf(const char* in, const char* out);
void compile_bf(const char* in, const char* out);

gen_data_t prog_from_file(const char* filename);
void vm_instance(gen_data_t* instr_mem, gen_data_t* data_mem, unsigned delay);

#endif