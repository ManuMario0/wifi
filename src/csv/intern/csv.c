//
//  csv.c
//  empty
//
//  Created by Emmanuel Mera on 05/06/2023.
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "csv.h"
#include "MEM_alloc.h"

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

static long acquire_raw_cell(char *current_line, long pos, char buffer[]);
static char **add_item(char **memory, size_t load, size_t *_mem_size, char *key);

/* ---------------------------- */
/*  implementation              */
/* ---------------------------- */

long CSV_get_column_count(FILE *fp) {
    if (!fp)
        return -1;
    
    fseek(fp, 0, SEEK_SET);
    
    char c;
    long column = 1;
    while ((c=fgetc(fp)) != '\n') {
        if (c == ',') column ++;
    }
    
    return column;
}

long CSV_get_row_count(FILE *fp) {
    if (!fp)
        return -1;
    
    fseek(fp, 0, SEEK_SET);
    
    long row = 1;
    char buff[4096];
    while (fgets(buff, 4096, fp) != NULL) {
        row ++;
    }
    
    return row;
}

CSV_file *CSV_parse(char file_name[], int *flags, int flags_length) {
    FILE *fp = fopen(file_name, "r");
    
    long column = CSV_get_column_count(fp);
    char buff[2048];
    
    if (flags_length < column)
        return NULL;
    
    CSV_file *f = MEM_malloc(sizeof(CSV_file), __func__);
    f->row_count = CSV_get_row_count(fp);
    f->column_count = 0;
    f->file_name = MEM_malloc(strlen(file_name)+1, __func__);
    memcpy(f->file_name, file_name, strlen(file_name)+1);
    
    for (int i=0; i<column; i++) {
        if (flags[i] != CSV_SKIP) {
            f->column_count ++;
        }
    }
    
    f->head = MEM_malloc_array(sizeof(char*), f->column_count, __func__);
    f->types = MEM_calloc_array(sizeof(CSV_type), f->column_count, __func__);
    fseek(fp, 0, SEEK_SET);
    
    long index = 0;
    long pos = 0;
    char current_line[4096];
    fgets(current_line, 4096, fp);
    for (int i=0; i<column; i++) {
        if (flags[i] != CSV_SKIP) {
            pos = acquire_raw_cell(current_line, pos, buff);
            f->head[index] = MEM_malloc(strlen(buff)+1, __func__);
            memcpy(f->head[index], buff, strlen(buff)+1);
            f->types[index].type = flags[i];
            if (flags[i] == CSV_AGGLO) {
                f->types[index].tbl = KER_hash_create_table(8192);
                f->types[index].item_count = 0;
            }
            index++;
        } else {
            pos = acquire_raw_cell(current_line, pos, buff);
        }
    }
    
    size_t *reverse_tbl_size = MEM_calloc_array(sizeof(size_t), f->column_count, __func__);
    long *reverse_tbl_load = MEM_calloc_array(sizeof(long), f->column_count, __func__);
    for (long i=0; i<f->column_count; i++) {
        reverse_tbl_size[i] = 64;
        f->types[i].reverse_tbl = MEM_calloc_array(sizeof(char *), 64, __func__);
    }
    f->data = MEM_malloc_array(sizeof(CSV_cell), f->column_count*f->row_count, __func__);
    for (int i=0; i<f->row_count; i++) {
        fgets(current_line, 4096, fp);
        pos = 0;
        CSV_cell *cell = &f->data[i*f->column_count];
        index = 0;
        for (int j=0; j<column; j++) {
            pos = acquire_raw_cell(current_line, pos, buff);
            switch (flags[j]) {
                case CSV_LONG:
                    cell[index].l = atol(buff);
                    index++;
                    break;
                    
                case CSV_HEX:
                    ;unsigned int res;
                    sscanf(buff, "%x", &res);
                    cell[index].l = res;
                    index++;
                    break;
                    
                case CSV_FLOAT:
                    cell[index].l = atof(buff);
                    MEM_free(buff);
                    index++;
                    break;
                    
                case CSV_STR:
                    cell[index].str = MEM_malloc(strlen(buff)+1, __func__);
                    memcpy(cell[index].str, buff, strlen(buff)+1);
                    index++;
                    break;
                    
                case CSV_AGGLO:
                    if (!KER_hash_find(f->types[index].tbl, buff, (int)strlen(buff))) {
                        f->types[index].reverse_tbl = add_item(f->types[index].reverse_tbl, f->types[index].item_count, &reverse_tbl_size[index], buff);
                        f->types[index].item_count++;
                        KER_hash_add(f->types[index].tbl, buff, (int)strlen(buff), (void*)f->types[index].item_count);
                    }
                    cell[index].l = (long)KER_hash_find(f->types[index].tbl, buff, (int)strlen(buff));
                    index++;
                    break;
                    
                case CSV_DATE:
                    ;int empty;
                    cell[index].date = MEM_malloc(sizeof(CSV_date), __func__);
                    sscanf(buff, "%d-%d-%dT%d:%d:%d.%dZ", &cell[index].date->year, &cell[index].date->month, &cell[index].date->day, &cell[index].date->hour, &cell[index].date->min, &cell[index].date->sec, &empty);
                    index ++;
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    fclose(fp);
    return f;
}

CSV_cell *CSV_get_row(CSV_file *f, long row) {
    return &f->data[row*f->column_count];
}

char *CSV_reverse_id(CSV_file *f, long column, long uid) {
    return f->types[column].reverse_tbl[uid-1];
}

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

long acquire_raw_cell(char *current_line, long pos, char buffer[]) {
    long length = 0;
    if (current_line[pos] != '\"') {
        while (current_line[pos] != ',' && current_line[pos] != '\0' && length < 2047) {
            buffer[length] = current_line[pos];
            length++;
            pos++;
        }
        if (current_line[pos] != ',' && current_line[pos] != '\0') {
            while (current_line[pos] != ',' && current_line[pos] != '\0') {pos++;}
        }
        buffer[length] = '\0';
    } else {
        pos++;
        while (current_line[pos] != '\"' && length < 2047) {
            buffer[length] = current_line[pos];
            length++;
            pos++;
        }
        if (current_line[pos] != '\"') {
            while (current_line[pos] != ',' && current_line[pos] != '\0') {pos++;}
        }
        pos++;
        buffer[length] = '\0';
    }
    
    return ++pos;
}

char **add_item(char **memory, size_t load, size_t *_mem_size, char *key) {
    size_t mem_size = *_mem_size;
    load++;
    if (load > mem_size) {
        mem_size *= 2;
        memory = MEM_realloc(memory, mem_size*sizeof(char *));
    }
    memory[load-1] = MEM_malloc(strlen(key)+1, __func__);
    memcpy(memory[load-1], key, strlen(key)+1);
    
    *_mem_size = mem_size;
    
    return memory;
}
