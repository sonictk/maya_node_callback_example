#ifndef APPLY_CALLBACK_COMMAND_H
#define APPLY_CALLBACK_COMMAND_H


#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MSelectionList.h>
#include <maya/MDGModifier.h>


class ApplyCallbackCommand : public MPxCommand
{
public:
	/**
	 * This function returns a new instance of the command.
	 *
	 * @return 	A new instance of this command.
	 */
	static void *creator();

	/**
	 * This method parses the arguments that were given to the command and stores
	 * it in local class data. It finally calls ``redoIt`` to implement the actual
	 * command functionality.
	 *
	 * @param args	The arguments that were passed to the command.
	 * @return		The status code.
	 */
	MStatus doIt(const MArgList &args);

	/**
	 * This method implements the actual functionality of the command. It is also
	 * called when the user elects to perform an interactive redo of the command.
	 *
	 * @return 	The status code.
	 */
	MStatus redoIt();

	/**
	 * This method is called when the user performs an undo of the command. It
	 * restores the scene to its earlier state before the command was run.
	 *
	 * @return 	The status code.
	 */
	MStatus undoIt();

	/**
	 * This method is used to specify whether or not the command is undoable.
	 *
	 * @return 	``true`` if the command is undo-able, ``false`` otherwise.
	 * 			Should only return ``true`` when the command is executed in
	 * 			non-query mode.
	 */
	bool isUndoable() const;

	/**
	 * This static method returns the syntax object for this command.
	 *
	 * @return The syntax object set up for this command.
	 */
	static MSyntax newSyntax();

	/// The name of the command that is meant to be run.
	static const MString kCOMMAND_NAME;

	/**
	 * This method parses the given arguments to the command and stores the
	 * results in local class data.
	 *
	 * @param args		The arguments that were passed to the command.
	 * @return			The status code.
	 */
	MStatus parseArgs(const MArgList &args);

	/// Storage for the flag arguments that will be passed into the command.
	bool flagHelpSpecified = false;
	MSelectionList flagSelList;

	/// Storage for the operations that this command performs on the DG so that we
	/// can undo them if necessary.
	MDGModifier dgMod;
};


#endif /* APPLY_CALLBACK_COMMAND_H */
