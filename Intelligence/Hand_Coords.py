import cv2
import mediapipe as mp
import numpy as np
import socket
import threading

#Nets
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = '127.0.0.1'
port = 12345
client_socket.connect((host, port))

# Initialize MediaPipe Hands
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(static_image_mode=False,
                       max_num_hands=2,
                       min_detection_confidence=0.7)

# Initialize MediaPipe Drawing
mp_drawing = mp.solutions.drawing_utils

# Open a connection to the webcam
cap = cv2.VideoCapture(0)

# Initialize Kalman Filter
kalman_x = cv2.KalmanFilter(4, 2)
kalman_y = cv2.KalmanFilter(4, 2)
kalman_z = cv2.KalmanFilter(4, 2)

# Initialize Kalman Filter parameters
for kf in (kalman_x, kalman_y, kalman_z):
    kf.measurementMatrix = np.array([[1, 0, 0, 0],
                                      [0, 1, 0, 0]], np.float32)
    kf.transitionMatrix = np.array([[1, 1, 0, 0],
                                     [0, 1, 0, 0],
                                     [0, 0, 1, 1],
                                     [0, 0, 0, 1]], np.float32)
    kf.processNoiseCov = np.array([[1, 0, 0, 0],
                                    [0, 1, 0, 0],
                                    [0, 0, 1, 0],
                                    [0, 0, 0, 1]], np.float32) * 0.03
    kf.measurementNoiseCov = np.array([[1, 0],
                                        [0, 1]], np.float32) * 0.5

def normalize(value, min_value=-1, max_value=1):
    return (value - min_value) / (max_value - min_value) * 2 - 1

while cap.isOpened():
    # Read a frame from the webcam
    success, image = cap.read()
    if not success:
        print("Ignoring empty camera frame.")
        continue

    # Convert the image to RGB
    image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    # Process the image and find hands
    results = hands.process(image_rgb)

    # Draw the hand annotations on the image
    if results.multi_hand_landmarks:
        for hand_landmarks in results.multi_hand_landmarks:
            # Draw landmarks on the hand
            mp_drawing.draw_landmarks(image, hand_landmarks, mp_hands.HAND_CONNECTIONS)

            # Get the position of the index finger (landmark 8)
            index_finger = hand_landmarks.landmark[mp_hands.HandLandmark.INDEX_FINGER_TIP]
            h, w, _ = image.shape
            index_x = int(index_finger.x * w)
            index_y = int(index_finger.y * h)
            index_z = index_finger.z

            # Kalman Filter Prediction and Update
            measurement_x = np.array([[np.float32(index_x)],
                                       [np.float32(index_y)]])
            kalman_x.predict()
            kalman_y.predict()
            kalman_z.predict()

            kalman_x.correct(measurement_x)
            kalman_y.correct(np.array([[np.float32(index_y)],
                                        [np.float32(index_z)]]))
            kalman_z.correct(np.array([[np.float32(index_z)],
                                        [np.float32(index_z)]]))

            # Get the filtered values as scalars
            smoothed_x = kalman_x.statePost[0, 0]
            smoothed_y = kalman_y.statePost[0, 0]
            smoothed_z = kalman_z.statePost[0, 0]

            # Normalize the coordinates
            normalized_x = normalize(smoothed_x, min_value=-w, max_value=w)
            normalized_y = normalize(smoothed_y, min_value=-h, max_value=h)
            normalized_z = normalize(smoothed_z, min_value=-1, max_value=1)  # Assuming z is already in this range

            # Print the normalized position of the index finger with one decimal point precision
            #print(f"Normalized Index Finger Position: (X: {normalized_x:.1f}, Y: {normalized_y:.1f}, Z: {normalized_z:.1f})")
            
            #Nets
            message = (f"C {normalized_x:.1f} {normalized_y:.1f}")
            print(message);
            client_socket.sendall(message.encode('utf-8'))
            
    # Show the image
    cv2.imshow('Hand Tracking', image)

    if cv2.waitKey(5) & 0xFF == 27:  # Press 'Esc' to exit
        break

# Release the webcam and close windows
cap.release()
cv2.destroyAllWindows()
