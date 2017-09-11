#include "apply_callback_command.h"
#include "callback_node.h"
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMessageAttribute.h>


const char *flagSelListLongName = "-node";
const char *flagSelListShortName = "-n";

const char *flagHelpLongName = "-help";
const char *flagHelpShortName = "-h";

const char *helpText = "This command will setup a callback on a given node.\n"
	"Usage:\n	applyCallback [options]\n"
	"Options:\n"
	"-h / -help	Prints this message.\n\n"
	"-n / -node	The name of the node to setup the callback example for.\n\n";

const MString ApplyCallbackCommand::kCOMMAND_NAME = "applyCallback";


void *ApplyCallbackCommand::creator()
{
	return new ApplyCallbackCommand();
}


MSyntax ApplyCallbackCommand::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag(flagHelpShortName, flagHelpLongName);
	syntax.addFlag(flagSelListShortName, flagSelListLongName, MSyntax::kSelectionItem);
	syntax.enableQuery(false);
	syntax.enableEdit(false);
	syntax.useSelectionAsDefault(true);

	return syntax;
}


MStatus ApplyCallbackCommand::parseArgs(const MArgList &args)
{
	MStatus result;
	MArgDatabase argDb(syntax(), args, &result);
	CHECK_MSTATUS_AND_RETURN_IT(result);

	if (argDb.isFlagSet(flagHelpShortName)) {
		displayInfo(helpText);
		flagHelpSpecified = true;
		return MStatus::kSuccess;
	} else {
		flagHelpSpecified = false;
	}

	if (argDb.isFlagSet(flagSelListShortName)) {
		argDb.getFlagArgument(flagSelListShortName, 0, flagSelList);
	}

	return result;
}


MStatus ApplyCallbackCommand::doIt(const MArgList &args)
{
	setCommandString(ApplyCallbackCommand::kCOMMAND_NAME);
	clearResult();

	MStatus result = parseArgs(args);
	CHECK_MSTATUS_AND_RETURN_IT(result);

	if (this->flagHelpSpecified == true) {
		return MStatus::kSuccess;
	}

	return redoIt();
}


MStatus ApplyCallbackCommand::redoIt()
{
	MStatus result;
	if (flagSelList.length() != 1) {
		MGlobal::displayError("You need to select a single node to apply the callback to!");
		return MStatus::kInvalidParameter;
	}

	if (doesCallbackNodeAlreadyExist() == true) {
		MGlobal::displayError("The feature already exists! You need to delete the existing callback node first!");
		return MStatus::kFailure;
	}

	MObject callbackNode = dgMod.createNode(CallbackNode::kNODE_ID, &result);
	CHECK_MSTATUS_AND_RETURN_IT(result);
	result = dgMod.doIt();
	CHECK_MSTATUS_AND_RETURN_IT(result);

	MFnDependencyNode fnNode(callbackNode);
	MPlug callbackNodeMsgPlug = fnNode.findPlug(CallbackNode::kIN_TRANSFORM_ATTR_NAME,
												false,
												&result);
	CHECK_MSTATUS_AND_RETURN_IT(result);

	MObject transform;
	result = flagSelList.getDependNode(0, transform);
	CHECK_MSTATUS_AND_RETURN_IT(result);
	if (!transform.hasFn(MFn::kDependencyNode)) {
		MGlobal::displayError("The object specified is not a valid DG node!");
		return MStatus::kInvalidParameter;
	}
	result = fnNode.setObject(transform);
	CHECK_MSTATUS_AND_RETURN_IT(result);
	if (!fnNode.hasAttribute(CallbackNode::kMSG_CXN_ATTR_NAME)) {
		MFnMessageAttribute fnMsgAttr;
		MObject msgAttr = fnMsgAttr.create(CallbackNode::kMSG_CXN_ATTR_NAME,
										   CallbackNode::kMSG_CXN_ATTR_NAME,
										   &result);
		CHECK_MSTATUS_AND_RETURN_IT(result);
		fnNode.addAttribute(msgAttr);
	}
	MDGModifier dgModCxn;
	MPlug msgPlug = fnNode.findPlug(CallbackNode::kMSG_CXN_ATTR_NAME, false, &result);
	CHECK_MSTATUS_AND_RETURN_IT(result);
	result = dgModCxn.connect(msgPlug, callbackNodeMsgPlug);
	CHECK_MSTATUS_AND_RETURN_IT(result);
	dgModCxn.doIt();

	return result;
}


MStatus ApplyCallbackCommand::undoIt()
{
	dgMod.undoIt();
	return MStatus::kSuccess;
}


bool ApplyCallbackCommand::isUndoable() const
{
	if (flagHelpSpecified == true) {
		return false;
	} else {
		return true;
	}
}
