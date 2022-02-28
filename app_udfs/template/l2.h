#ifndef __L2_H__
#define __L2_H__

/* source vertex property process */
inline prop_t preprocessProperty(prop_t srcProp)
{
    return 0;
}

/* source vertex property & edge property */
inline prop_t scatterFunc(prop_t srcProp, prop_t edgeProp)
{
    return 0;
}

/* destination property update in RAW solver */
inline prop_t updateMergeInRAWSolver(prop_t ori, prop_t update)
{
    return 0;
}

/* destination property update dst buffer update */
inline prop_t gatherFunc(prop_t ori, prop_t update)
{
    return 0;
}


#endif /* __L2_H__ */
