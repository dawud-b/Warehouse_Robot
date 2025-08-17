# Autonomous Warehouse Robot

## Project Overview

This project demonstrates programming an autonomous robot using the iRobot Create 2. It is designed to navigate a warehouse environment by integrating sensors such as Ping and IR. The robot is capable of detecting and avoiding common obstacles, like columns, walls, spills, and borders. The system uses a Python GUI interface to control the robot and visualize its status via UART communication. The goal of this project is to create a functional prototype of an autonomous robot capable of performing routine tasks in warehouse environments.

## Key Features

- **Autonomous Navigation:** The robot is programmed to independently navigate a warehouse. Its goal is to locate an item (set using coordinates) and then return the item to the drop-off zone. It uses a coordinate grid system, relying on trigonometric calculations to find the shortest path to the item and drop-off.
  
- **Obstacle Detection & Avoidance:** The robot uses external Ping, IR, and bump sensors to detect obstacles in its path and adjust its movement to avoid collisions. If a path is blocked, the robot will turn around and recalculate the next shortest path.

- **Python GUI Interface:** The project includes a custom graphical user interface (GUI) written in Python, allowing users to activate the robot, monitor its status, and control key actions remotely and in real time.

## Technical Highlights

- **Sensor Integration:** Using a Ping (ultrasonic) and IR sensor attached to a servo, the robot can detect objects within a 60-centimeter range in a 180-degree arc.

- **Control System:** With every degree of position marked by the sensors, the robot calculates obstacles' size and distance. Using custom algorithms, the robot adjusts its movement to avoid collisions.

- **UART Communication:** The robot communicates with an external Python server through UART, allowing users to see real-time updates on the robot’s movements, sensor status, and detected obstacles.

## Use Cases

- **Warehouse Automation:** The robot could be employed in a warehouse to autonomously navigate aisles, avoiding obstacles and performing simple tasks like object retrieval.

- **Facility Management:** Using the GUI, there’s no need to be physically present with the robot to track its position. The robot and any obstacles it detects can be displayed remotely.

- **Research & Development:** This project can serve as a foundation for more advanced robotics research, including machine learning for adaptive navigation, multi-robot coordination, and autonomous decision-making.
