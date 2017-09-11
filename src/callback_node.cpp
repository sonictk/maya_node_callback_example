#include "callback_node.h"
#include <cstring>
#include <cmath>
#include <iostream>
#include <maya/MFnMatrixData.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MObjectHandle.h>
#include <maya/MPlugArray.h>
#include <maya/MItDependencyNodes.h>

using std::cerr;
using std::strstr;
using std::sin;
using std::cos;

const MTypeId CallbackNode::kNODE_ID(0x0007ffff);
const MString CallbackNode::kNODE_NAME = "callbackNodeExample";
const char *CallbackNode::kIN_TRANSFORM_ATTR_NAME = "transform";
const char *CallbackNode::kMSG_CXN_ATTR_NAME = "callback";
const char *CallbackNode::kTOGGLE_ATTR_NAME = "toggle";

MObject CallbackNode::inTransformAttr;
MObject CallbackNode::toggleAttr;

MCallbackIdArray CallbackNode::callbacks;


bool doesCallbackNodeAlreadyExist()
{
	MStatus status;
	MItDependencyNodes itDn(MFn::kPluginDependNode, &status);
	if (status != MStatus::kSuccess) {
		cerr << "Error when attempting to create iterator!\n";
		return false;
	}
	MFnDependencyNode fnNode;
	for (; !itDn.isDone(); itDn.next()) {
		MObject curNode = itDn.thisNode();
		fnNode.setObject(curNode);
		if (fnNode.typeId() == CallbackNode::kNODE_ID) {
			return true;
		}
	}
	return false;
}


MStatus uninstallCallback()
{
	MStatus status = MMessage::removeCallbacks(CallbackNode::callbacks);
	MGlobal::displayInfo("Removed feature!");
	return status;
}


void uninstallCallback(MObject &node, void *data)
{
	uninstallCallback();
}


/**
 * This callback is triggered whenever an attribute changes on the callback node.
 * It is responsible for setting up the example features in the scene.
 *
 * @param msg				The message indicating why this callback was triggered.
 * @param plug				The plug representing the attribute that changed.
 * @param otherPlug		Unused argument.
 * @param data				Unused argument.
 */
void featureCallback(MNodeMessage::AttributeMessage msg,
					 MPlug &plug,
					 MPlug &otherPlug,
					 void *data)
{
	if (msg != (MNodeMessage::kAttributeSet|MNodeMessage::kIncomingDirection)) {
		return;
	}
	const char *plugName = plug.partialName(0,0,0,0,0,1).asChar();
	if (strstr("translateX", plugName) == NULL) {
		return;
	}
	MStatus status;
	MPlug transformPlug = plug.parent(&status);
	if (status != MStatus::kSuccess) {
		return;
	}
	double xVal = plug.asDouble();
	MPlug transformYPlug = transformPlug.child(1, &status);
	if (status != MStatus::kSuccess) {
		return;
	}
	MPlug transformZPlug = transformPlug.child(2, &status);
	if (status != MStatus::kSuccess) {
		return;
	}
	// NOTE: (sonictk) Here we make the node go in a spiral motion as an example
	double newYVal = sin(xVal);
	double newZVal = cos(transformZPlug.asDouble() + xVal);
	transformYPlug.setDouble(newYVal);
	transformZPlug.setDouble(newZVal);
}


/**
 * This callback is triggered whenever an attribute changes on the callback node.
 * It is responsible for setting up the example features in the scene.
 *
 * @param msg				The message indicating why this callback was triggered.
 * @param plug				The plug representing the attribute that changed.
 * @param otherPlug		Unused argument.
 * @param data				Unused argument.
 */
void installCallback(MNodeMessage::AttributeMessage msg,
					 MPlug &plug,
					 MPlug &otherPlug,
					 void *data)
{
	if (msg == (MNodeMessage::kConnectionBroken|
				MNodeMessage::kIncomingDirection|
				MNodeMessage::kOtherPlugSet)) {
		uninstallCallback();
	}
	if (msg != (MNodeMessage::kConnectionMade|
				MNodeMessage::kIncomingDirection|
				MNodeMessage::kOtherPlugSet)) {
		return;
	}
	// NOTE: (sonictk) We check if the node has its message connection connected
	// first to determine if we should install the real callback onto that node
	MObject callbackNode = plug.node();
	MFnDependencyNode fnNode(callbackNode);
	MPlug cxnPlug = fnNode.findPlug(CallbackNode::kIN_TRANSFORM_ATTR_NAME);
	MPlugArray connectedPlugs;
	cxnPlug.connectedTo(connectedPlugs, true, false);
	if (connectedPlugs.length() != 1) {
		return;
	}
	MObject transformNode = connectedPlugs[0].node();
	if (!transformNode.hasFn(MFn::kTransform)) {
		return;
	}
	// NOTE: (sonictk) Install the callback onto the other node and add it to the
	// registry of callbacks to track
	MStatus status;
	MCallbackId featureCallbackId = MNodeMessage::addAttributeChangedCallback(transformNode,
																			  featureCallback,
																			  NULL,
																			  &status);
	if (status != MStatus::kSuccess) {
		return;
	}
	CallbackNode::callbacks.append(featureCallbackId);
	MGlobal::displayInfo("Feature installed!");
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
		uninstallCallback();
		return;
	}
	callbacks.append(installId);
	MNodeMessage::addNodePreRemovalCallback(thisNode,
											uninstallCallback,
											//callbacksPtr,
											NULL,
											&status);
	if (status != MStatus::kSuccess) {
		MGlobal::displayError("Unable to install example feature!");
		uninstallCallback();
		return;
	}
}
