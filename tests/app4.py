from flask import Flask, jsonify, render_template, request
import requests

app = Flask(__name__)

# Replace with your ESP32 IP address
ESP32_IP = 'http://192.168.156.21/'

@app.route('/')
def index():
    return render_template('rot.html')

@app.route('/data')
def data():
    try:
        response = requests.get(ESP32_IP)
        response.raise_for_status()  # Raise an exception for HTTP errors
        data = response.json()
        return jsonify(data)
    except requests.exceptions.RequestException as e:
        return jsonify({'error': str(e)}), 500

@app.route('/update_setpoint', methods=['POST'])
def update_setpoint():
    new_setpoint = request.json.get('setpoint')
    if new_setpoint is not None:
        try:
            response = requests.post(f"{ESP32_IP}update_set", data={'set': new_setpoint})
            response.raise_for_status()  # Raise an exception for HTTP errors
            return jsonify({'status': 'success'}), 200
        except requests.exceptions.RequestException as e:
            return jsonify({'error': str(e)}), 500
    return jsonify({'error': 'Invalid setpoint'}), 400

@app.route('/update_rotational_setpoint', methods=['POST'])
def update_rotational_setpoint():
    new_rotational_setpoint = request.json.get('rotational_setpoint')
    if new_rotational_setpoint is not None:
        try:
            response = requests.post(f"{ESP32_IP}update_rotational_set", data={'rotational_set': new_rotational_setpoint})
            response.raise_for_status()  # Raise an exception for HTTP errors
            return jsonify({'status': 'success'}), 200
        except requests.exceptions.RequestException as e:
            return jsonify({'error': str(e)}), 500
    return jsonify({'error': 'Invalid rotational setpoint'}), 400

if __name__ == '__main__':
    app.run(debug=True, port=5001)
