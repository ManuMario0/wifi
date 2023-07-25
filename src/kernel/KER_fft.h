//
//  KER_fft.h
//  empty
//
//  Created by Emmanuel Mera on 05/07/2023.
//

#ifndef KER_fft_h
#define KER_fft_h

#include <complex.h>

extern float complex *KER_fft(const float complex *input, unsigned long _len);
extern void KER_static_fft(float complex *input, unsigned long _len);
extern float *KER_rfft(const float *input, unsigned long _len);
extern void KER_static_rfft(float *input, unsigned long _len);

#endif /* KER_fft_h */
