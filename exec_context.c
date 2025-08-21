#include "exec_context.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

gen_data_t prog_from_file(const char* filename) {
    gen_data_t ret_val = {NULL, 0};

    if(filename == NULL) {
        return ret_val;
    }

    int fd = open(filename, O_RDONLY);
    if((int)fd == -1) {
        printf("FAILURE TO OPEN FILE\n");
        exit(1);
    }

    struct stat st;
    fstat(fd, &st);
    unsigned file_length = st.st_size;

    char* file_buf = malloc(file_length);
    read(fd, file_buf, file_length);

    close(fd);

    //Overkill, but guarentees adequate length
    ret_val.data = malloc((file_length/2) + 1);
    memset(ret_val.data, 0, (file_length/2) + 1);

    unsigned offset = 0;
    unsigned actual = 0;

    char toggle_instr = 0;

    while(actual < file_length) {
        char data = 0;
        char data_written = 1;
        switch (file_buf[actual]) {
            case '+':
            case '-':
            case '.':
            case ',':
                data = file_buf[actual] - '+';
                break;

            case '<':
                data = 4;
                break;
            case '>':
                data = 5;
                break;

            case '[':
                data = 6;
                break;
            case ']':
                data = 7;
                break;

            case ';':
                while(file_buf[actual] != '\n' && actual < file_length) {
                    actual++;
                }
            default:
                data_written = 0;
        }

        if(data_written) {
            //printf("actual : %d : %c : offset : %d : %d%d\n", actual, file_buf[actual], offset, ((instr_pair_t*)ret_val.data)[offset].instr1, ((instr_pair_t*)ret_val.data)[offset].instr2);
            if(toggle_instr != 0) {
                ((instr_pair_t*)ret_val.data)[offset].instr2 = data;
                offset++;
                toggle_instr = 0;
            } else {
                ((instr_pair_t*)ret_val.data)[offset].instr1 = data;
                toggle_instr = 1;
            }
        }
        actual++;
    }

    ret_val.length = offset*2 + 1;
    free(file_buf);

    return ret_val;
}

static const char* debug_symbols = "+,-.<>[]";

void vm_instance(gen_data_t* instr_mem, gen_data_t* data_mem, unsigned delay) {

    unsigned instr_ptr = 0;
    unsigned data_ptr = 0;

    gen_data_t data_out;
    gen_data_t data_in;

    while(instr_ptr <= instr_mem->length) {
        char data = data_mem->data[data_ptr];
        instr_pair_t pair = ((instr_pair_t*)instr_mem->data)[instr_ptr/2];
        char instr = (instr_ptr%2 ? pair.instr2 : pair.instr1);

        //printf("%u : %c | %u : %d \n", instr_ptr + 1, debug_symbols[instr], data_ptr, data);

        switch(instr) {
            case 0: //+
                data_mem->data[data_ptr]++;
                break;
            case 1: //,
                if(data_mem->data[data_mem->length-1] == 0) {
                    data_mem->data[data_ptr] = (char)getc(stdin);
                } else {
                    unsigned x = data_ptr;
                    for(unsigned i = 0; (i < data_in.length) && (i < ((unsigned*)data_mem)[(data_mem->length)]); i++, x = (x+1) % data_mem->length) {
                        data_mem->data[x] = data_in.data[i];
                    }
                }
                break;
            case 2: //-
                data_mem->data[data_ptr]--;
                break;
            case 3: //.
                if(data_ptr == (data_mem->length-1)) {
                    switch(data) {
                        case 0:
                            printf("AHAHAHHAHAHA\n");
                            break;
                        case 1:
                            break;
                        default:
                            printf("default\n");
                            return;
                    }
                } else {
                    printf("%c", data_mem->data[data_ptr]);
                }
                break;
            case 4:
                data_ptr = (data_ptr-1) % data_mem->length;
                break;
            case 5:
                data_ptr = (data_ptr+1) % data_mem->length;
                break;
            case 6:
                if(data == 0) {
                    unsigned unmatched = 0;
                    while(1) {
                        instr_ptr++;
                        pair = ((instr_pair_t*)instr_mem->data)[instr_ptr/2];
                        instr = (instr_ptr%2 ? pair.instr2 : pair.instr1);
                        if(instr == 6) {
                            unmatched++;
                        } else if (instr == 7) {
                            if(unmatched == 0) {
                                break;
                            }
                            unmatched--;
                        }
                    }
                }
                break;
            case 7:
                if(data != 0) {
                    unsigned unmatched = 0;
                    while(1) {
                        instr_ptr--;
                        pair = ((instr_pair_t*)instr_mem->data)[instr_ptr/2];
                        instr = (instr_ptr%2 ? pair.instr2 : pair.instr1);
                        if(instr == 7) {
                            unmatched++;
                        } else if (instr == 6) {
                            if(unmatched == 0) {
                                break;
                            }
                            unmatched--;
                        }
                    }
                    break;
                }
                break;
        }
        instr_ptr++;
        sleep(delay);
    }
}