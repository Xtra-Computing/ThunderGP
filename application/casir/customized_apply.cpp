
// pre-define:
//                  uProp = theta_i * N
//                  tProp = sum(sourceVertexProp * w) in scatter-gather stage
//                  wProp: write back value

/* Apply: updates all vertices for next iteration */
int applyFunc( ... )
{
#pragma THUNDERGP APPLY_BASE_TYPE  float

#pragma THUNDERGP DEF_ARRAY theta_s
	float theta_s;
#pragma THUNDERGP DEF_ARRAY theta_r
	float theta_r;
#pragma THUNDERGP DEF_INPUT_ONLY_ARRAY  N
	float N;
#pragma THUNDERGP DEF_SCALAR epsilon
	float epsilon;
#pragma THUNDERGP DEF_SCALAR nu
	float nu;
#pragma THUNDERGP DEF_SCALAR a
	float a;
#pragma THUNDERGP DEF_SCALAR eta
	float eta;


#pragma THUNDERGP USER_APPLY_CODE_START
//start
	/*
	    St = Sp-epsilon*Sp*Ip - Sp*a * epsilon * eta* wetVec
	    It = (1-nu)*Ip+epsilon*Sp*Ip + Sp *a * epsilon * eta * wetVec
	    Rt = Rp+nu*Ip
	*/
	float theta_i = uProp / N / FIXED_SCALE;
	float wetVec = tProp / N / FIXED_SCALE;
	float newtheta_s = theta_s
	                   - epsilon * theta_s * theta_i
	                   - theta_s * a * eta * wetVec;
	float newtheta_r = theta_r + nu * theta_i;

	float newtheta_i = (1 - nu) * theta_i
	                   + epsilon * theta_s * theta_i
	                   + theta_s * a * eta * wetVec;
	wProp = newtheta_i * N * FIXED_SCALE;
#pragma THUNDERGP USER_APPLY_CODE_END
//end
}
