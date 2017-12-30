/*	This is an implementation of asinf.  It is written in standard C except
	except float and double are expected be IEEE 754 single- and
	double-precision implementations and that "volatile" is used to attempt to
	force certain floating-point operations to occur at run time (to generate
	exceptions that might not be generated if the operations are performed at
	compile time).  It should be good enough to serve as the libm asinf with
	tolerable performance.
*/


// Include math.h to ensure we match the declarations.
#include <math.h>


float asinf_tbl[] = {0.3790f, /* 0.37 */
0.3898f,
0.4006f,
0.4115f,
0.4225f,
0.4334f,
0.4445f,
0.4556f,
0.4668f,
0.4780f,
0.4893f,
0.5007f,
0.5121f,
0.5236f,
0.5352f,
0.5469f,
0.5586f,
0.5704f,
0.5824f,
0.5944f,
0.6065f,
0.6187f,
0.6311f,
0.6435f,
0.6561f,
0.6687f,
0.6816f,
0.6945f,
0.7076f,
0.7208f,
0.7342f,
0.7478f,
0.7615f,
0.7754f,
0.7895f,
0.8038f,
0.8183f,
0.8331f,
0.8481f,
0.8633f,
0.8788f,
0.8947f,
0.9108f,
0.9273f,
0.9442f,
0.9614f,
0.9791f,
0.9973f,
1.0160f,
1.0353f,
1.0552f,
1.0759f,
1.0973f,
1.1198f,
1.1433f,
1.1681f,
1.1944f,
1.2226f,
1.2532f}; /* 0.95 */

/* Works on input value from 0.37 to 0.95 */
float __twb_math_asinfTbl(float x){
	int idx = (fabs(x) * 100.0f) - 37;
	return (x < 0) ? -asinf_tbl[idx] : asinf_tbl[idx];
}

float twb_asinf(float x){
	/*
	 * If less than 0.37 our error in radians
	 * between input and output is 0.0083 or 0.47 degrees
	 * well within our 0.5 degree tolerance so just return the input
	 */

	if(fabs(x) < 0.37f)
		return x;

	/*
	 * If more than 0.37f, use a lookup table
	 * This gives us roughly a accuracy of 0.55 over the
	 * range, again since this is for our noisy accerometer
	 * it should be OK.
	 */
	if(fabs(x) < 0.951f)
		return __twb_math_asinfTbl(x);

	float result = 1.4777f;

	if(fabs(x) < 0.955f)
		result = 1.2614f;
	else if(fabs(x) < 0.960f)
		result = 1.2783f;
	else if(fabs(x) < 0.965f)
		result = 1.2962f;
	else if(fabs(x) < 0.97f)
		result = 1.3153f;
	else if(fabs(x) < 0.975f)
		result = 1.3360f;
	else if(fabs(x) < 0.980f)
		result = 1.3586f;
	else if(fabs(x) < 0.985f)
		result = 1.3839f;
	else if(fabs(x) < 0.990f)
		result = 1.4133f;
	else if(fabs(x) < 0.995f)
		result = 1.4500f;

	if(x < 0.0f)
		return result * -1.0f;
	else
		return result;
}

