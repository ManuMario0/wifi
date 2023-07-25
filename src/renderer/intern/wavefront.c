//
//  wavefront.c
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 23/02/2023.
//

#include "wavefront.h"
#include "MEM_alloc.h"

#include <stdlib.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobjloader/tinyobj_loader.h"

void file_reader(void* ctx, const char* filename, const int is_mtl, const char* obj_filename, char** data, size_t* len) {
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *buff = MEM_calloc_array(*len, sizeof(char), __func__);
    
    fread(buff, sizeof(char), *len, f);
    
    void **ppctx = ctx;
    *ppctx = buff;
    *data = buff;
    
    fclose(f);
}

void acquireData(float **data, uint16_t **indicies, int *dataSize, int *indexCount, char filename[]) {
    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes;
    tinyobj_material_t* materials = NULL;
    size_t num_materials;
    
    void *ctx;
    
    tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, filename, file_reader, (void*)&ctx, 0);
    MEM_free(ctx);
    
    /* THEIR WILL BE MEMORY LEAKS FOR NOW */
    // TODO: rewrite all the code my self, will be more efficient for my purposes
    
    float       *pdata = MEM_calloc_array(attrib.num_faces, 6*sizeof(float), __func__); // we will always use triangular faces in the storage files
    uint16_t    *pindicies = MEM_calloc_array(attrib.num_faces, sizeof(uint16_t), __func__);
    for (int i=0; i<attrib.num_face_num_verts; i++) {
        assert(attrib.face_num_verts[i] == 3);
        for (int j=0; j<3; j++) {
            pindicies[i*3+j] = i*3+j;
            for (int k=0; k<3; k++) {
                pdata[i*18+j*6+k] = attrib.vertices[3*attrib.faces[3*i+j].v_idx+k];
                pdata[i*18+j*6+3+k] = attrib.normals[3*attrib.faces[3*i+j].vn_idx+k];
            }
        }
    }
    
    *data = pdata;
    *indicies = pindicies;
    *indexCount = attrib.num_face_num_verts*3;
    *dataSize = attrib.num_face_num_verts*3*6*sizeof(float);
    
    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);
}
