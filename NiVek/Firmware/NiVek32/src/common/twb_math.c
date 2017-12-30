/*
 * twb_math.c
 *
 *  Created on: Nov 25, 2012
 *      Author: kevinw
 */

#include "./common/twb_math.h"
#include "./common/twb_common.h"
#include <stdlib.h>

const uint16_t __asinLookup[101] = {0,57,115,172,229,287,344,401,459,516,574,632,689,747,805,863,921,979,1037,1095,1154,1212,1271,1330,1389,1448,1507,1566,1626,1686,1746,1806,1866,1927,1988,2049,2110,2172,2233,2295,2358,2420,2483,2547,2610,2674,2739,2803,2869,2934,3000,3066,3133,3201,3268,3337,3406,3475,3545,3616,3687,3759,3832,3905,3979,4054,4130,4207,4284,4363,4443,4523,4605,4689,4773,4859,4946,5035,5126,5219,5313,5410,5508,5610,5714,5821,5932,6046,6164,6287,6416,6551,6693,6843,7005,7181,7374,7593,7852,8189,9000};

float twb_clampf(float in, float min, float max){
	if(in < min) return min;
	if(in > max) return max;

	return in;
}

int16_t TWB_Math_Asin(int16_t value){
	bool changeSign = value < 0;
	if(changeSign)
		value = -value;

	if(value > 100)
		value = 100;

	if(changeSign)
		return -__asinLookup[value];
	else
		return __asinLookup[value];
}

float toRadians(float degrees){
	return degrees * 0.017453f;
}

float toDegrees(float radians){
	return radians * 57.2957f;
}

float TWB_MATH_LimitOneToMinusOne(float val){
	if(val < -1.0f) return -1.0f;
	if(val > 1.0f) return 1.0f;

	return val;
}

/*
 * SIN/COS approximations from:
	http://lab.polygonal.de/wp-content/assets/070718/fastTrig.as

	Thanks!
*/

float twb_sinf(float r){
	if (r < -3.14159265f)
	    r += 6.28318531f;
	else
	if (r >  3.14159265f)
	    r -= 6.28318531f;


	float sin;

	if (r < 0){
	    sin = 1.27323954f * r + .405284735f * r * r;

	    if (sin < 0)
	        sin = .225f * (sin *-sin - sin) + sin;
	    else
	        sin = .225f * (sin * sin - sin) + sin;
	}
	else{
	    sin = 1.27323954f * r - 0.405284735f * r * r;

	    if (sin < 0)
	        sin = .225f * (sin *-sin - sin) + sin;
	    else
	        sin = .225f * (sin * sin - sin) + sin;
	}

	return sin;
}

float initialCosValue;
float cosValue;
float inputAngle;

float twb_cosf(float r){
	inputAngle = r;
	r += 1.57079632f;
	if (r >  3.14159265f)
	    r -= 6.28318531f;

	if (r < 0)
	{
		cosValue = 1.27323954f * r + 0.405284735f * r * r;

		initialCosValue = cosValue;

	    if (cosValue < 0)
	    	cosValue = .225f * (cosValue *-cosValue - cosValue) + cosValue;
	    else
	    	cosValue = .225f * (cosValue * cosValue - cosValue) + cosValue;
	}
	else
	{
		cosValue = 1.27323954f * r - 0.405284735f * r * r;

		initialCosValue = cosValue;

	    if (cosValue < 0)
	    	cosValue = .225f * (cosValue *-cosValue - cosValue) + cosValue;
	    else
	    	cosValue = .225f * (cosValue * cosValue - cosValue) + cosValue;
	}

	return cosValue;
}

void vector_cross(const vector_t *a,const vector_t *b, vector_t *out){
	out->x = a->y * b->z - a->z * b->y;
	out->y = a->z * b->x - a->x * b->z;
	out->z = a->x * b->y - a->y * b->x;
}

float vector_dot(const vector_t *a,const vector_t *b){
	return a->x * b->x+a->y * b->y+a->z * b->z;
}

void vector_normalize(vector_t *a){
	float mag = sqrt(vector_dot(a,a));
	a->x /= mag;
	a->y /= mag;
	a->z /= mag;
}

#define PI_FLOAT     3.14159265f
#define PIBY2_FLOAT  1.5707963f

float twb_atanf(float x){
    return PI_FLOAT*x - x*(fabsf(x) - 1.0f)*(0.2447f + 0.0663f*fabsf(x));
}

float twb_atan2f(float y, float x){
	if ( x == 0.0f )
	{
		if ( y > 0.0f ) return PIBY2_FLOAT;
		if ( y == 0.0f ) return 0.0f;
		return -PIBY2_FLOAT;
	}

	float atan;
	float z = y/x;
	if ( fabs( z ) < 1.0f )
	{
		atan = z/(1.0f + 0.28f*z*z);
		if ( x < 0.0f )
		{
			if ( y < 0.0f ) return atan - PI_FLOAT;
			return atan + PI_FLOAT;
		}
	}
	else
	{
		atan = PIBY2_FLOAT - z/(z*z + 0.28f);
		if ( y < 0.0f ) return atan - PI_FLOAT;
	}
	return atan;
}

#define SQRT_TOLERANCE 0.0001

int32_t twb_sqrt(int32_t value){
    double old;
    double new;
    double delta;

    /* check for valid input value */
    if (value < 0.0)
    {
        return (-1.0);
    }
    else if (value == 0.0)
    {
        return(value);
    }

    /* start with some guess */
    if (value < 1.0)
    {
        old = value * 2.0;
    }
    else
    {
        old = value / 2.0;
    }
    new = (old + (value / old)) / 2.0;

    /* start successive approximation */
    do
    {
        old = new;
        new = (old + (value / old)) / 2.0;
        delta = (old - new) / new;
    } while ((delta > SQRT_TOLERANCE) || (delta < (-SQRT_TOLERANCE)));

    return (new);
	return 0;
}
