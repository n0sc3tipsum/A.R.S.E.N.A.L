import matplotlib.pyplot as plt
import requests
import matplotlib.animation as animation
from scipy.ndimage import gaussian_filter1d
from matplotlib.widgets import Button, TextBox
import time

# Replace with your ESP32 IP address
ESP32_IP = 'http://192.168.191.163/'

tiltX_data = []
time_data = []
speed_data = []

# initialization
setpoint = -0.04

# Initialize plot
fig, (ax, ay) = plt.subplots(2, 1, figsize=(10, 8))
plt.subplots_adjust(bottom=0.4)  # Adjust subplots to make room for buttons and text boxes
 # Adjust subplots to make room for buttons

last_request_time = 0  # Initialize the last request time

# Flag to indicate if recording should stop
stop_recording = False

def fetch_data():
    try:
        response = requests.get(ESP32_IP)
        response.raise_for_status()  # Raise an exception for HTTP errors
        data = response.json()
        return data['tiltX'], data['speed'], data['time']
    except requests.exceptions.RequestException as e:
        print(f"Error fetching data: {e}")
        return None, None
    except ValueError as e:
        print(f"Error parsing JSON: {e}")
        print(f"Response content: {response.content}")  # Debugging line to print response content
        return None, None

def update(frame):
    global last_request_time, stop_recording

    current_time = time.time()
    
    # Check if stop button was pressed
    if stop_recording:
        ani.event_source.stop()
        print("Recording stopped.")
        return

    # Check if 100 milliseconds have passed since the last request
    if (current_time - last_request_time) >= 0.1:  # 0.1 seconds = 100 milliseconds
        tiltX, speed,time_value = fetch_data()
        if tiltX is not None and time_value is not None and speed is not None:
            time_data.append(time_value)
            speed_data.append(speed)
            tiltX_data.append(tiltX)

            # Keep the last 100 data points for a smoother display
            if len(time_data) > 100:
                time_data.pop(0)
                tiltX_data.pop(0)
                speed_data.pop(0)

            # Apply Gaussian smoothing
            smoothed_tiltX = gaussian_filter1d(tiltX_data, sigma=2)  # Adjust sigma as needed
            smoothed_speed = gaussian_filter1d(speed_data, sigma=2) 

            ax.clear()
            ax.plot(time_data, smoothed_tiltX, label='Tilt X', linestyle='-', marker='', color='blue', linewidth=2)
            # ax.fill_between(time_data, smoothed_tiltX, color='blue', alpha=0.2)  # Shading under the curve
            ax.legend(loc='upper left')  # Display the legend
            ax.set_title('IMU Tilt Data')
            ax.set_xlabel('Time (seconds)')
            ax.set_ylabel('Tilt Angle (degrees)')
            plt.grid(True, which='both', linestyle='--', linewidth=0.5)  # Add grid lines

            ay.clear()
            ay.plot(time_data, smoothed_speed, label='Motor Speed', linestyle='-', marker='', color='red', linewidth=2)
            ay.fill_between(time_data, smoothed_speed, color='red', alpha=0.2)  # Shading under the curve
            ay.legend(loc='upper left')  # Display the legend
            ay.set_title('Motor Speed Data')
            ay.set_xlabel('Time (seconds)')
            ay.set_ylabel('Speed (units)')
            ay.grid(True, which='both', linestyle='--', linewidth=0.5)  

        last_request_time = current_time  # Update the last request time

def stop(event):
    global stop_recording
    stop_recording = True
    print("Stop button pressed.")

def update_setpoint(text):
    global setpoint
    try:
        setpoint = float(text)
        response = requests.post(f"{ESP32_IP}update_set", data={'set': setpoint})
        print(f"Request URL: {response.url}")  # Debugging line to print the request URL
        if response.status_code == 200:
            print(f"Setpoint updated to {setpoint}")
        else:
            print(f"Failed to update setpoint: {response.text}")
    except ValueError:
        print("Invalid input for setpoint")



# Create stop button
axstop = plt.axes([0.81, 0.05, 0.1, 0.075])
button_stop = Button(axstop, 'Stop')
button_stop.on_clicked(stop)

axbox_setpoint = plt.axes([0.1, 0.15, 0.5, 0.05])
text_box_setpoint = TextBox(axbox_setpoint, 'Setpoint', initial=str(setpoint))
text_box_setpoint.on_submit(update_setpoint)

ani = animation.FuncAnimation(fig, update, interval=100, save_count=200)  # Call update every 100 ms, with save_count to suppress warning
plt.show()

print("Recording completed and plot displayed.")