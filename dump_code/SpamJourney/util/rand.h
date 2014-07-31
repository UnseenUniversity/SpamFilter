/*
 * rand.h
 *
 *  Created on: Apr 16, 2014
 *      Author: alexei
 */

#ifndef RAND_H_
#define RAND_H_

#include <cstdlib>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEF_MEAN  0.5
#define DEF_SIGMA 0.3
#define WEIGHT_LIMIT 2

float inline randf()
{
	return ((float)(rand() / (0.0 + RAND_MAX)));
}

float inline randfr( float abs ){
	return ((float)( randf() * 2 - 1 ) * abs );
}

#define randfr() randfr(WEIGHT_LIMIT)


/* uniform distribution, (0..1] */
inline double drand(){
  return (rand()+1.0)/(RAND_MAX+1.0);
}

/*
 * For more accurate results, consider implementing ziggurat method
 * http://people.sc.fsu.edu/~jburkardt/c_src/ziggurat/ziggurat.html
 */

/* normal distribution, centered on 0, std dev 1 */
inline double random_normal(){
	return sqrt(-2*log(drand())) * cos(2*M_PI*drand());
}

/* normal distribution, custom mean/sigm */
template< int MEAN, int SIGMA >
float rand_normal_distribution(){
	return random_normal() * SIGMA + MEAN;
};



#endif /* RAND_H_ */
