#include "callback_node.h"
#include <maya/MFnMatrixData.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MNodeMessage.h>
#include <maya/MGlobal.h>


const MTypeId CallbackNode::kNODE_ID(0x0007ffff);
const MString CallbackNode::kNODE_NAME = "callbackNodeExample";
const char *CallbackNode::kIN_TRANSFORM_ATTR_NAME = "transform";
const char *CallbackNode::kMSG_CXN_ATTR_NAME = "callback";
const char *CallbackNode::kTOGGLE_ATTR_NAME = "toggle";

MObject CallbackNode::inTransformAttr;
MObject CallbackNode::toggleAttr;

MCallbackIdArray CallbackNode::callbacks;


/**
 * This callback is triggered whenever an attribute changes on the callback node.
 * It is responsible for setting up the example features in the scene.
 *
 * @param msg
 * @param plug
 * @param otherPlug
 * @param data
 */
void installCallback(MNodeMessage::AttributeMessage msg,
					 MPlug &plug,
					 MPlug &otherPlug,
					 void *data)
{
	// TODO: (sonictk) Implement
	MGlobal::displayInfo("Feature installed!");

}


/**
 * This callback is triggered whenever the callback node is deleted. It is responsible
 * for handling un-installation of all example features in the scene.
 *
 * @param node
 * @param data
 */
void uninstallCallback(MObject &node, void *data)
{
	// TODO: (sonictk) Implement
	MGlobal::displayInfo("Removed feature!");
}


void *CallbackNode::creator()
{
	return new CallbackNode();
}


MStatus CallbackNode::initialize()
{
	MStatus result;

	MFnMessageAttribute fnMsgAttr;
	inTransformAttr = fnMsgAttr.create(CallbackNode::kIN_TRANSFORM_ATTR_NAME,
									   CallbackNode::kIN_TRANSFORM_ATTR_NAME,
									   &result);
	CHECK_MSTATUS_AND_RETURN_IT(result);

	MFnNumericAttribute fnNumAttr;
	toggleAttr = fnNumAttr.create(CallbackNode::kTOGGLE_ATTR_NAME,
								  CallbackNode::kTOGGLE_ATTR_NAME,
								  MFnNumericData::kBoolean,
								  0,
								  &result);
	CHECK_MSTATUS_AND_RETURN_IT(result);
	fnNumAttr.setKeyable(true);

	addAttribute(inTransformAttr);
	addAttribute(toggleAttr);

	return result;
}


void CallbackNode::postConstructor()
{
	MStatus status;
	MObject thisNode = thisMObject();

	MCallbackId installId = MNodeMessage::addAttributeChangedCallback(thisNode,
																	  installCallback,
																	  NULL,
																	  &status);
	if (status != MStatus::kSuccess) {
		MGlobal::displayError("Unable to install example feature!");
		MMessage::removeCallbacks(callbacks);
		return;
	}
	callbacks.append(installId);

	MCallbackId uninstallId = MNodeMessage::addNodePreRemovalCallback(thisNode,
																	  uninstallCallback,
																	  NULL,
																	  &status);
	if (status != MStatus::kSuccess) {
		MGlobal::displayError("Unable to install example feature!");
		MMessage::removeCallbacks(callbacks);
		return;
	}
	callbacks.append(uninstallId);
}
