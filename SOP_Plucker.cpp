//!
//! \file SOP_Plucker.cpp
//! \brief Contains the definition of the SOP_Plucker class.
//!
//! Copyright (c) 2012 Framestore.
//! All rights reserved.
//!
//! This file contains confidential and proprietary source code, belonging to
//! Framestore. Its contents may not be disclosed to third parties, copied or
//! duplicated in any form, in whole or in part, without prior permission.
//!
//! \author
//!     - Oleksandr Panaskevych <alexander.panaskevych@framestore.com>
//!     - Stefan Habel <stefan.habel@framestore.com>
//!     - Mark Hodgkins
//!

#include <UT/UT_DSOVersion.h>       // required for HDK version tracking
#include <UT/UT_Version.h>
#include <SYS/SYS_Math.h>
#include <UT/UT_Matrix3.h>
#include <UT/UT_Matrix4.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <UT/UT_Vector3Array.h>
#include <UT/UT_PointTree.h>
#include <UT/UT_PtrArray.h>
#include <UT/UT_Vector3.h>
#include <GEO/GEO_Point.h>
#include <GEO/GEO_PointTree.h>
#include <SOP/SOP_Guide.h>
#include <GEO/GEO_AttributeHandle.h>
#include <GA/GA_Types.h>
#include "SOP_Plucker.h"


///
/// Macro Definitions for Attribute Names
///


//!
//! The name of the attribute that stores point normals.
//!
#define NORMAL_ATTRIBUTE_NAME "N"

//!
//! The name of the attribute that stores diffuse colors per point.
//!
#define DIFFUSE_COLOR_ATTRIBUTE_NAME "Cd"

//!
//! The name of the attribute that stores a flag per point whether the point
//! should be added to the group of points that should be removed.
//!
#define REMOVE_POINT_ATTRIBUTE_NAME "to_be_removed"


///
/// Macro Definitions for Input Labels
///


//!
//! The label of the operator's first input.
//!
#define INPUT0_LABEL "Geometry to Plucker"


///
/// Macro Definitions for Point Group Names
///


//!
//! The name of the group to which points are added that should be kept.
//!
#define KEEP_GROUP_NAME "newGroup"

//!
//! The name of the group to which points are added that should be removed.
//!
#define REMOVE_GROUP_NAME "removedGroup"


///
/// Functions with File Scope
///


//!
//! Adds the Density operator to the given operator table.
//!
//! \param table The operator table to add the operator to.
//!
void newSopOperator ( OP_OperatorTable *table )
{
    table->addOperator(new OP_Operator("Plucker", "Plucker",
                                       SOP_Plucker::create,
                                       SOP_Plucker::s_parameterTemplates,
                                       1, 1, 0));
}


///
/// Variables with File Scope for Parameter Names
///

//!
//! The name of the invisible folder parameter that contains metadata for the
//! operator type.
//!
static PRM_Name metadataFolderName ("metadatafolder");

//!
//! The name of the parameter that contains information about the operator
//! type.
//!
static PRM_Name metadataName ("metadata", "Metadata");

//!
//! The name of the parameter that contains the longer description of the
//! operator type.
//!
static PRM_Name metadataDescriptionName ("metadata_description",
                                         "Description");

//!
//! The name of the parameter that contains a description of the operator's
//! inputs.
//!
static PRM_Name metadataInputsName ("metadata_inputs", "Inputs");

//!
//! The name of the parameter that contains the distance value.
//!
static PRM_Name distanceName ("dist", "Distance");

//!
//! The name of a deprecated parameter.
//!
static PRM_Name buildTreeName ("hit", "Build Tree");

//!
//! The name of a deprecated parameter.
//!
static PRM_Name useDirectionVectorName ("usedir", "Use Direction Vector");


///
/// Variables with File Scope for Parameter Defaults
///


//!
//! The default value of the metadata parameter.
//!
static PRM_Default metadataDefault (0,
    "Summary: Supports the creation of an even distribution of points with a "
    "specific spacing by separating them to two new point groups with one of "
    "them representing points to be removed.\n"
    "Creator: Julian Hodgson\n"
    "LinkedInLink: pub/julian-hodgson/1/2b5/97b\n"
    "IMDbLink: name/nm1165174/\n"
    "Maintainer: Stefan Habel\n"
    "Department: FX\n"
    "Package: PluckerSOP\n"
);

//!
//! The default value of the description metadata parameter.
//!
static PRM_Default metadataDescriptionDefault (0,
    "The Plucker SOP helps in creating an even distribution of points by "
    "adding points that are closer to other points than a specific distance "
    "to a new point group, so that they can be easily removed later, e.g. by "
    "a [Delete SOP|Node:sop/delete].\n"
    "\n"
    "The group of points to be removed is named <tt>" REMOVE_GROUP_NAME
    "</tt>, and the group of points that should be kept <tt>" KEEP_GROUP_NAME
    "</tt>.\n"
    "\n"
    "== How it Works Internally ==\n"
    "Points on the input geometry will be iterated over and successively "
    "added to the <tt>" KEEP_GROUP_NAME "</tt> group. Neighboring points "
    "within the specified distance around these points will be marked as "
    "points to be removed by adding them to the <tt>" REMOVE_GROUP_NAME "</tt>"
    " group, and will then be ignored in the iteration.\n"
    "\n"
    "== Controlling the Effect ==\n"
    "The effect of removing the points can be controlled through the red "
    "component of the diffuse color point attribute (<tt>Cd</tt>) in the "
    "input geometry. The distance around a point in which neighboring points "
    "are added to the <tt>" REMOVE_GROUP_NAME "</tt> group is scaled by the "
    "red channel of a point's color, which means that points in black areas "
    "of an input geometry will not be marked to be removed.\n"
    "\n"
    "== History ==\n"
    "The Plucker SOP was used to get an even distribution for the feather "
    "system used on Fawkes the Phoenix, Dumbledore's loyal companion, in "
    "Harry Potter 1, 5 & 6."
);

//!
//! The default value of the inputs description metadata parameter.
//!
static PRM_Default metadataInputsDefault (0,
    INPUT0_LABEL ": The geometry whose points to separate into the new "
                   "groups.\n"
);

//!
//! The name of the tabs contained in the metadata folder parameter.
//!
static PRM_Default metadataFolderDefaults[] = {
    PRM_Default(3, "Metadata") // The first value is the number of parameters
                               // on the tab
};


///
/// Public Static Data
///


//!
//! The list of parameter templates defining the operator's parameters.
//!
PRM_Template SOP_Plucker::s_parameterTemplates[] = {
    // metadata parameters
    PRM_Template(PRM_SWITCHER | PRM_TYPE_INVISIBLE,
                 sizeof(metadataFolderDefaults) / sizeof(PRM_Default),
                 &metadataFolderName, metadataFolderDefaults),
    PRM_Template(PRM_LABEL, 1, &metadataName, &metadataDefault, 0, 0, 0, 0, 1,
                 "Contains information about the operator type."),
    PRM_Template(PRM_LABEL, 1, &metadataDescriptionName,
                 &metadataDescriptionDefault, 0, 0, 0, 0, 1,
                 "Contains a longer description of the operator."),
    PRM_Template(PRM_LABEL, 1, &metadataInputsName, &metadataInputsDefault,
                 0, 0, 0, 0, 1,
                 "Contains a description of the operator's inputs."),

    // operator parameters
    PRM_Template(PRM_STRING, 1, &PRMgroupName, 0, &SOP_Node::pointGroupMenu,
                 0, 0, 0, 1, "The group of points to process."),
    PRM_Template(PRM_FLT_J, 1, &distanceName, PRMzeroDefaults, 0,
                 &PRMscaleRange, 0, 0, 0, "The radius around a point in which "
                 "to add neighboring points to the \"" REMOVE_GROUP_NAME
                 "\" group."),

    // deprecated parameters
    PRM_Template(PRM_FLT_J | PRM_TYPE_INVISIBLE, 1, &buildTreeName,
                 PRMzeroDefaults, 0, &PRMscaleRange, 0, 0, 1, "Deprecated."),
    PRM_Template(PRM_TOGGLE | PRM_TYPE_INVISIBLE, 1, &useDirectionVectorName,
                 0, 0, 0, 0, 0, 1, "Deprecated."),
    PRM_Template(PRM_ORD | PRM_TYPE_INVISIBLE, 1, &PRMorientName, 0,
                 &PRMplaneMenu, 0, 0, 0, 1, "Deprecated."),
    PRM_Template(PRM_DIRECTION | PRM_TYPE_INVISIBLE, 3, &PRMdirectionName,
                 PRMzaxisDefaults, 0, 0, 0, 0, 1, "Deprecated."),
    PRM_Template(),
};


///
/// Public Static Functions
///


//!
//! Creates a new Plucker SOP using the given parent, name and entry.
//!
//! \param parent The parent network the operator will be a part of.
//! \param name The name to use for the operator.
//! \param entry The operator's entry.
//!
OP_Node * SOP_Plucker::create ( OP_Network *parent, const char *name,
                                OP_Operator *entry )
{
    return new SOP_Plucker(parent, name, entry);
}


///
/// Constructors and Destructors
///


//!
//! Constructor of SOP_Plucker class.
//!
//! \param parent The parent network the operator will be a part of.
//! \param name The name to use for the operator.
//! \param entry The operator's entry.
//!
SOP_Plucker::SOP_Plucker ( OP_Network *parent, const char *name,
                           OP_Operator *entry ) :
    SOP_Node(parent, name, entry),
    m_pointGroupToModify(0)
{
}


//!
//! Destructor of SOP_Plucker class.
//!
SOP_Plucker::~SOP_Plucker()
{
}


///
/// Public Functions
///


///
/// Protected Functions
///


//!
//! Returns the label for the operator input with the given index.
//!
//! \param index The index of the operator input whose label to return.
//! \return The label for the operator input with the given index.
//!
const char * SOP_Plucker::inputLabel ( unsigned int index ) const
{
    if (index == 0)
        return INPUT0_LABEL;
    else
        return "Unsupported Input";
}


//!
//! Cooks the geometry for the SOP.
//!
//! \param context The context to use for cooking.
//! \return The status of cooking the operator's output.
//!
OP_ERROR SOP_Plucker::cookMySop ( OP_Context &context )
{
    // Before we do anything, we must lock our inputs and duplicate the first
    // input's geometry. Before returning, we have to make sure that the inputs
    // get unlocked again.
    if (lockInputs(context) >= UT_ERROR_ABORT)
        return error();
    duplicateSource(0, context);

    // get the values of the operator's parameters for the current time
    float time = context.getTime();
    float distance = evalFloat(distanceName.getToken(), 0, time);

    // create two new point groups to divide the points on the input geometry
    // into
    GA_PointGroup *removeGroup = gdp->newPointGroup(REMOVE_GROUP_NAME);
    GA_PointGroup *keepGroup   = gdp->newPointGroup(KEEP_GROUP_NAME);

    // add point attributes to the geometry
    //gdp->addPointAttrib(REMOVE_POINT_ATTRIBUTE_NAME, sizeof(int), GA_ATTRIB_INT, 0);
    gdp->addIntTuple(GA_ATTRIB_POINT, REMOVE_POINT_ATTRIBUTE_NAME, 1);

    // get a reference or handle to the diffuse color point attribute
    GA_RWAttributeRef diffuseColorReference = gdp->findDiffuseAttribute(GA_ATTRIB_POINT);

    // get a reference or handle to the keep group point attribute
    //GA_RWAttributeRef removePointReference = gdp->findAttribute(REMOVE_POINT_ATTRIBUTE_NAME, GB_ATTRIB_INT, GEO_POINT_DICT);
    GA_RWAttributeRef removePointReference = gdp->findIntTuple(GA_ATTRIB_POINT, REMOVE_POINT_ATTRIBUTE_NAME, 1);
    
    if (!removePointReference.isValid())
        addFatal(SOP_ERR_INVALID_ATTRIBUTE_NAME, REMOVE_POINT_ATTRIBUTE_NAME);

    // skip the rest of the function if there has been an error
    if (error() >= UT_ERROR_ABORT) {
        unlockInputs();
        return error();
    }

    // iterate over the points on the input geometry, build a list of points
    // and initialize the value of the keep group point attribute
    UT_PtrArray<GEO_Point *> points;
    GEO_Point *point;
    GA_FOR_ALL_GPOINTS(gdp, point)
    {
        points.append(point);
        point->setValue<int>(removePointReference, 0);
    }

    // build a point tree from the given list of points
    GEO_PointTree pointTree;
    pointTree.build(gdp, points);

    // iterate over all points of the input geometry
    GA_FOR_ALL_GPOINTS(gdp, point)
    {
        // get the value of the remove point point attribute for the current
        // point and check if the point has been marked as to be removed yet
        int removePoint = 0;
        point->get<int>(removePointReference, removePoint);

        if (removePoint)
            continue;

        // get a multiplier value from the diffuse color point attribute of the
        // current point if available
        float multiplier = 1;

        if (diffuseColorReference.isValid()) {
            UT_Vector3 color = UT_Vector3(1, 0, 0);
            point->get<UT_Vector3>(diffuseColorReference, color);

            multiplier = color[0];
        }

        // iterate over the list of points nearest to the current point and set
        // the value of their remove point point attribute to 1, so that they
        // will be added to the point group of points to be removed later
        UT_PtrArray<GEO_Point *> nearestPoints;
        pointTree.findAllClosePt(point->getPos(), distance * multiplier,
                                 nearestPoints);
        unsigned int numNearestPoints = nearestPoints.entries();
        for (int i = 0; i < numNearestPoints; ++i) {
            GEO_Point *nearestPoint = nearestPoints[i];
            nearestPoint->setValue<int>(removePointReference, 1);

        }
        point->setValue<int>(removePointReference, 0);
    }
    pointTree.clear();

    // iterate over all points of the input geometry
    GA_FOR_ALL_GPOINTS(gdp, point)
    {
        int removePoint = 0;
        point->get<int>(removePointReference, removePoint);

        if (removePoint)
            removeGroup->add(point);
        else
            keepGroup->add(point);
    }

    // remove the temporarily created remove point point attribute
    //gdp->destroyPointAttrib(REMOVE_POINT_ATTRIBUTE_NAME, sizeof(int), GB_ATTRIB_INT);
    gdp->destroyPointAttrib(REMOVE_POINT_ATTRIBUTE_NAME);

    unlockInputs();
    return error();
}
