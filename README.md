# A.R.S.E.N.A.L
### Autonomous Robotic Stabilization, Environment Navigation, and Adaptive Localization

## Directory Structure

### Control Directory
The Control Directory contains the source files for the implementation of the cascaded PID controller. It is set up in PlatformIO on VS Code. If using PlatformIO, the configuration file is already included in the directory, so you can simply upload the code onto the ESP32.

#### Contents of the `src` Subfolder:
1. [main.cpp](./control/src/main.cpp) - The main file where the PID controllers are implemented and connected.
2. [constant.h](./control/src/constant.h) - Contains the definitions and values of each variable used in the `Main.cpp` file.
3. [filtering.h](./control/src/filter.h) - Contains the algorithm used for filtering sensor values.
4. [power.h](./control/src/power.hpp) - Contains the program for power management.
5. [net.h](./control/src/net.h) - The network code used for testing, which sets up a web server on the ESP32 that can communicate through HTTP requests.

### Testing Directory
## How to use the telemetry
The Testing Directory contains all scripts written in Python 3. To run the script:
1. Make sure the ESP32 and the device for monitoring are connected to the same LAN. For ESP32, the LAN could be described in the `net.h` file.
2. Upload the code to the ESP32 and note the IP address it is connected to
3. Change the IP address on the py file for the app that wants to be runned
4. Run the file by typing `python3 {name_of_the_program}` on terminal.
5. The terminal should return the webpage for monitoring the ESP32

#### Description of Each Test File:
1. [app](./tests/app.py) - Sets up a connection to the server on the ESP32 and draws graphs displaying the velocity and tilt of the robot.
2. [app2](./tests/app2.py) - Sets up a connection to the ESP32, takes in data, plots it using Matplotlib, and saves the readings in a CSV file using Pandas.
3. [app3](./tests/app3.py) - Similar to `app2.py`, but also reads the setpoints, which is useful for impulse response experiments.
4. [app4](./tests/app4.py) - the same as app1 but only tracks the outer loop variables (yaw & linear velocity)