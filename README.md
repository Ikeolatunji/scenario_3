# scenario_3 submission
 

This code is Line Following Robot following line A with Obstacle Avoidance, finding the shortest pathway and returning back to the starting base.

Description

This Arduino sketch implements a line-following robot with obstacle avoidance and pathfinding capabilities using a Zumo 32U4 robot kit. The robot utilizes infrared sensors to detect black lines on a white surface and navigate along the path. Additionally, it incorporates proximity sensors to detect obstacles and avoid collisions. The pathfinding algorithm implemented in the code allows the robot to find the shortest pathway through a maze.

Installation

To use this code, you'll need:
- Arduino IDE
- Zumo 32U4 robot kit
- Required libraries: Wire.h, Zumo32U4.h

1. Connect your Zumo 32U4 robot to your computer via USB.
2. Open the provided Arduino sketch (`Scernari0_3.ino`) in the Arduino IDE.
3. Install the necessary libraries if you haven't already (`Wire.h`, `Zumo32U4.h`).
4. Upload the sketch to your Zumo 32U4 robot.

Usage

1. Ensure your robot is placed on a surface with black lines to follow.
2. Press the button labelled "A" on your robot to start the program.
3. The robot will begin following the black line using the line sensors. It would use this to know the lanes to avoid hitting the walls.
4. If an obstacle(house) is detected in front of the robot, it will stop for 3 seconds, turn away, and then continue moving around the maze like a loop.
5. The robot will also find the shortest pathway on the map using the implemented pathfinding algorithm.
