#ifndef __L2_H__
#define __L2_H__


#define MAX_PROP            (INT_MAX - 1)


/* source vertex property process */
inline prop_t preprocessProperty(prop_t srcProp)
{
    return (srcProp);
}

/* source vertex property & edge property */
inline prop_t updateCalculation(prop_t srcProp, prop_t edgeProp)
{
    return ((srcProp) + (edgeProp));
}

/* destination property update in RAW solver */
inline prop_t updateMergeInRAWSolver(prop_t ori, prop_t update)
{
    return ((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK)))?(update):(ori));
}

/* destination property update dst buffer update */
inline prop_t updateDestination(prop_t ori, prop_t update)
{
    return (((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) || (ori == 0x0))?(update):(ori));
}

/* destination property merge */
inline prop_t applyMerge(prop_t ori, prop_t update)
{
    return ((((((ori)& (~VERTEX_ACTIVE_BIT_MASK)) > ((update) & (~VERTEX_ACTIVE_BIT_MASK))) && (update != 0)) || (ori == 0x0))?(update):(ori));
}

inline prop_t applyCalculation( prop_t tProp,
                                prop_t source,
                                prop_t outDeg,
                                unsigned int &extra,
                                unsigned int arg
                              )
{
    prop_t update = 0;

    prop_t uProp  = source;
    prop_t wProp;
    if ((uProp & 0x80000000) == (tProp & 0x80000000))
    {
        extra = 0;
        wProp = uProp & 0x7fffffff;  // last active vertex
    }
    else if ((tProp & 0x80000000) == 0x80000000)
    {
        extra = 1;
        wProp = tProp; // current active vertex
    }
    else
    {
        extra = 0;
        wProp = MAX_PROP; // not travsered
    }
    update = wProp;

    return update;
}

#endif /* __L2_H__ */
