# A.R.S.E.N.A.L Server Boot
### Autonomous Robotic Stabilization, Environment Navigation, and Adaptive Localization

This branch contains code for the server booter and UI. The server booter is a Flask server that takes commands from the UI and will activate different parts of the ROS2 navigation based on it. This is done by using bash scripts or sending data over the ROS2 system. It will also present values to the UI for display.

The UI code is in `src/ConnectionManager.vb` and `src/Form1.vb`. 

The main server boot code is in `src/arsenal_server/arsenal_server/server_node.py`.

The bash scripts are at root (ending in `.sh`).

The rest is largely unimportant for understanding the code.
