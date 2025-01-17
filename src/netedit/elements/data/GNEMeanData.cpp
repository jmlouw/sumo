/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2022 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    GNEMeanData.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2022
///
// Class for representing MeanData
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/elements/data/GNEMeanData.h>
#include <netedit/frames/common/GNESelectorFrame.h>
#include <netedit/frames/data/GNEEdgeDataFrame.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/div/GUIDesigns.h>

#include "GNEMeanData.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNEMeanData::GNEMeanData(GNENet *net, GNEEdge* edge, const std::string &file) :
    GUIGlObject(GLO_MEANDATAEDGE, edge->getID(), nullptr),
    GNEHierarchicalElement(net, SUMO_TAG_MEANDATA_EDGE, {}, {edge}, {}, {}, {}, {}),
    myFile(file) {
}


GNEMeanData::GNEMeanData(GNENet *net, GNELane* lane, const std::string &file) :
    GUIGlObject(GLO_MEANDATALANE, lane->getID(), nullptr),
    GNEHierarchicalElement(net, SUMO_TAG_MEANDATA_LANE, {}, {}, {lane}, {}, {}, {}),
    myFile(file) {
}


GNEMeanData::~GNEMeanData() {}


GUIGlObject*
GNEMeanData::getGUIGlObject() {
    return this;
}


void
GNEMeanData::updateGeometry() {
    // nothing to update
}


Position 
GNEMeanData::getPositionInView() const {
    if (getParentLanes().size() > 0) {
        return getParentLanes().front()->getPositionInView();
    } else {
        return getParentEdges().front()->getPositionInView();
    }
}


GUIGLObjectPopupMenu*
GNEMeanData::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    // build header
    buildPopupHeader(ret, app);
    // build menu command for center button and copy cursor position to clipboard
    buildCenterPopupEntry(ret);
    buildPositionCopyEntry(ret, app);
    // buld menu commands for names
    GUIDesigns::buildFXMenuCommand(ret, "Copy " + getTagStr() + " name to clipboard", nullptr, ret, MID_COPY_NAME);
    GUIDesigns::buildFXMenuCommand(ret, "Copy " + getTagStr() + " typed name to clipboard", nullptr, ret, MID_COPY_TYPED_NAME);
    new FXMenuSeparator(ret);
    // build selection and show parameters menu
    buildShowParamsPopupEntry(ret);
    // show option to open additional dialog
    if (myTagProperty.hasDialog()) {
        GUIDesigns::buildFXMenuCommand(ret, ("Open " + getTagStr() + " Dialog").c_str(), getACIcon(), &parent, MID_OPEN_ADDITIONAL_DIALOG);
        new FXMenuSeparator(ret);
    } else {
        GUIDesigns::buildFXMenuCommand(ret, ("Cursor position in view: " + toString(getPositionInView().x()) + "," + toString(getPositionInView().y())).c_str(), nullptr, nullptr, 0);
    }
    return ret;
}


GUIParameterTableWindow*
GNEMeanData::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& /* parent */) {
    // Create table
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // Iterate over attributes
    for (const auto& tagProperty : myTagProperty) {
        // Add attribute and set it dynamic if aren't unique
        if (tagProperty.isUnique()) {
            ret->mkItem(tagProperty.getAttrStr().c_str(), false, getAttribute(tagProperty.getAttr()));
        } else {
            ret->mkItem(tagProperty.getAttrStr().c_str(), true, getAttribute(tagProperty.getAttr()));
        }
    }
    // close building
    ret->closeBuilding();
    return ret;
}


void
GNEMeanData::drawGL(const GUIVisualizationSettings& s) const {
    if (myNet->getViewNet()->getEditModes().isCurrentSupermodeData()) {
        // first push GL ID
        GLHelper::pushName(getGlID());
        // get lanes to draw
        std::vector<GNELane*> lanes;
        if (getParentLanes().size() > 0) {
            lanes.push_back(getParentLanes().front());
        } else {
            lanes = getParentEdges().front()->getLanes();
        }
        // draw over all lanes
        for (const auto& lane : lanes) {
            // get lane width
            const double laneWidth = s.addSize.getExaggeration(s, lane) * s.edgeRelWidthExaggeration *
                                     (lane->getParentEdge()->getNBEdge()->getLaneWidth(lane->getIndex()) * 0.5);
            // Add a draw matrix
            GLHelper::pushMatrix();
            // Start with the drawing of the area translating matrix to origin
            myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_EDGEDATA, 0);
            GLHelper::setColor(RGBColor::BLACK);
            // draw box lines
            GUIGeometry::drawLaneGeometry(s, myNet->getViewNet()->getPositionInformation(),
                                          lane->getLaneShape(), lane->getShapeRotations(),
                                          lane->getShapeLengths(), {}, laneWidth, false);
            // translate to top
            glTranslated(0, 0, 0.01);
            if (getParentLanes().size() > 0) {
                GLHelper::setColor(RGBColor::ORANGE);
            } else {
                GLHelper::setColor(RGBColor::CYAN);
            }
            // draw interne box lines
            GUIGeometry::drawLaneGeometry(s, myNet->getViewNet()->getPositionInformation(),
                                          lane->getLaneShape(), lane->getShapeRotations(),
                                          lane->getShapeLengths(), {}, (laneWidth - 0.1), false);
            // Pop last matrix
            GLHelper::popMatrix();
            // draw lock icon
            GNEViewNetHelper::LockIcon::drawLockIcon(this, getType(), getPositionInView(), 1);
            // check if mouse is over element
            for (const auto& laneParent : lane->getParentEdge()->getLanes()) {
                // get lane drawing constants
                GNELane::LaneDrawingConstants laneDrawingConstants(s, laneParent);
                mouseWithinGeometry(laneParent->getLaneShape(), laneDrawingConstants.halfWidth * s.edgeRelWidthExaggeration);
            }
            // inspect contour
            if (myNet->getViewNet()->isAttributeCarrierInspected(this)) {
                GNEEdge::drawDottedContourEdge(s, GUIDottedGeometry::DottedContourType::INSPECT,
                                               lane->getParentEdge(), true, true, s.edgeRelWidthExaggeration);
            }
            // front contour
            if (myNet->getViewNet()->getFrontAttributeCarrier() == this) {
                GNEEdge::drawDottedContourEdge(s, GUIDottedGeometry::DottedContourType::FRONT,
                                               lane->getParentEdge(), true, true, s.edgeRelWidthExaggeration);
            }
            // delete contour
            if (myNet->getViewNet()->drawDeleteContour(this, this)) {
                GNEEdge::drawDottedContourEdge(s, GUIDottedGeometry::DottedContourType::REMOVE,
                                               lane->getParentEdge(), true, true, s.edgeRelWidthExaggeration);
            }
            // select contour
            if (myNet->getViewNet()->drawSelectContour(this, this)) {
                GNEEdge::drawDottedContourEdge(s, GUIDottedGeometry::DottedContourType::SELECT,
                                               lane->getParentEdge(), true, true, s.edgeRelWidthExaggeration);
            }
        }
        // Pop name
        GLHelper::popName();
    }
}


void
GNEMeanData::deleteGLObject() {
    myNet->deleteMeanData(this, myNet->getViewNet()->getUndoList());
}


void
GNEMeanData::selectGLObject() {
    if (isAttributeCarrierSelected()) {
        unselectAttributeCarrier();
    } else {
        selectAttributeCarrier();
    }
    // update information label
    myNet->getViewNet()->getViewParent()->getSelectorFrame()->getSelectionInformation()->updateInformationLabel();
}


void
GNEMeanData::updateGLObject() {
    // nothing to do
}


Boundary
GNEMeanData::getCenteringBoundary() const {
    return getParentEdges().front()->getCenteringBoundary();
}


std::string
GNEMeanData::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            if (getParentLanes().size() > 0) {
                return getParentLanes().front()->getID();
            } else {
                return getParentEdges().front()->getID();
            }
        case SUMO_ATTR_FILE:
            return myFile;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEMeanData::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


bool
GNEMeanData::isAttributeEnabled(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return false;
        default:
            return true;
    }
}


void
GNEMeanData::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    switch (key) {
        case SUMO_ATTR_FILE:
            undoList->changeAttribute(new GNEChange_Attribute(this, key, value));
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEMeanData::isValid(SumoXMLAttr key , const std::string& value) {
    switch (key) {
        case SUMO_ATTR_FILE:
            return SUMOXMLDefinitions::isValidFilename(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


std::string
GNEMeanData::getPopUpID() const {
    return getTagStr();
}


std::string
GNEMeanData::getHierarchyName() const {
    if (getParentLanes().size() > 0) {
        return getTagStr() + ": " + getParentLanes().front()->getID();
    } else {
        return getTagStr() + ": " + getParentEdges().front()->getID();
    }
    
}


const Parameterised::Map&
GNEMeanData::getACParametersMap() const {
    return GNEAttributeCarrier::PARAMETERS_EMPTY;
}


void
GNEMeanData::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_FILE:
            myFile = value;
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}

/****************************************************************************/
