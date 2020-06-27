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
inline prop_t updateCalculation(prop_t srcProp, prop_t edgeProp)
{
	return (srcProp);
}

/* destination property update in RAW solver */
inline prop_t updateMergeInRAWSolver(prop_t ori, prop_t update)
{
	return ((ori) + (update));
}

/* destination property update dst buffer update */
inline prop_t updateDestination(prop_t ori, prop_t update)
{
	return ((ori) + (update));
}

/* destination property merge */
inline prop_t applyMerge(prop_t ori, prop_t update)
{
	return ((ori) + (update));
}

inline prop_t applyCalculation( prop_t tProp,
                                prop_t source,
                                prop_t outDeg,
                                unsigned int &extra,
                                unsigned int arg
                              )
{

	prop_t old_score = source;
	prop_t new_score = arg  + ((kDampFixPoint * tProp) >> 7);

	prop_t update = 0;
	if (outDeg != 0)
	{
		update = new_score / outDeg;
	}
	else
	{
		update = 0;
	}
	extra = (new_score - old_score) > 0 ? (new_score - old_score) : (old_score - new_score) ;

	return update;
}
#endif /* __L2_H__ */
