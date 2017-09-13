# Tutorial on how to write a install-able Maya node #

## About ##

This is a tutorial on how to write a node that sets up features in your Maya
scene without having explicit dirty propagation relationships in the dependency graph.

For this node, we're going to focus mostly on the *how* of writing the node
itself, and thus the actual functionality will be restricted to something very
basic; in this case, creating a spiral motion when a user moves the transform
node in the scene that we set up the feature for.

All the source code for this example node is available [here](https://github.com/sonictk/maya_node_callback_example).

### What exactly is this approach?

You know how you "normally" do an IK/FK switch in a rig by having 2 joint chains, 
constrain them both to a 3rd joint chain, and then have a bunch of connections in 
the Node Editor tied to some custom attribute on some transform node to blend the 
constraints? 

Think about it for a second: rather than having this "feature" be built into the
dependency graph of the rig itself, why not have it be able to be set up
*on-demand*? There is no *reason* that this feature needs to stay in the rig,
which creates all sorts of complications when you switch between the IK/FK
chains and affect the dependency graph unnecessarily. In short, **there's no
need to store this feature as a fixed state in the dependency graph.**

By utilizing the callback mechanism of Maya, we can avoid having to "bake" this
features/state into the rig, and setup our desired rig behaviours completely
independently of what the rig's graph might actually have, at runtime. We can
also un-install this feature as well, *all without affecting the original
graph*. This philosophy makes it tremendously easy to reason about components of
the rig independently. (Very much like traditional software development!)

Now, it's important to note that not all problems lend themselves well to this 
approach; particularly, problems that are *stateful* (i.e. are tied to time,
velocity, etc.). Those problems tend to require a little more careful thought in
order to determine their suitability towards such an approach.

For more information on this approach to installing scene features on-demand,
please refer to *Raffaele Fragapane's* [Cult Of Rig](http://www.cultofrig.com/2017/07/22/
pilot-season-day-16-automatically-loading-callbacks-scene-load/) 
series on the thought process behind this approach. He goes over it in a lot
more detail.

### Why not just do what Raffaele does and use a ``scriptNode``?

The reason we're using a compiled node here instead of a ``scriptNode`` as he
details in the original approach is due to the fact that the script executed
from a script node in Maya has no concept of which node executed it, and thus
makes it very difficult to implement in production when you reference this
script node in a separate namespace without implementing a callback manager of
some sort to manage the callbacks you are installing into the scene, along with
managing the associated namespaces/nodes for each of those callbacks registered
by the script node.

By using a compiled node instead, we can manage all of that registration/
un-registration mess within the node itself much more cleanly and reason about
the state of our Maya scene a lot more easily than a script node would otherwise
have allowed for.

## Requirements ##

### What you should know ###

- **Basic knowledge of C/C++**. I will focus on including only the code that is
  important; I expect you to be able to understand how to fill in the rest as needed.
- Knowledge of how the dependency graph works in Maya and how dirty propagation works.
- **Basic knowledge of how Maya plugins work and how to write/build them**. There is
  a sample ``CMakeLists.txt`` build script provided for reference if you need a
  refresher on that.
- If you're unfamilar with Maya's callback mechanisms, to **watch the stream**
  listed above to get an idea of what's going on and how they tie to dirty
  propagation in the dependency graph.
- Most importantly: **How to convert between tabs/spaces.** (Of course, you
  should also prefer tabs, but this is a well-understood *fact*, and so I don't
  think further discussion is required on the topic.)

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
    static void *creator();

    static MStatus initialize();

    static const MTypeId kNODE_ID;      /// The unique ID that identifies this node.
    static const MString kNODE_NAME;    /// The name of the DG node.
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
files-into-a-single-compilation-unit) (which is easy, since there's only one
source file right now): 

```c++
#include "callback_node.cpp"

MStatus initializePlugin(MObject obj);

MStatus uninitializePlugin(MObject obj);
```

You should be able to call the compiler command on your corresponding platform
on just ``plugin_main.cpp`` and have the plugin compile correctly.

*I will not go into the details regarding a STU build here, but suffice to say
that I have found them much more beneficial to build times than any other
compiler feature (LTO, IncrediBuild, splitting the code out into pre-compiled
libs, whatever). For such a small project, it doesn't matter; you can switch back
to a more traditional build setup if you prefer.*

*If you have issues building the plugin on your own, please refer to the
included ``CMakeLists.txt`` to see how I manage my own builds. You are not
required to use CMake; it is just my own preference.*

If you got past all that and got a plugin building, great! We have a node that
does...well, *nothing*.

Now we just need to make it work.


### What are we even doing? ###

So before we jump into the node's features itself, let's take a step back and
think about what we want this node to do at a high-level:

{% dot high_level_overview.svg

digraph callbackNodeExample {
    graph[fontname="Arial", fontsize=14, nodesep=1];
    node[fontname="Arial", fontsize=14, shape=box];
    edge[fontname="Arial", fontsize=10];
    transform[label="Transform node", shape=oval];
    callback[label="Callback node"];
    transform -> callback[label="    1. Read translateX"];
    callback -> transform[label="    2. Set new translate Y/Z in a spiral-ly fashion", color=red, style=dashed];
}

%}

Ok. So we know that we want our node to read the translate information from the 
transform node. However, I just promised that we wouldn't have explicit
connections between our nodes in the DG. So what we'll do instead is use
*message attributes* to do the job of making sure our nodes know about each
other. Kind of like Tinder, but just for DG nodes.

### Creating the message attribute ###

If you need a refresher on what a message attribute is:

> **A message attribute is a dependency node attribute that does not transmit
> data**. *Message attributes only exist to formally declare relationships between
> nodes. By connecting two nodes via message attributes, a relationship between
> those nodes is expressed.*
>
> *The Maya Documentation for ``MFnMessageAttribute``, verse 2:0*

Basically, by utilizing this Maya feature, we'll be able to avoid having any data
transmitted explicitly in the graph; we'll be reading it directly from the
transform node itself.

Let's add this message attribute to our custom node definition:

```c++
const char *CallbackNode::kIN_TRANSFORM_ATTR_NAME = "transform";
MObject CallbackNode::inTransformAttr;


MStatus CallbackNode::initialize()
{
    MStatus result;

    MFnMessageAttribute fnMsgAttr;
    inTransformAttr = fnMsgAttr.create(CallbackNode::kIN_TRANSFORM_ATTR_NAME,
                                       CallbackNode::kIN_TRANSFORM_ATTR_NAME,
                                       &result);
    CHECK_MSTATUS_AND_RETURN_IT(result);

    addAttribute(inTransformAttr);
    return result;
}
```
However, let's not forget that this attribute needs to be connected to
something. In this case, that something is the transform node that we're going
to be moving around. 

### Writing the ``MPxCommand`` to setup everything ###

To make things easier for our end-users, let's make a ``MPxCommand`` that they 
can run in order to create our node automatically, along with handling the connections 
between it and the transform node. If you've never written such a thing before
or need some reference, the command I've written is below:

``apply_callback_command.h``

```c++
class ApplyCallbackCommand : public MPxCommand
{
public:
    static void *creator();

    MStatus doIt(const MArgList &args);

    MStatus redoIt();

    MStatus undoIt();

    bool isUndoable() const;

    static MSyntax newSyntax();

    /// The name of the command that is meant to be run.
    static const MString kCOMMAND_NAME;

    MStatus parseArgs(const MArgList &args);

    /// Storage for the flag arguments that will be passed into the command.
    bool flagHelpSpecified = false;
    MSelectionList flagSelList;

    /// Storage for the operations that this command performs on the DG so that we
    /// can undo them if necessary.
    MDGModifier dgMod;
};
```

Let's go over method-by-method of how to implement each of these in 
``apply_callback_command.cpp``:

First, let's define some of the constants we'll be using:

```c++

const char *flagSelListLongName = "-node";
const char *flagSelListShortName = "-n";

const char *flagHelpLongName = "-help";
const char *flagHelpShortName = "-h";

const char *helpText = "This command will setup a callback on a given node.\n"
    "Usage:\n   applyCallback [options]\n"
    "Options:\n"
    "-h / -help     Prints this message.\n\n"
    "-n / -node     The name of the node to setup the callback example for.\n\n";

const MString ApplyCallbackCommand::kCOMMAND_NAME = "applyCallback";

```

The creator function will basically return a new instance of the command.

```c++

void *ApplyCallbackCommand::creator()
{
    return new ApplyCallbackCommand();
}

```

The ``newSyntax()`` function is one we define ourselves, and it's what sets up the 
actual parameters that the command will accept in Maya. We'll also implement ``parseArgs()``
to actually take the arguments we give to the command and figure out if we're 
just calling it with a help flag, or if we're actually passing an object in.

```c++
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

```

The ``doIt`` and ``redoIt`` functions are where the meat of the command happens; 
``doIt`` basically calls ``redoIt`` (so that redos actually work correctly!). We 
create a new callback node, along with checking if the transform node passed into 
the command is valid.

```c++

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
        MGlobal::displayError("The feature already exists!");
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

    return result;
}

```

As for ``isUndoable()``, we just basically tell Maya that this command is undo-able, 
and implement the functionality in ``undoIt()`` by deleting the node.

```c++

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
```

It's a bit of boilerplate code to write, but it's not that complicated when you
see what it's actually doing. We basically create a new ``CallbackNode``, 
accept a single transform as a command argument, and do some basic sanity
checking of the inputs to make sure everything's good. Nothing special, really. 

*(Again, if something here doesn't make sense to you, please look through some of
the examples of how to write command plugins in the Maya documentation.)*

There's one thing we need to do before we call this command good, though; we
need to actually make the connection between the callback node and the transform
node that we pass to the command:

```c++
    //... the earlier part of redoIt()
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
```

We basically create a new message attribute on the transform node and connect it
up (using a different ``MDGModifier``, since our internal one is going to be
reserved for deleting the callback node we created if the user chooses to undo,
and deleting the node will automatically break the connections anyway)

We also need to define ``CallbackNode::kMSG_CXN_ATTR_NAME`` and 
``CallbackNode::kIN_TRANSFORM_ATTR_NAME``, which will be the names of the
message attributes on the callback and transform nodes respectively. You can do
that in the respective source files however you like.

Finally, we should also probably register/de-register this command so that we can
actually use it proper:

```c++

MStatus initializePlugin(MObject obj)
{
    // ...previous stuff

	status = plugin.registerCommand(ApplyCallbackCommand::kCOMMAND_NAME,
									ApplyCallbackCommand::creator,
									ApplyCallbackCommand::newSyntax);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}


MStatus uninitializePlugin(MObject obj)
{
    // ...again, more previous stuff

	status = plugin.deregisterCommand(ApplyCallbackCommand::kCOMMAND_NAME);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}

```

## Getting somewhere ##

Once all of that is done and your plugin is built, you should be able to run the 
following MEL script (or similar):

```
file -f -newFile;
unloadPlugin "callbackNodeExample";
loadPlugin "c:/Users/sonictk/Git/experiments/maya_node_callback_example/build/Debug/callbackNodeExample.mll";

createNode "transform";
applyCallback -n "transform1";
```

Which should give you something similar to the following:

{% dot msg_cxn_setup.svg

digraph callbackNodeExample {
    graph[fontname="Arial", fontsize=12, nodesep=1, rankdir=LR];
    node[fontname="Arial", fontsize=12, shape=Mrecord];
    edge[fontname="Arial", fontsize=10];
    transform[label="<f0>Transform node|<f1>callback node msg. attr."];
    callback[label="<f0>Callback node|<f1>transform node msg. attr."];
    transform:f1 -> callback:f1[label="message conn.", color=blue];
}

%}

If you're still following along; great! Let's now focus on actually *doing*
something useful with the nodes we've made.

### A more detailed overview of the entire setup ###

Now that we've established the relationship between the nodes, let's think about
what we need to do next. We know that we want to get the ``translateX`` value
from the transform node whenever a user changes it interactively. We also know
that we don't want this behaviour to be dirty propagation-based (i.e. no
explicit connection in the graph). 

If you paid attention at all during the stream, you'll know what the answer is:
it's in the form of Maya's various callback mechanisms. However, before we jump
right into writing it up, let's think a little again over what we need to do in
greater detail this time:

{% dot cb_setup.svg

digraph callbackNodeExample {
    graph[fontname="Arial", fontsize=10, nodesep=1, rankdir=LR];
    node[fontname="Arial", fontsize=10, shape=Mrecord];
    edge[fontname="Arial", fontsize=8];
    transform[label="<f0>Transform node|<f1>callback node msg. attr.|<f2>translateX|<f3>attr. changed callback|<f4>translateY/Z"];
    callback[label="<f0>Callback node|<f1>transform node msg. attr.|<f2>installation callback|<f3>callback registry"];
    readTranslateXCB[label="Callback function", shape=oval, style=dashed];
    transform:f1 -> callback:f1[label="message conn.", color=blue, fontcolor=blue];
    callback:f2 -> transform:f3[label="1. Registers this callback when \nnode is created/connected", color=red, fontcolor=red, style=dashed];
    transform:f3 -> readTranslateXCB[label="2. Calls function when translateX. \nchanges interactively on transform node", color=green, fontcolor=green];
    transform:f2 -> transform:f3[color=green, style=dotted];
    readTranslateXCB -> transform:f4:s[label="3. Sets final ty/tz values \non the transform node", color=brown, fontcolor=brown, style=dashed];
    transform:f3 -> callback:f3[label="callback ID is stored so that \nit can be un-registered on node \ndeletion/disconnection", style=dashed, color=grey, fontcolor=grey];
}

%}

Ok, that's a little confusing. As
a [great man](https://en.wikipedia.org/wiki/Logic_(musician)) 
once said, let's [break it down](https://www.youtube.com/watch?v=UhAml-4uDco).

### Registering the callback onto the transform node ###

{% dot cb_setup_p1.svg

digraph callbackNodeExample {
    graph[fontname="Arial", fontsize=10, nodesep=1, rankdir=LR];
    node[fontname="Arial", fontsize=10, shape=Mrecord];
    edge[fontname="Arial", fontsize=8];
    transform[label="<f0>Transform node|<f1>callback node msg. attr.|<f2>translateX|<f3>attr. changed callback|<f4>translateY/Z"];
    callback[label="<f0>Callback node|<f1>transform node msg. attr.|<f2>installation callback|<f3>callback registry"];
    readTranslateXCB[label="Callback function", shape=oval, style=dashed];
    transform:f1 -> callback:f1[label="message conn.", color=blue, fontcolor=blue];
    callback:f2 -> transform:f3[label="1. Registers this callback when \nnode is created/connected", color=grey, fontcolor=grey, style=dotted];
    transform:f3 -> readTranslateXCB[label="2. Calls function when
    translateX. \nchanges interactively on transform node", color=grey, fontcolor=grey, style=dotted];
    transform:f2 -> transform:f3[color=grey, style=dotted];
    readTranslateXCB -> transform:f4:s[label="3. Sets final ty/tz values \non the transform node", color=grey, fontcolor=grey, style=dotted];
    transform:f3 -> callback:f3[label="callback ID is stored so that \nit can be un-registered on node \ndeletion/disconnection", style=dotted, color=grey, fontcolor=grey];
}

%}

We already did the message connection earlier, so we can move on.

{% dot cb_setup_p1.svg

digraph callbackNodeExample {
    graph[fontname="Arial", fontsize=10, nodesep=1, rankdir=LR];
    node[fontname="Arial", fontsize=10, shape=Mrecord];
    edge[fontname="Arial", fontsize=8];
    transform[label="<f0>Transform node|<f1>callback node msg. attr.|<f2>translateX|<f3>attr. changed callback|<f4>translateY/Z"];
    callback[label="<f0>Callback node|<f1>transform node msg. attr.|<f2>installation callback|<f3>callback registry"];
    readTranslateXCB[label="Callback function", shape=oval, style=dashed];
    transform:f1 -> callback:f1[label="message conn.", color=grey, fontcolor=grey, style=dotted];
    callback:f2 -> transform:f3[label="1. Registers this callback when \nnode is created/connected", color=red, fontcolor=red];
    transform:f3 -> readTranslateXCB[label="2. Calls function when
    translateX. \nchanges interactively on transform node", color=grey, fontcolor=grey, style=dotted];
    transform:f2 -> transform:f3[color=grey, style=dotted];
    readTranslateXCB -> transform:f4:s[label="3. Sets final ty/tz values \non the transform node", color=grey, fontcolor=grey, style=dotted];
    transform:f3 -> callback:f3[label="callback ID is stored so that \nit can be un-registered on node \ndeletion/disconnection", style=dotted, color=grey, fontcolor=grey];
}

%}

Let's start here instead. We need to set-up a callback on the callback node when
it is created that is responsible for *setting up another callback on the
transform node* in order to watch for any attribute changes on it. We can do
this by making use of the ``MPxNode::postConstructor`` virtual method, like so:

```c++

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
                                            NULL,
                                            &status);
    if (status != MStatus::kSuccess) {
        MGlobal::displayError("Unable to install example feature!");
        uninstallCallback();
        return;
    }
}

```

What are ``installCallback`` and ``uninstallCallback``, you ask? Let's take a
look at the documentation for both ``MNodeMessage::addAttributeChangedCallback``
and ``MNodeMessage::addNodePreRemovalCallback`` to get some hints:

```c++
MCallbackId addAttributeChangedCallback(MObject &node, 
                                        MNodeMessage::MAttr2PlugFunction func, 
                                        void *clientData = NULL, 
                                        MStatus *ReturnStatus = NULL)
                                        
MCallbackId addNodePreRemovalCallback(MObject &node, 
                                      MMessage::MNodeFunction func, 
                                      void *clientData = NULL, 
                                      MStatus *ReturnStatus = NULL)
```

So basically our ``installCallback`` and ``uninstallCallback`` functions need to
match the signatures of a ``MAttr2PlugFunction`` and a ``MNodeFunction``,
whatever those might be. Looking at an ``MAttr2PlugFunction`` signature gives
the following:

```c++

typedef void(* MAttr2PlugFunction) (MNodeMessage::AttributeMessage msg, 
                                    MPlug &plug, 
                                    MPlug &otherPlug, 
                                    void *clientData)

```

Basically a function pointer that takes two ``MPlugs``. And a ``MNodeFunction``?

```c++

typedef void(* MNodeFunction) (MObject &node, void *clientData)

```

Yep, a function pointer that takes a ``MObject`` node. Not terribly complicated,
which is always good.

Knowing this, we can go ahead and starting writing our callback functions. The
first will be ``uninstallCallback``, which basically just un-registers all the
callbacks that currently exist in the *callback registry* (We'll worry about
this in a bit). We have an overloaded version that returns nothing, and takes an
``MObject&`` along with some arbitrary data in order to match the function
pointer signature detailed above.

```c++

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

```

With that done, we can then go ahead and implement the callback that handles installation 
of the callback onto the transform node itself.

```c++

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

```
Basically, we check the message type that Maya passes us to see if we care about
the type of event that just occurred, and either remove the callbacks from a
global registry that the node maintains if we detect a disconnection, or install
a new callback onto the transform node otherwise. Yes, this means we also need
to define what this "global callback registry" is:

```c++
static MCallbackIdArray callbacks;
```

That's it. No need to over-complicate matters. (Remember, in a STU build, any
variable with ``static`` storage duration is effectively global!)

We'll implement ``featureCallback`` in a little bit. For now, let's look at what
our overview looks like now:

{% dot cb_setup_p1.svg

digraph callbackNodeExample {
    graph[fontname="Arial", fontsize=10, nodesep=1, rankdir=LR];
    node[fontname="Arial", fontsize=10, shape=Mrecord];
    edge[fontname="Arial", fontsize=8];
    transform[label="<f0>Transform node|<f1>callback node msg. attr.|<f2>translateX|<f3>attr. changed callback|<f4>translateY/Z"];
    callback[label="<f0>Callback node|<f1>transform node msg. attr.|<f2>installation callback|<f3>callback registry"];
    readTranslateXCB[label="Callback function", shape=oval, style=dashed];
    transform:f1 -> callback:f1[label="message conn.", color=grey, fontcolor=grey, style=dotted];
    callback:f2 -> transform:f3[label="1. Done!", color=red, fontcolor=red];
    transform:f3 -> readTranslateXCB[label="2. Calls function when
    translateX. \nchanges interactively on transform node", color=grey, fontcolor=grey, style=dotted];
    transform:f2 -> transform:f3[color=grey, style=dotted];
    readTranslateXCB -> transform:f4:s[label="3. Sets final ty/tz values \non the transform node", color=grey, fontcolor=grey, style=dotted];
    transform:f3 -> callback:f3[label="callback ID is stored so that \nit can be un-registered on node \ndeletion/disconnection", color=green, fontcolor=green];
}

%}

Yes, we did skip a little ahead, but it was all for a good cause. Let's go ahead
and implement that ``featureCallback`` now to read the ``translateX`` value off
of the transform node.

```c++

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
    double xVal = plug.asDouble();
}
```

As we can see, it's very similar in theory to how we implemented
``installCallback``; the difference here being that we look for a different type
of event (``kAttributeSet`` instead), and we just get the value of the
translateX plug without doing anything else.

So just like that, our overview looks like this:

{% dot cb_setup_p1.svg

digraph callbackNodeExample {
    graph[fontname="Arial", fontsize=10, nodesep=1, rankdir=LR];
    node[fontname="Arial", fontsize=10, shape=Mrecord];
    edge[fontname="Arial", fontsize=8];
    transform[label="<f0>Transform node|<f1>callback node msg. attr.|<f2>translateX|<f3>attr. changed callback|<f4>translateY/Z"];
    callback[label="<f0>Callback node|<f1>transform node msg. attr.|<f2>installation callback|<f3>callback registry"];
    readTranslateXCB[label="Callback function", shape=oval, style=dashed];
    transform:f1 -> callback:f1[label="message conn.", color=blue, fontcolor=blue];
    callback:f2 -> transform:f3[label="1. Done!", color=red, fontcolor=red];
    transform:f3 -> readTranslateXCB[label="2. Calls function when translateX. \nchanges interactively on transform node", color=green, fontcolor=green];
    transform:f2 -> transform:f3[color=green];
    readTranslateXCB -> transform:f4:s[label="3. Sets final ty/tz values \non the transform node", color=grey, fontcolor=grey, style=dotted];
    transform:f3 -> callback:f3[label="done!", color=green, fontcolor=green];
}

%}

Fast, wasn't it?

Now that we're reading the values, we can focus on step 3: setting new values
back onto the transform node.

### Getting our spiral behavior working ###

If you have any background in high school maths at all, you probably know what
comes next:

```c++
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
    double newYVal = sin(xVal);
    double newZVal = cos(transformZPlug.asDouble() + xVal);
    transformYPlug.setDouble(newYVal);
    transformZPlug.setDouble(newZVal);
}
```

That's right, simple trigonometry functions!

And just like that, we're done!

...Kind of. There's just a bit of cleanup to do: in the ``uninitializePlugin``
function we wrote earlier, we need to make sure that we call
``uninstallCallback()`` as well to remove all the callbacks from the global
registry (since we don't want those to persist if we remove the callback node
from the scene).

```c++

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);
    MStatus status;
    uninstallCallback();
    status =  plugin.deregisterNode(CallbackNode::kNODE_ID);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.deregisterCommand(ApplyCallbackCommand::kCOMMAND_NAME);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}

```

Ok, *now* we're done.

## Conclusion ##

Is this the right solution for everything? No, absolutely not. This method is
best used for installable features onto a rig where you can control the entire
behaviour of the feature within a single set of depedency nodes that you
control. You also need to take care that you do not trigger unnecessary DG
evaluations within your callback methods that could potentially cause cycles in
the DG (which won't be caught by Maya!)

However, this method will work far better than using ``scriptNodes``, since they
will work even when referenced, thus making them far better suited to
production. You also are able to reason about the current callbacks that have
been registered far more easily than if you had been using script nodes and
managing which ones were registered to which namespace/object combinations in
the scene.

Use with a healthy dose of caution and wonder, as always!


## Credits ##

**[Raffaele Fragapane](http://www.cultofrig.com/)**: For the idea regarding this 
in the first place, and for being an awesome smart dude.

**[Ryan Porter](https://github.com/yantor3d)**: For discussing/confirming with me
the limitations of the Maya ``scriptNode`` and for bugging me to write this up in
the first place.

**Siew Yi Liang**: Duh, I wrote these words and the example code here. You can find 
more of my ramblings [here](http://www.sonictk.com/blog).
