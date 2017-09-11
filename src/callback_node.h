/**
 * @brief 		This is a defintion of a Maya node that demonstrates a method of
 *				installing callbacks onto the scene during its lifetime.
 */
#ifndef CALLBACK_NODE_H
#define CALLBACK_NODE_H


#include <maya/MDataBlock.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MTypeId.h>
#include <maya/MCallbackIdArray.h>
#include <maya/MNodeMessage.h>


/**
 * This function checks if a callback node already exists in the current Maya session.
 *
 * @return 	``true`` if the node already exists, ``false`` otherwise.
 */
bool doesCallbackNodeAlreadyExist();


/**
 * This function will handle cleanup of all callbacks that were installed for the
 * example feature to work.
 *
 * @return		``MStatus::kSuccess`` if the feature was successfully removed.
 */
MStatus uninstallCallback();


/**
 * This callback is triggered whenever the callback node is deleted. It is responsible
 * for handling un-installation of all the callbacks that were initially set up by
 * this node in the current Maya scene.
 *
 * @param node		Ununsed argument.
 * @param data		Unused argument.
 */
void uninstallCallback(MObject &node, void *data);


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
	 * This function is run when the node is first created. It is responsible for
	 * setting up all the necessary callbacks.
	 *
	 */
	virtual void postConstructor();

	static const MTypeId kNODE_ID; 	/// The unique ID that identifies this node.
	static const MString kNODE_NAME;	/// The name of the DG node.

	static const char *kIN_TRANSFORM_ATTR_NAME;	/// The name of the message connection attribute on the callback node.
	static const char *kMSG_CXN_ATTR_NAME;			/// The name of the message connection attribute on the transform node.
	static const char *kTOGGLE_ATTR_NAME;			/// The name of the attribute to toggle the feature.

	static MObject inTransformAttr;
	static MObject toggleAttr;

	static MCallbackIdArray callbacks; 		/// Storage for the callbacks registered by this node.
};


#endif /* CALLBACK_NODE_H */
