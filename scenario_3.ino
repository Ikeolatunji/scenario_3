#include <Wire.h>
#include <Zumo32U4.h>
#include <stdlib.h> // Included from library for random numbers

#define QTR_THRESHOLD     900  // microseconds
#define PROX_THRESHOLD    6    // Threshold for proximity sensor detection

#define REVERSE_SPEED     120  // 0 is stopped, 400 is full speed
#define TURN_SPEED        200
#define FORWARD_SPEED     180
#define REVERSE_DURATION  100  // ms
#define TURN_DURATION     300  // ms (adjust this value for 180-degree turn)
#define MAX_CORNER_COUNT  2    // Maximum corner count before turning 180 degrees
#define RETURN_OBSTACLE_THRESHOLD 3 // Number of obstacles to trigger return to starting base

#define OBSTACLE_REVERSE_DURATION 5000 // 5 seconds

Zumo32U4LCD display;

Zumo32U4ButtonA buttonA;
Zumo32U4Buzzer buzzer;
Zumo32U4Motors motors;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;

#define NUM_SENSORS 3
unsigned int lineSensorValues[NUM_SENSORS];

bool detectedMiddleLine = false;
bool isFollowingWall = false; // Flag to indicate if the robot is following a wall
int obstacleCount = 0; // Count of obstacles encountered

void waitForButtonAndCountDown() {
  ledYellow(1);
  display.clear();
  display.print(F("Press A"));
  buttonA.waitForButton();
  ledYellow(0);
  display.clear();
  // Play audible countdown.
  for (int i = 0; i < 3; i++) {
    delay(1000);
    buzzer.playNote(NOTE_G(3), 200, 15);
  }
  delay(1000);
  buzzer.playNote(NOTE_G(4), 500, 15);
  delay(1000);
}

void setup() {
  lineSensors.initThreeSensors();
  proxSensors.initThreeSensors();
  waitForButtonAndCountDown();
}

void stop() {
  motors.setSpeeds(0, 0);
}

void returnToBase() {
  // Turn 180 degrees
  motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
  delay(2 * TURN_DURATION);
  // Go forward while following the line until the starting point is reached
  while (true) {
    lineSensors.read(lineSensorValues);
    if (lineSensorValues[1] > QTR_THRESHOLD - 500) {
      // If the middle sensor detects the line, go forward
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    } else if (lineSensorValues[0] > QTR_THRESHOLD) {
      // If the leftmost sensor detects the line, turn slightly to the right
      motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    } else if (lineSensorValues[NUM_SENSORS - 1] > QTR_THRESHOLD) {
      // If the rightmost sensor detects the line, turn slightly to the left
      motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    }
    // Check if the robot has returned to the starting point
    if (proxSensors.readBasicFront() && proxSensors.readBasicLeft() && proxSensors.readBasicRight()) {
      break; // Exit the loop when the starting point is reached
    }
  }
  stop(); // Stop the robot
}


void loop() {
  if (buttonA.isPressed()) {
    motors.setSpeeds(0, 0);
    buttonA.waitForRelease();
    waitForButtonAndCountDown();
  }

  lineSensors.read(lineSensorValues);
  proxSensors.read();

  uint8_t leftValue = proxSensors.countsFrontWithLeftLeds(); //Make left sensors LEDs light active
  uint8_t rightValue = proxSensors.countsFrontWithRightLeds(); // Make right sensors LEDs light active

  // Check if the middle sensor detects the line
  detectedMiddleLine = lineSensorValues[1] > QTR_THRESHOLD - 500;

  if (leftValue >= PROX_THRESHOLD || rightValue >= PROX_THRESHOLD) {
    stop(); // Stop the robot
    delay(3000); // Stop for 3 seconds
    // Turn 180 degrees
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(2 * TURN_DURATION); // Adjust for 180-degree turn
    motors.setSpeeds(0, 0); // Stop after turning
    isFollowingWall = false; // Reset wall-following flag
    obstacleCount++; // Increment obstacle count
    if (obstacleCount >= RETURN_OBSTACLE_THRESHOLD) {
      // If obstacle count reaches threshold, return to base
      returnToBase();
    }
  } else {
    if (isFollowingWall) {
      // Continue following the wall
      if (lineSensorValues[0] > QTR_THRESHOLD) {
        // If leftmost sensor detects line, turn slightly to the right.
        motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
        delay(REVERSE_DURATION);
        motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
        delay(TURN_DURATION);
        motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
      } else if (lineSensorValues[NUM_SENSORS - 1] > QTR_THRESHOLD) {
        // If rightmost sensor detects line, turn slightly to the left.
        motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
        delay(REVERSE_DURATION);
        motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
        delay(TURN_DURATION);
        motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
      } else if (detectedMiddleLine) {
        // If middle sensor detects line, go straight slowly to improve detection of black border lines.
        motors.setSpeeds(FORWARD_SPEED / 2, FORWARD_SPEED / 2);
      }
    } else {
      // If not following a wall, start following the wall
      if (detectedMiddleLine) {
        // If middle sensor detects line, back up a little and randomly move left or right.
        motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
        delay(REVERSE_DURATION);

        // Randomly choose a direction to turn
        int randomTurn = rand() % 2; // 0 or 1

        if (randomTurn == 0) {
          // Turn left
          motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
        } else {
          // Turn right
          motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
        }
        delay(TURN_DURATION);
        motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
      } else {
        // Otherwise, go straight slowly to improve detection of black border lines.
        motors.setSpeeds(FORWARD_SPEED / 2, FORWARD_SPEED / 2);
      }

      // Check if the robot is now following the wall
      if (lineSensorValues[0] > QTR_THRESHOLD || lineSensorValues[NUM_SENSORS - 1] > QTR_THRESHOLD) {
        isFollowingWall = true;
      }
    }
  }
}
