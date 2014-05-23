//!
//! \file SOP_Plucker.h
//! \brief Contains the declaration of the SOP_Plucker class.
//!
//! Copyright (c) 2010 Framestore.
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

#ifndef __SOP_Plucker_h__
#define __SOP_Plucker_h__

#include <SOP/SOP_Node.h>


//!
//! Class representing a Plucker SOP.
//!
class SOP_Plucker : public SOP_Node
{

public: // static functions

    //!
    //! Creates a new Plucker SOP using the given parent, name and entry.
    //!
    //! \param parent The parent network the operator will be a part of.
    //! \param name The name to use for the operator.
    //! \param entry The operator's entry.
    //!
    static OP_Node * create ( OP_Network *parent, const char *name,
                              OP_Operator *entry );

public: // static data

    //!
    //! The list of parameter templates defining the operator's parameters.
    //!
    static PRM_Template s_parameterTemplates[];

public: // constructors and destructors

    //!
    //! Constructor of SOP_Plucker class.
    //!
    //! \param parent The parent network the operator will be a part of.
    //! \param name The name to use for the operator.
    //! \param entry The operator's entry.
    //!
    SOP_Plucker ( OP_Network *parent, const char *name, OP_Operator *entry );

    //!
    //! Destructor of SOP_Plucker class.
    //!
    virtual ~SOP_Plucker ();

protected: // functions

    //!
    //! Returns the label for the operator input with the given index.
    //!
    //! \param index The index of the operator input whose label to return.
    //! \return The label for the operator input with the given index.
    //!
    virtual const char * inputLabel ( unsigned int index ) const;

    //!
    //! Cooks the geometry for the SOP.
    //!
    //! \param context The context to use for cooking.
    //! \return The status of cooking the operator's output.
    //!
    virtual OP_ERROR cookMySop ( OP_Context &context );

private: // data

    //!
    //! The group of geometry to be manipulated by this SOP and cooked by the
    //! cookInputGroups() function.
    //!
    const GA_PointGroup *m_pointGroupToModify;

};

#endif
