//
//  graph.c
//  empty
//
//  Created by Emmanuel Mera on 08/07/2023.
//

#include <string.h>
#include <assert.h>

#include "MEM_alloc.h"

#include "graph.h"

#define MIN(a, b) ((a < b) ? (a) : (b))

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

static float *compute_dist(KER_graph *g, long dim, float *x);
static void compute_gradient(KER_graph *g, long dim, float *grad, float *x);

/* ---------------------------- */
/*  implementation              */
/* ---------------------------- */

KER_graph *KER_create_graph(long size, char type) {
    KER_graph *g = MEM_calloc(sizeof(KER_graph), __func__);
    
    g->size = (type == KER_STATIC) ? size : 0;
    g->type = type;
    
    return g;
}

void KER_floyd_warshall(KER_graph *g) {
#ifdef DEBUG
    assert(g->type == KER_STATIC);
#endif
    
    for (long i=0; i<g->size; i++) {
        for (long j=0; j<g->size; j++) {
            for (long k=0; k<g->size; k++) {
                g->weights[i*g->size + j] = MIN(g->weights[i*g->size + k], g->weights[i*g->size + k] + g->weights[k*g->size + j]);
            }
        }
    }
}

float KER_get_distance(KER_graph *g, long a, long b) {
    if (g->type == KER_STATIC) {
        if (a < g->size && b < g->size) {
            return g->weights[a*g->size+b];
        }
    } else {
        ;
    }
    return 0.;
}

long KER_add_node(KER_graph *g, void *data) {
    return 0;
}

void KER_add_edge(KER_graph *g, long a, long b, float weight) {
    return;
}

void KER_set_weight(KER_graph *g, long a, long b, float weight) {
    if (g->type == KER_STATIC) {
        if (a < g->size && b < g->size) {
            g->weights[a*g->size+b] = weight;
        }
    } else {
        ;
    }
}

void KER_unsafe_set_weight(KER_graph *g, long a, long b, float weight) {
    if (g->type == KER_STATIC) {
        g->weights[a*g->size+b] = weight;
    } else {
        ;
    }
}

void KER_to_static(KER_graph *g) {
    return;
}

void KER_to_dynamic(KER_graph *g) {
    return;
}

KER_graph_proj *KER_project_graph(KER_graph *g, long dim) {
    KER_graph_proj *gp =  MEM_calloc(sizeof(KER_graph_proj), __func__);
    
    gp->dim = dim;
    gp->size = g->size;
    
    
    
    
    
    
    return gp;
}

float **KER_graph_fft(KER_graph *g, long *input, long size, long dim) {
    return NULL;
}

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

/*
 We are trying to find a solution to the following non-linear system of equations :
 || e_1 - e_2 || - d(1, 2) = 0
 || e_1 - e_3 || - d(1, 3) = 0
 ....
 || e_1 - e_n || - d(1, n) = 0
 || e_2 - e_3 || - d(2, 3) = 0
 ....
 
 Where e_n in |R^dim.
 
 For that, we introduce x =
 a_1    a_2     ....    a_n
 b_1    b_2     ....    b_n
 ...
 dim_1  dim_2   ....    dim_n
 
 And G(x) =
 || e_1 - e_2 || - d(1, 2) = (a_1 - a_2)^2 + .... (dim_1 - dim_2)^2 - d(1, 2)
 || e_1 - e_3 || - d(1, 3)
 ....
 || e_1 - e_n || - d(1, n) = 0
 || e_2 - e_3 || - d(2, 3) = 0
 ....
 
 and we try to find the minimum of the function F(x) -> G(x)^T*G(x).
 For that we'll use the gradient descent method with some optimization.
 Because F is strongly connexe, the speed of convergence is O(log(1/e)) where
 e is the disired precision. The overall algorithm run in O(log(1/e)*dim*n^2).
 */


float *compute_dist(KER_graph *g, long dim, float *x) {
    float *dist = MEM_calloc_array(sizeof(float), dim*g->size, __func__);
    
    long index = 0;
    for (long i=0; i<g->size; i++) {
        for (long j=i+1; j<g->size; j++) {
            // compute distance between e_i and e_j
            for (long k=0; k<dim; k++) {
                dist[index] += (x[k*g->size+i] - x[k*g->size+j])*(x[k*g->size+i] - x[k*g->size+j]);
            }
            dist[index] -= KER_get_distance(g, i, j);
            index++;
        }
    }
    
    return dist;
}

/*
 Instead of actually computing the gradient (which would mostly be empty),
 I'm gonna dirrectly compute the product with G(x)
 */

void compute_gradient(KER_graph *g, long dim, float *grad, float *x) {
    float *f = compute_dist(g, dim, x);
    
    memset(grad, 0, dim*g->size*sizeof(float));
    
    for (long d=0; d<dim; d++) {
        for (long i=0; i<g->size; i++) {
            for (long j=0; j<g->size; j++) {
                if (i != j) {
                    grad[dim*g->size+i] += f[d*g->size+i] - f[d*g->size+j];
                }
            }
            grad[dim*g->size+i] *= 2.;
        }
    }
    
    MEM_free(f);
}
