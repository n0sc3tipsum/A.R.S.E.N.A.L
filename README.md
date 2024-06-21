# A.R.S.E.N.A.L
### Autonomous Robotic Stabilization, Environment Navigation, and Adaptive Localization

### Branch Structure
- **Control**
  - Control
    - Implementation of PID control (Development Controller)
  - Tests
    - code for monitoring robot (telemetry) and data collection
- **EspBridge-Dual**
    - Implementation of PID control and integration  (Production Controller).
- **ArsenalServer**
    - Server side code 


This branch contains code for the server booter and UI. The server booter is a Flask server that takes commands from the UI and will activate different parts of the ROS2 navigation based on it. This is done by using bash scripts or sending data over the ROS2 system. It will also present values to the UI for display.

The UI code is in `src/ConnectionManager.vb` and `src/Form1.vb`. The rest is code for the server.
