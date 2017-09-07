/**
 * @brief 		This is a defintion of a Maya node that demonstrates a method of
 *				installing callbacks onto the scene during its lifetime.
 */
#include "callback_node.h"
#include <maya/MFnTypedAttribute.h>


const MTypeId CallbackNode::kNODE_ID(0x0007ffff);
const MString CallbackNode::kNODE_NAME = "callbackNodeExample";


void *CallbackNode::creator()
{
	return new CallbackNode();
}


MStatus CallbackNode::initialize()
{
	MStatus result;
	// TODO: (sonictk) Implement

	return result;
}


MStatus CallbackNode::compute(const MPlug &plug, MDataBlock &dataBlock)
{
	MStatus result;
	// TODO: (sonictk) Implement

	return result;
}
