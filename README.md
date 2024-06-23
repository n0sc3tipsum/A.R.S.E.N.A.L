# A.R.S.E.N.A.L
### Autonomous Robotic Stabilization, Environment Navigation, and Adaptive Localization

### Branch Structure
- **Control**
  - Control
    - Implementation of PID control (Development Controller)
  - Tests
    - code for monitoring robot (telemetry) and data collection
- **EspBridgeDual**
    - Implementation of PID control and integration  (Production Controller).
- **ArsenalServer**
    - Server side code which includes SLAM navigation and mapping
- **ArsenalServerBoot**
    - User interface code
    - Server code to handle HTTP requests from UI
    - Bash scripts for automated start up of other subsystems
    

