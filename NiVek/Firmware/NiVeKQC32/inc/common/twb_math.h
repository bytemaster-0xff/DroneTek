/*
 * twb_math.h
 *
 *  Created on: Nov 25, 2012
 *      Author: kevinw
 */

#ifndef TWB_MATH_H_
#define TWB_MATH_H_

#include "twb_common.h"

float TWB_MATH_LimitOneToMinusOne(float in);
/*float TWB_Math_Asin(float value);*/

int16_t TWB_Math_Asin(int16_t value);

float fmul(float f1, float f2);

float twb_asinf(float f1);

float twb_atanf(float x);

float twb_atan2f(float y, float x);

void vector_normalize(vector_t *a);
float vector_dot(const vector_t *a,const vector_t *b);
void vector_cross(const vector_t *a,const vector_t *b, vector_t *out);

float toDegrees(float radians);
float toRadians(float degrees);

float twb_clampf(float in, float min, float max);
float twb_cosf(float r);
float twb_sinf(float r);

int32_t twb_sqrt(int32_t in);

#endif /* TWB_MATH_H_ */
