//
//
//Define SOP_UniPdist Node
//
//



#ifndef __SOP_UniPdist_h__
#define __SOP_UniPdist_h__

#include <SOP/SOP_Node.h>



class SOP_UniPdist : public SOP_Node
{
public:
	     SOP_UniPdist(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_UniPdist();
	virtual OP_ERROR             cookInputGroups(OP_Context &context, 
                                                int alone = 0);

    static PRM_Template		 myTemplateList[];
    static OP_Node		*myConstructor(OP_Network*, const char *,
							    OP_Operator *);

protected:
    virtual OP_ERROR		 cookMySop(OP_Context &context);
private:
	
	GU_DetailGroupPair   myDetailGroupPair;
	const GA_PointGroup *myGroup;
};

#endif
