#ifndef __L2_H__
#define __L2_H__

/* source vertex property process */
inline prop_t preprocessProperty(prop_t srcProp)
{
    return (srcProp);
}

/* source vertex property & edge property */
inline prop_t updateCalculation(prop_t srcProp, prop_t edgeProp)
{
    return ((srcProp) * (edgeProp));
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

#endif /* __L2_H__ */
