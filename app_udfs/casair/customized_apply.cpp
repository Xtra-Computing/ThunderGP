
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
#pragma THUNDERGP DEF_ARRAY theta_a
float theta_a;
#pragma THUNDERGP DEF_ARRAY theta_r
float theta_r;
#pragma THUNDERGP DEF_INPUT_ONLY_ARRAY alpha
float alpha;
#pragma THUNDERGP DEF_INPUT_ONLY_ARRAY  pi
float pi;
#pragma THUNDERGP DEF_INPUT_ONLY_ARRAY  N
float N;
#pragma THUNDERGP DEF_SCALAR beta
float beta;
#pragma THUNDERGP DEF_SCALAR gamma
float gamma;


#pragma THUNDERGP USER_APPLY_CODE_START
//start

float n = 1 / N;
float theta_i = uProp * n / FIXED_SCALE;
float weightSum = tProp / FIXED_SCALE;
float updated_i =  beta * pi * theta_s * (theta_i +  weightSum * n);
float theta_i_update = (1 - gamma) * theta_i + updated_i;

float newtheta_a =  theta_a + alpha * theta_s;
float newtheta_s = (1 - alpha) * theta_s - updated_i;
float newtheta_r = theta_r + gamma * theta_i;

wProp = (theta_i_update * N) * FIXED_SCALE;

#pragma THUNDERGP USER_APPLY_CODE_END
//end
}
