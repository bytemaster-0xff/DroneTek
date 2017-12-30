#define ssin(val) (val)
#define scos(val) 1.0f

int16_t _atan2(float y, float x){
#define fp_is_neg(val) ((((byte*)&val)[3] & 0x80) != 0)
	float z = y / x;
	int16_t zi = abs(int16_t(z * 100));
	int8_t y_neg = fp_is_neg(y);
	if (zi < 100){
		if (zi > 10)
			z = z / (1.0f + 0.28f * z * z);
		if (fp_is_neg(x)) {
			if (y_neg) z -= PI;
			else z += PI;
		}
	}
	else {
		z = (PI / 2.0f) - z / (z * z + 0.28f);
		if (y_neg) z -= PI;
	}
	z *= (180.0f / PI * 10);
	return z;
}

// Rotate Estimated vector(s) with small angle approximation, according to the gyro data
void rotateV(struct fp_vector *v, float* delta) {
	fp_vector v_tmp = *v;
	v->Z -= delta[ROLL] * v_tmp.X + delta[PITCH] * v_tmp.Y;
	v->X += delta[ROLL] * v_tmp.Z - delta[YAW] * v_tmp.Y;
	v->Y += delta[PITCH] * v_tmp.Z + delta[YAW] * v_tmp.X;
}

float InvSqrt(float x){
	union{
		int32_t i;
		float   f;
	} conv;

	conv.f = x;
	conv.i = 0x5f3759df - (conv.i >> 1);

	return 0.5f * conv.f * (3.0f - x * conv.f * conv.f);
}

int32_t isq(int32_t x){ 
	return x * x; 
}


