#ifndef __L2_H__
#define __L2_H__


#define kDamp               (0.85f)
#define kDampFixPoint       108//(0.85 << 7)  // * 128

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

	prop_t old_score = source;
	prop_t new_score = arg  + ((kDampFixPoint * tProp) >> 7);
	prop_t tmp;
	if (outDeg != 0)
	{
		tmp = (1 << 16 ) / outDeg;
	}
	else
	{
		tmp = 0;
	}

	prop_t update = (new_score * tmp) >> 16;

	extra[0] = (new_score - old_score) > 0 ? (new_score - old_score) : (old_score - new_score) ;

	return update;
}
#endif /* __L2_H__ */
