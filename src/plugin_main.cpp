#include "plugin_main.h"
#include <maya/MFnPlugin.h>

const char *kAUTHOR = "Siew Yi Liang";
const char *kVERSION = "1.0.0";
const char *kREQUIRED_API_VERSION = "Any";


MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);

	status = plugin.registerNode(CallbackNode::kNODE_NAME,
								 CallbackNode::kNODE_ID,
								 &CallbackNode::creator,
								 &CallbackNode::initialize,
								 MPxNode::kDependNode);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = plugin.registerCommand(ApplyCallbackCommand::kCOMMAND_NAME,
									ApplyCallbackCommand::creator,
									ApplyCallbackCommand::newSyntax);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}


MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	MStatus status;
	status =  plugin.deregisterNode(CallbackNode::kNODE_ID);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = plugin.deregisterCommand(ApplyCallbackCommand::kCOMMAND_NAME);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	uninstallCallback();
	return status;
}
