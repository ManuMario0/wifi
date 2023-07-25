//
//  graph.h
//  empty
//
//  Created by Emmanuel Mera on 08/07/2023.
//

#ifndef graph_h
#define graph_h

enum {
    KER_STATIC,
    KER_DYNAMIC
};

typedef struct {
    void *  data;
    
} KER_node;

typedef struct {
    char    type;
    
    long        size;
    KER_node *  node;
    
    float *     weights;
} KER_graph;

typedef struct {
    long    dim;
    long    size;
    
    float * mat;
} KER_graph_proj;

#endif /* graph_h */
