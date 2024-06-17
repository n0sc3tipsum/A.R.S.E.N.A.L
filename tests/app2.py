import requests
import pandas as pd
import matplotlib.pyplot as plt
import time

# Replace with your ESP32 IP address
ESP32_IP = 'http://192.168.246.21/'

# Data lists
tiltX_data = []
time_data = []
speed_data = []

# Function to fetch data from ESP32
def fetch_data():
    try:
        response = requests.get(ESP32_IP)
        response.raise_for_status()  # Raise an exception for HTTP errors
        data = response.json()
        return data['tiltX'], data['speed'], data['time']
    except requests.exceptions.RequestException as e:
        print(f"Error fetching data: {e}")
        return None, None, None
    except ValueError as e:
        print(f"Error parsing JSON: {e}")
        print(f"Response content: {response.content}")  # Debugging line to print response content
        return None, None, None

# Function to save data to CSV
def save_to_csv(filename, tiltX_data, speed_data, time_data):
    df = pd.DataFrame({
        'Time': time_data,
        'TiltX': tiltX_data,
        'Speed': speed_data
    })
    df.to_csv(filename, index=False)

# Function to plot data
def plot_data(tiltX_data, speed_data, time_data):
    plt.figure(figsize=(10, 5))

    # Plot tiltX data
    plt.subplot(2, 1, 1)
    plt.plot(time_data, tiltX_data, label='Tilt X', color='blue')
    plt.xlabel('Time (seconds)')
    plt.ylabel('Tilt Angle (degrees)')
    plt.title('Tilt X over Time')
    plt.legend()
    plt.grid(True)

    # Plot speed data
    plt.subplot(2, 1, 2)
    plt.plot(time_data, speed_data, label='Speed', color='red')
    plt.xlabel('Time (seconds)')
    plt.ylabel('Speed (units)')
    plt.title('Speed over Time')
    plt.legend()
    plt.grid(True)

    plt.tight_layout()
    plt.show()

# Main script
if __name__ == '__main__':
    duration = 60  # Duration to collect data in seconds
    start_time = time.time()

    while (time.time() - start_time) < duration:
        tiltX, speed, current_time = fetch_data()
        if tiltX is not None and speed is not None and current_time is not None:
            tiltX_data.append(tiltX)
            speed_data.append(speed)
            time_data.append(current_time)
            print(f"Time: {current_time}, TiltX: {tiltX}, Speed: {speed}")

        time.sleep(1)  # Fetch data every second

    # Save data to CSV
    save_to_csv('sensor_data.csv', tiltX_data, speed_data, time_data)
    print("Data saved to sensor_data.csv")

    # Plot data
    plot_data(tiltX_data, speed_data, time_data)
