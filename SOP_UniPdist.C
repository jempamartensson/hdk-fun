//
//
//
//Rewritten Plucker SOP for the new GA Library in Houdini 13 
//
//
//
//


#include <UT/UT_DSOVersion.h>
#include <SYS/SYS_Math.h>
#include <GU/GU_Detail.h>
#include <GEO/GEO_AttributeHandle.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Director.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <GA/GA_AttributeRef.h>
#include <GEO/GEO_PointTree.h>

#include <iostream.h>
#include "SOP_UniPdist.h"

#define INPUT0_LABEL "Points to distrubute"
#define REMOVE_GROUP_NAME "removePointGroup"
#define KEEP_GROUP_NAME "keepPointGroup"

void
newSopOperator(OP_OperatorTable *table)
{
     table->addOperator(new OP_Operator("uniformPdist",
					"Uniform Point Dist ",
					 SOP_UniPdist::myConstructor,
					 SOP_UniPdist::myTemplateList,
					 1,
					 1,
					 0));
}

static PRM_Range rangeP(PRM_RANGE_UI , 0.0, PRM_RANGE_UI , 100.0);
static PRM_Default  oneDefault(1);
static PRM_Name distanceName ("dist", "Distance");
static PRM_Name keepGroup("keepgroup", "Keep group");


PRM_Template
SOP_UniPdist::myTemplateList[] = {
	PRM_Template(PRM_FLT_J, 1, &distanceName, &oneDefault, 0, &rangeP, 0, 0, 0, "Distance/Radius"),
    PRM_Template(),
};


OP_Node *SOP_UniPdist::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_UniPdist(net, name, op);
}

SOP_UniPdist::SOP_UniPdist(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op), myGroup(0)
{
}

SOP_UniPdist::~SOP_UniPdist()
{
}


OP_ERROR
SOP_UniPdist::cookInputGroups(OP_Context &context, int alone)
{
    // The SOP_Node::cookInputPointGroups() provides a good default
    // implementation for just handling a point selection.
    return cookInputPointGroups(context, myGroup, myDetailGroupPair, alone);
}


OP_ERROR SOP_UniPdist::cookMySop(OP_Context &context)
{
    // Before we do anything, we must lock our inputs.  Before returning,
    // we have to make sure that the inputs get unlocked.
    if (lockInputs(context) >= UT_ERROR_ABORT)
	return error();

    // Duplicate input geometry
    duplicateSource(0, context);

	float time = context.getTime();
	float dist = evalFloat(distanceName.getToken(), 0, time);
	
	GA_PointGroup *removeGroup = gdp->newPointGroup(REMOVE_GROUP_NAME);
	GA_PointGroup *keepGroup   = gdp->newPointGroup(KEEP_GROUP_NAME);
	

    // Flag the SOP as being time dependent (i.e. cook on time changes)
    flags().timeDep = 1;

	//Create a removeAttrib
	GA_RWHandleI removeAttrib(gdp->addIntTuple(GA_ATTRIB_POINT, "__remove__", 1));
	
	//Creating PointTree
	GEO_PointTreeGAOffset pttree;
	//Build PointTree with all the points
	pttree.build(gdp,NULL);
	
	
	//Create the Array wich holds the distance for the current point in the for loop
	UT_FloatArray ptdist;
	
	// Index offset
	const GA_IndexMap points = gdp->getPointMap();
	GA_Size ptoffsetindex;
	ptoffsetindex = points.offsetSize()-points.indexSize();
	

	//set all
	for (GA_Iterator ptoff(gdp->getPointRange()); !ptoff.atEnd(); ++ptoff) 
	{
		removeAttrib.set(*ptoff,0);
	}

	
	
	
	
	// loop over all points, find second closest point
	for (GA_Iterator ptoff(gdp->getPointRange()); !ptoff.atEnd(); ++ptoff)

	{
		int removeMe;
		removeMe = int(removeAttrib.get(*ptoff));
		if(removeMe)
		{
			removeGroup->addIndex(*ptoff-ptoffsetindex);
			continue;
		}
			
		// Create the Array which holds a list sorted on distance to current point in the for loop
		GEO_PointTree::IdxArrayType plist; //plist
		UT_Vector3 pos = gdp->getPos3(*ptoff); // create pos
		pttree.findAllCloseIdx(pos,dist,plist); // find all points within the search radius
		
		
		unsigned int tempListP = plist.entries(); //create a int for entries in the array

		for(int i=0;i < tempListP; ++i)
		{
			removeAttrib.set(plist[i],1); //set the tempList points to be removed
		}
		removeAttrib.set(*ptoff,0);
		keepGroup->addIndex(*ptoff-ptoffsetindex);
	}
	pttree.clear(); //clear the pointtree
	
	
    unlockInputs();
    return error();
}
