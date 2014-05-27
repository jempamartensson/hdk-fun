/*
 * Copyright (c) 2013
 *	Side Effects Software Inc.  All rights reserved.
 *
 * Redistribution and use of Houdini Development Kit samples in source and
 * binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. The name of Side Effects Software may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 */

/// This is the pure C++ implementation of the wave SOP.
/// @see @ref HOM/SOP_HOMWave.py, @ref HOM/SOP_HOMWaveNumpy.py, @ref HOM/SOP_HOMWaveInlinecpp.py, @ref HOM/SOP_HOMWave.C, @ref SOP/SOP_VEXWave.vfl

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

using namespace HDK_Sample;

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

PRM_Template
SOP_UniPdist::myTemplateList[] = {
    PRM_Template(),
};


OP_Node *
SOP_UniPdist::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_UniPdist(net, name, op);
}

SOP_UniPdist::SOP_UniPdist(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op)
{
}

SOP_UniPdist::~SOP_UniPdist()
{
}

OP_ERROR
SOP_UniPdist::cookMySop(OP_Context &context)
{
    // Before we do anything, we must lock our inputs.  Before returning,
    // we have to make sure that the inputs get unlocked.
    if (lockInputs(context) >= UT_ERROR_ABORT)
	return error();

    // Duplicate input geometry
    duplicateSource(0, context);

    // Flag the SOP as being time dependent (i.e. cook on time changes)
    flags().timeDep = 1;

    fpreal frame = OPgetDirector()->getChannelManager()->getSample(context.getTime());
    frame *= 0.03;

	//Create a test attrib
	GA_RWHandleF attrib(gdp->addFloatTuple(GA_ATTRIB_POINT, "test", 1));
	//Creating PointTree
	GEO_PointTreeGAOffset pttree;
	//Build PointTree with all the points
	pttree.build(gdp,NULL);
	//Setting the search distance
	fpreal dist = 1.2;
	//Setting number of points to search for
	int numpt = 10;
	//Create the Array wich holds the distance for the current point in the for loop
	UT_FloatArray ptdist;
	// Create the Array which holds a list sorted on distance to current point in the for loop
	GEO_PointTree::IdxArrayType plist;

	
	
	
	
	// loop over all points, find second closest point
	for (GA_Iterator ptoff(gdp->getPointRange()); !ptoff.atEnd(); ++ptoff)

	{
		//UT_Vector3 pos;
		//int pt = *ptoff;
		UT_Vector3 pos = gdp->getPos3(*ptoff);
		//GA_Offset pt = pttree.findNearestIdx(pos);
		//GA_Offset offset = ptoff.getOffset();
		//UT_Vector3 test(0,offset,0);
		//gdp->setPos3(pt,test);
		
		pttree.findNearestGroupIdx(pos,dist,numpt,plist,ptdist);
		
		attrib.set(*ptoff,plist[1]);
	}

    unlockInputs();
    return error();
}
