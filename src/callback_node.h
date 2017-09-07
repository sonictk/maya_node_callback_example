#ifndef CALLBACK_NODE_H
#define CALLBACK_NODE_H


#include <maya/MDataBlock.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MTypeId.h>


/// This is a dependency node that will install a callback during its lifetime.
class CallbackNode : MPxNode
{
public:
	/**
	 * The creator function.
	 *
	 * @return	A new instance of the node.
	 */
	static void *creator();

	/**
	 * This is the initialization of the node. It creates the attributes and sets up
	 * their dependencies.
	 *
	 * @return	The status code.
	 */
	static MStatus initialize();

	/**
	 * This function is run when the node is evaluated in the dependency graph.
	 *
	 * @param plug 		The plug representing the attribute that needs to be recomputed.
	 * @param dataBlock	The data block containing storage for the node's attributes.
	 *
	 * @return 			The status code.
	 */
	virtual MStatus compute(const MPlug &plug, MDataBlock &dataBlock);

	static const MTypeId kNODE_ID; 	/// The unique ID that identifies this node.
	static const MString kNODE_NAME;	/// The name of the DG node.
};


#endif /* CALLBACK_NODE_H */
