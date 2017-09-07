/**
 * @brief	This is the definiton of the plugin initalization functions. It is
 * 		responsible for registration of the plugin nodes and all other
 * 		associated types that are available to be loaded in Maya.
 *
 */
#include "plugin_main.h"
#include <maya/MFnPlugin.h>

const char *kAUTHOR = "Siew Yi Liang";
const char *kVERSION = "1.0.0";
const char *kREQUIRED_API_VERSION = "Any";
const char *kAPPLY_CALLBACK_COMMAND_NAME = "applyCallbackExample";


//const MString ApplyCallbackCommand::kCOMMAND_NAME = kAPPLY_TENSION_COMMAND_NAME;


/**
 * This is the entry point of the plugin. It is run when the plugin is first
 * loaded into Maya.
 *
 * @param obj	The internal Maya object containing Maya private information
 * 			about the plug-in.
 *
 * @return		The status code.
 */
MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);

	// Register the dependency nodes included with this plugin.
	status = plugin.registerNode(CallbackNode::kNODE_NAME,
								 CallbackNode::kNODE_ID,
								 &CallbackNode::creator,
								 &CallbackNode::initialize,
								 MPxNode::kDependNode);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Register the Maya commands associated with this plugin.
	// status = plugin.registerCommand(ApplyCallbackCommand::kCOMMAND_NAME,
	// 								ApplyCallbackCommand::creator,
	// 								ApplyCallbackCommand::newSyntax);
	// CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}


/**
 * The teardown function of the plugin. This function unregisters all dependency
 * nodes and other services that the plug-in registers during initialization.
 *
 * @param obj	The internal Maya object containing Maya private information
 * 			about the plug-in.
 *
 * @return		The status code.
 */
MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	MStatus status;
	status =  plugin.deregisterNode(CallbackNode::kNODE_ID);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// status = plugin.deregisterCommand(ApplyCallbackCommand::kCOMMAND_NAME);
	// CHECK_MSTATUS_AND_RETURN_IT(status);

	// TODO: (sonictk) Need to account for de-registration of callbacks later
	return status;
}
