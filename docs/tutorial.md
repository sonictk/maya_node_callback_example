# Tutorial #

## About ##

This is a tutorial on how to write a node that sets up features in your Maya
scene without having explicit connections in the dependency graph.

For this node, we're going to focus mostly on the *how* of writing the node
itself, and thus the actual functionality will be restricted to something very
basic (in this case, creating a spiral motion when a user moves the transform
node in the scene that we set up the feature for.).

For more information on this approach to setting up scene features, please refer
to *Raffaele Fragapane's* [Cult Of Rig](http:// www.cultofrig.com/2017/07/22/
pilot-season-day -16-automatically-loading-callbacks- scene-load/) series on the
reasoning behind this approach. The reason we're using a compiled node here
instead of a ``scriptNode`` as he details in the original approach is due to the
fact that the script executed from a script node in Maya has no concept of which
node executed it, and thus makes it very difficult to implement in production
when you reference this script node in a separate namespace without implementing
a callback manager of some sort to manage the callbacks you are installing into
the scene, along with managing the associated namespaces/nodes for each of those
callbacks registered by the script node.

By using a compiled node instead, we can manage all of that
registration/un-registration within the node itself much more cleanly and reason
about the state of our Maya scene a lot more easily than a script node would
otherwise have allowed for.

## Requirements ##

### What you should know ###

- Basic knowledge of C/C++. I will focus on including only the code that is
  important; I expect you to be able to understand how to do things like add
  ``#include`` statements for Maya headers as necessary, along with include
  guards and so on.
- Knowledge of how the dependency graph works in Maya.
- Basic knowledge of how Maya plugins work and how to write/build them. There is
  a sample ``CMakeLists.txt`` build script provided for reference if you need a
  refresher on that.
- If you're unfamilar with Maya's callback mechanisms, to watch the stream
  listed above to get an idea of what's going on and how they tie to dirty
  propagation in the dependency graph.

## Getting started ##

Firstly, we'll just get a basic skeleton setup of the plugin going. As a refresher,
this just means that you need to create a defintion of a ``MPxNode``
that implements a creator function and an initializer function, which
we will call ``creator()`` and ``initialize()`` respectively.

Thus, in ``callback_node.h``:

```c++
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

	static const MTypeId kNODE_ID; 	/// The unique ID that identifies this node.
	static const MString kNODE_NAME;	/// The name of the DG node.
```

And ``callback_node.cpp``, which, for now, looks pretty sparse:

```c++
#include "callback_node.h"

const MTypeId CallbackNode::kNODE_ID(0x0007ffff);
const MString CallbackNode::kNODE_NAME = "callbackNodeExample";


void *CallbackNode::creator()
{
	return new CallbackNode();
}


MStatus CallbackNode::initialize()
{
	MStatus result;
	return result;
}
```

We also add two extra attributes called ``kNODE_ID`` and ``kNODE_NAME`` to help
us identify the dependency node later on.

Great! We've got our node now, let's write the basic plugin structure to
register it. In case you needed a refresher:

In ``plugin_main.cpp``:

```c++
#include "plugin_main.h"
#include <maya/MFnPlugin.h>

const char *kAUTHOR = "Me, the author";
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

	return status;
}


MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	MStatus status;
	status =  plugin.deregisterNode(CallbackNode::kNODE_ID);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}
```

In ``plugin_main.h``, I go ahead and setup a [Single Translation Unit (STU)
Build/Unity build](https://stackoverflow.com/questions/543697/include-all-cpp-
files-into-a-single-compilation-unit):

```c++
#include "callback_node.cpp"

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
```

You should be able to call the compiler command on your corresponding platform
on just ``plugin_main.cpp`` and have the plugin compile correctly.

*I will not go into the details regarding a STU build here, but suffice to say
that I have found them much more beneficial to build times than any other
compiler feature, LTO, IncrediBuild, splitting the code out into pre-compiled
libs, whatever. For such a small project, it doesn't matter; you can switch back
to a more traditional build if you prefer.*

*If you have issues building the plugin on your own, please refer to the
included ``CMakeLists.txt`` to see how I manage my own builds. You are not
required to use CMake; it is just my own preference.*

If you got past all that and got a plugin building, great! We have a node.

Now we just need to make it work.


## What are we even doing? ##

So before we jump into the node's features itself, let's take a step back and
think about what we want this node to do:

{% dot high_level_overview.svg

digraph callbackNodeExample {
    graph[fontname="Arial", fontsize=14, nodesep=1];
    node[fontname="Arial", fontsize=14, shape=box];
    edge[fontname="Arial", fontsize=10];
    transform[label="transform", shape=oval];
    callback[label="callback node"];
    transform -> callback[label="translateX"];
    callback -> transform[label="new translate Y/Z", color=red];
}

%}
