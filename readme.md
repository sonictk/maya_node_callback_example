# Readme

This is the readme for the **Callback Node Example** Maya plugin.

# About

This is an example of how to create a custom node that handles installation of a
callback onto a rig without having explicit connections in the dependency graph.

You can view the full tutorial that goes over creating this node step-by-step [here](https://sonictk.github.io/maya_node_callback_example/).

## Building

### Requirements

* Maya 2016 (and above) needs to be installed in order to link to the libraries. 
* You will also need the devkit headers, which are available from the official 
  Autodesk Github repository.
* CMake 2.8.11 and higher is required for building.

### Instructions for building with CMake

* Create a ``build`` directory and navigate to it. 
* Run ``cmake ../ -DCMAKE_BUILD_TYPE=Release`` and then ``cmake --build .
  --config Release --target install``. The binaries should be installed in the
  ``bin`` folder.

## Sample code

In MEL:

```
file -f -newFile;
unloadPlugin "callbackNodeExample";
loadPlugin "c:/Users/sonictk/Git/experiments/maya_node_callback_example/build/Debug/callbackNodeExample.mll";

createNode "transform";
applyCallback -n "transform1";
```

# Credits

Siew Yi Liang (a.k.a **sonictk**)
