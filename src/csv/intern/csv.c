//
//  csv.c
//  empty
//
//  Created by Emmanuel Mera on 05/06/2023.
//

#include <stdio.h>

#include "csv.h"
#include "MEM_alloc.h"

void parse_csv(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char c;
    int column = 1, row = 1;
    while ((c=fgetc(fp)) != '\n') {
        if (c == ',') column ++;
    }
    
    fseek(fp, 0, SEEK_SET);
    while ((c=fgetc(fp)) != EOF) {
        if (c == '\n') row ++;
    }
    
    MEM_calloc_array(row, column, __func__);
    
}
