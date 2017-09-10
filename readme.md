# Readme

This is the readme for the **Callback Node Example** Maya plugin.

# About

This is an example of how to create a custom node that handles installation of a
callback onto a rig without having explicit connections in the dependency graph.

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
