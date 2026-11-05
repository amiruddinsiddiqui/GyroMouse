import json
import time
import threading
import paho.mqtt.client as mqtt
import pyautogui

BROKER = "broker.emqx.io"
PORT = 1883
TOPIC = "esp/data"

GAIN = 8.0
DEADZONE = 1.5
FILTER = 0.45
MAX_STEP = 35

INVERT_X = True
INVERT_Y = True

CALIBRATION_SAMPLES = 150
BUTTON_DEBOUNCE = 3
MOUSE_HZ = 120

pyautogui.FAILSAFE = True
pyautogui.PAUSE = 0

lock = threading.Lock()

offset_x = offset_y = 0.0
sum_x = sum_y = 0.0
samples = 0
calibrated = False

gx_filtered = gy_filtered = 0.0
left_pressed = right_pressed = False
left_count = right_count = 0
left_candidate = right_candidate = False
last_packet = 0.0


def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("Connected to MQTT")
        client.subscribe(TOPIC)


def on_message(client, userdata, msg):
    global offset_x, offset_y, sum_x, sum_y, samples, calibrated
    global gx_filtered, gy_filtered, left_pressed, right_pressed
    global left_count, right_count, left_candidate, right_candidate
    global last_packet

    try:
        data = json.loads(msg.payload.decode())

        gx = float(data["Gyro_X"])
        gy = float(data["Gyro_Y"])
        left = int(data["Left_btn"]) == 0
        right = int(data["Right_btn"]) == 0

        with lock:
            # Button debounce
            if left == left_candidate:
                left_count += 1
            else:
                left_candidate, left_count = left, 1

            if right == right_candidate:
                right_count += 1
            else:
                right_candidate, right_count = right, 1

            if left_count >= BUTTON_DEBOUNCE:
                left_pressed = left

            if right_count >= BUTTON_DEBOUNCE:
                right_pressed = right

            last_packet = time.perf_counter()

        # Calibration
        if not calibrated:
            sum_x += gx
            sum_y += gy
            samples += 1

            if samples >= CALIBRATION_SAMPLES:
                offset_x = sum_x / samples
                offset_y = sum_y / samples
                calibrated = True
                print("Calibration complete. Gyro mouse active!")

            return

        # Offset correction and filtering
        gx -= offset_x
        gy -= offset_y

        with lock:
            gx_filtered += FILTER * (gx - gx_filtered)
            gy_filtered += FILTER * (gy - gy_filtered)

            if abs(gx_filtered) < DEADZONE:
                gx_filtered = 0.0
            if abs(gy_filtered) < DEADZONE:
                gy_filtered = 0.0

    except (ValueError, KeyError, TypeError, json.JSONDecodeError) as e:
        print("Invalid data:", e)


def mouse_worker():
    global last_packet

    previous = time.perf_counter()
    remainder_x = remainder_y = 0.0
    left_down = right_down = False
    interval = 1.0 / MOUSE_HZ

    while True:
        now = time.perf_counter()
        dt = min(now - previous, 0.03)
        previous = now

        with lock:
            active = last_packet != 0 and now - last_packet < 0.25
            gx, gy = gx_filtered, gy_filtered
            left = left_pressed if active else False
            right = right_pressed if active else False

        try:
            if left != left_down:
                (pyautogui.mouseDown if left else pyautogui.mouseUp)(button="left")
                left_down = left

            if right != right_down:
                (pyautogui.mouseDown if right else pyautogui.mouseUp)(button="right")
                right_down = right

            if active and calibrated:
                vx = gy * GAIN * (-1 if INVERT_X else 1)
                vy = gx * GAIN * (-1 if INVERT_Y else 1)

                remainder_x += vx * dt
                remainder_y += vy * dt

                move_x, move_y = int(remainder_x), int(remainder_y)
                remainder_x -= move_x
                remainder_y -= move_y

                move_x = max(-MAX_STEP, min(MAX_STEP, move_x))
                move_y = max(-MAX_STEP, min(MAX_STEP, move_y))

                if move_x or move_y:
                    pyautogui.moveRel(move_x, move_y)

            else:
                remainder_x = remainder_y = 0.0

        except pyautogui.FailSafeException:
            print("Failsafe activated")
            return

        time.sleep(max(0, interval - (time.perf_counter() - now)))


client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message

threading.Thread(target=mouse_worker, daemon=True).start()

print("Keep MPU6050 stationary for calibration...")
client.connect(BROKER, PORT, 60)

try:
    client.loop_forever()
except KeyboardInterrupt:
    print("Stopping...")
finally:
    pyautogui.mouseUp(button="left")
    pyautogui.mouseUp(button="right")
    client.disconnect()
    
    