//
//  fft.c
//  empty
//
//  Created by Emmanuel Mera on 04/07/2023.
//

#include <complex.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define ROLL_RIGHT_1(a, len) ((a) >> 1) + (((a) & 1) << (len-1))
#define ROLL_LEFT_1(a, len) (((a) & ~(0xffffffff << (len-1))) << 1) + ((a) >> (len-1))

#include "MEM_alloc.h"

#include "fft.h"

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

static void fft(float complex *input, long power);
static void reorder(float complex *entry, long power);

/* ---------------------------- */
/*  implementation              */
/* ---------------------------- */

float *KER_rfft(const float *input, unsigned long _len) {
    long pow = 0;
    while (_len >> pow != 0) {
        pow ++;
    }
    
    float complex *coutput = MEM_calloc_array(sizeof(float complex), 1<<pow, __func__);
    
    pow--;
    
    for (long i = 0; i<_len; i++) {
        coutput[i] = (float complex)input[i];
    }
    
    fft(coutput, pow);
    reorder(coutput, pow);
    
    float *output = MEM_calloc_array(sizeof(float), 1<<++pow, __func__);
    for (long i = 0; i<1<<pow; i++) {
        output[i] = cabsf(coutput[i]);
    }
    
    MEM_free(coutput);
    
    return output;
}

void KER_static_rfft(float *input, unsigned long _len) {
    long pow = 0;
    while ((_len-1) >> pow != 0) {
        pow ++;
    }
    
    float complex *coutput = MEM_calloc_array(sizeof(float complex), 1<<pow, __func__);
    
    for (long i = 0; i<_len; i++) {
        coutput[i] = (float complex)input[i];
    }
    
    reorder(coutput, pow);
    fft(coutput, pow);
    
    ++pow;
    for (long i = 0; i<_len; i++) {
        input[i] = cabsf(coutput[i]);//(float)(1<<pow);
    }
    
    MEM_free(coutput);
}

float complex *KER_fft(const float complex *input, unsigned long _len) {
    long pow = 0;
    while ((_len-1) >> pow != 0) {
        pow ++;
    }
    
    float complex *output = MEM_calloc_array(sizeof(float complex), 1<<pow, __func__);
    
    memcpy(output, input, _len*sizeof(float complex));
    
    reorder(output, pow);
    fft(output, pow);
    
    return output;
}

void KER_static_fft(float complex *input, unsigned long _len) {
    long pow = 0;
    while ((_len-1) >> pow != 0) {
        pow ++;
    }
    
    float complex *output = MEM_calloc_array(sizeof(float complex), 1<<pow, __func__);
    
    memcpy(output, input, _len*sizeof(float complex));
    
    reorder(output, pow);
    fft(output, pow);
    
    memcpy(input, output, _len*sizeof(float complex));
    
    MEM_free(output);
}

// save it for later ...
float *KER_irfft(void) {
    return NULL;
}

float complex *KER_ifft(void) {
    return NULL;
}

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

unsigned long reverse(unsigned long in, long power) {
    unsigned long out = 0;
    for (int i=0; i<power; i++) {
        out<<=1;
        out |= in&1;
        in>>=1;
    }
    return out;
}

void reorder(float complex *entry, long power) {
    for (unsigned long i=0; i<1<<(power-1); i++) {
        float complex tmp = entry[i];
        entry[i] = entry[reverse(i, power)];
        entry[reverse(i, power)] = tmp;
    }
}

void fft(float complex *input, long power) {
    for (unsigned long i=1; i<power+1; i++) {
        for (unsigned long j=0; j<1<<(power-1); j++) {
            float complex omega = cexpf(-2.*M_PI/(float)(1<<i)*I);
            unsigned long filter = 0xffffffff;
            filter <<= i-1;
            
            unsigned long monome = (j - (j & filter)) + ((j & filter) << 1);
            
            float complex left = input[monome];
            float complex right = input[monome+(1<<(i-1))];
            
            input[monome] = left + cpowf(omega, monome) * right;
            input[monome+(1<<(i-1))] = left - cpowf(omega, monome)*right;
        }
    }
}
