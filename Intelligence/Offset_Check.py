import cv2
import numpy as np
import mediapipe as mp

# Initialize MediaPipe Hands
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)

# Initialize video capture
cap = cv2.VideoCapture(0)

# Define padding
padding = 50  # Padding around the frame
initial_position = None  # To store the initial index finger position
frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

# Maximum distance in pixels based on frame size and padding
max_distance_x = (frame_width - 2 * padding) / 2
max_distance_y = (frame_height - 2 * padding) / 2

def detect_index_finger(hand_landmarks):
    # Return the coordinates of the index finger tip
    index_finger_tip = hand_landmarks.landmark[mp_hands.HandLandmark.INDEX_FINGER_TIP]
    return (int(index_finger_tip.x * (frame_width - 2 * padding)) + padding, 
            int(index_finger_tip.y * (frame_height - 2 * padding)) + padding)

def normalize_distance(distance, max_distance):
    # Normalize the distance from -max_distance to +max_distance to the range [-1.0, 1.0]
    return distance / max_distance

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Flip the frame horizontally for a selfie-view effect
    frame = cv2.flip(frame, 1)

    # Add padding to the frame
    padded_frame = frame[padding:frame_height-padding, padding:frame_width-padding]

    # Convert the frame to RGB
    rgb_frame = cv2.cvtColor(padded_frame, cv2.COLOR_BGR2RGB)

    # Process the frame and find hands
    results = hands.process(rgb_frame)

    if results.multi_hand_landmarks:
        for hand_landmarks in results.multi_hand_landmarks:
            # Get the index finger position
            finger_position = detect_index_finger(hand_landmarks)

            # Check if initial position is set
            if initial_position is None:
                initial_position = finger_position
                print("Initial Position Set:", initial_position)

            # Calculate the distance from the initial position
            distance_x = finger_position[0] - initial_position[0]
            distance_y = finger_position[1] - initial_position[1]

            # Normalize the distance
            normalized_distance_x = normalize_distance(distance_x, max_distance_x)
            normalized_distance_y = normalize_distance(distance_y, max_distance_y)

            # Print normalized distances
            print(f"Normalized Distance from Initial Position - X: {normalized_distance_x:.2f}, Y: {normalized_distance_y:.2f}")

            # Draw landmarks on the frame
            mp.solutions.drawing_utils.draw_landmarks(padded_frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)

    else:
        # If no hand is detected, reset the initial position
        initial_position = None

    # Display the padded frame
    cv2.imshow("Hand Tracking with Padding", padded_frame)

    # Break on 'q' key press
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
