/**
 * @brief	This is the declaration of the plugin initalization functions. It is
 * 		responsible for registration of the plugin nodes and all other
 * 		associated types that are available to be loaded in Maya.
 */
#ifndef PLUGIN_MAIN_H
#define PLUGIN_MAIN_H


#include "callback_node.cpp"
#include "apply_callback_command.cpp"


/**
 * This is the entry point of the plugin. It is run when the plugin is first
 * loaded into Maya.
 *
 * @param obj	The internal Maya object containing Maya private information
 * 			about the plug-in.
 *
 * @return		The status code.
 */
MStatus initializePlugin(MObject obj);


/**
 * The teardown function of the plugin. This function unregisters all dependency
 * nodes and other services that the plug-in registers during initialization.
 *
 * @param obj	The internal Maya object containing Maya private information
 * 			about the plug-in.
 *
 * @return		The status code.
 */
MStatus uninitializePlugin(MObject obj);


#endif /* PLUGIN_MAIN_H */
