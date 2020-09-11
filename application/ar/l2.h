#ifndef __L2_H__
#define __L2_H__


#define kDamp               (0.85f)
#define kDampFixPoint       108//(0.85 << 7)  // * 128

#define SCALE_DEGREE		(16)
#define SCALE_DAMPING		(7)

/* source vertex property process */
inline prop_t preprocessProperty(prop_t srcProp)
{
	return (srcProp);
}

/* source vertex property & edge property */
inline prop_t scatterFunc(prop_t srcProp, prop_t edgeProp)
{
	return (srcProp);
}

/* destination property update dst buffer update */
inline prop_t gatherFunc(prop_t ori, prop_t update)
{
	return ((ori) + (update));
}

inline prop_t applyFunc( prop_t tProp,
                         prop_t source,
                         prop_t outDeg,
                         unsigned int (&extra)[APPLY_REF_ARRAY_SIZE],
                         unsigned int arg
                       )
{

	prop_t new_score ;
	prop_t old_score ;
	unsigned int C_avg = arg;

	prop_t tmp;
	prop_t C_Ta = outDeg;
	tmp = (1 << SCALE_DEGREE ) / (C_Ta + C_avg);

	old_score = source * tmp;
	new_score = kDampFixPoint * tProp + (unsigned int) ((1 << (SCALE_DEGREE + SCALE_DAMPING)) * (1.0f - kDamp));

	prop_t update = (new_score * tmp);

	extra[0] = (new_score - old_score) > 0 ? (new_score - old_score) : (old_score - new_score) ;

	return update;
}
#endif /* __L2_H__ */
