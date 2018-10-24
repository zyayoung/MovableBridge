# MovableBridge

## Introduction

### Circuit Diagram

Our bridge must know whether there is a car A or B on it and whether there is a big car (ship) is waiting below or passing. To fulfill this requirement, we need two infrared object sensor to judge whether there is a car on the bridge and two ultrasonic distance sensors to detect whether is a ship waiting/passing under the bridge.

Next, we must raise the bridge when there is a ship waiting under the bridge and there is no car on the bridge. To fulfill this requirement, we need two DC motors and a DC motor driver.

Then, we need to stop the car when the bridge is going to raise or have been raised, so we need a servo (to raise and lower the blocking system) and two LEDs (red and green) each side.

Furthermore, we need an Arduino to read the sensors and control the bridge-raising system and the car-blocking system.

![](https://raw.githubusercontent.com/zyayoung/MovableBridge/master/cable_connection.png?token=AGkZAypo4t3WaviW3AHfkFnpgQ2Oi9qrks5bxa9MwA%3D%3D)

In the circuit diagram, we label all 5v by red and GND by black. Since a single N20 motor only need 0.3W@6v when raising the bridge and 5w when being locked. Thus a power bank, which can offer 2A@5V, is enough.

### Programming

The main logic is that we keep the bridge lowered unless there is a ship waiting below and there is no car on the bridge.

We designed the program to be an automaton: it saves some states (flags) and update these states due to the readings from the sensors and take action due to the states. The main loop will be executed 10 times per second and will not be blocked unless the bridge is raising or lowering. By doing so, we can deal with unprecise readings from the ultrasonic sensors and behave correctly regardless of the sequence of testing.

For example, we use an "ultrasonic sensor current state" to indicate whether the ultrasonic sensor has detected a ship.

To update this state, we use another state to indicate the time the indication from the ultrasonic sensor remains the same. When it remains for 1s, the "ultrasonic sensor current state" will be updated.

To use this state, if the state is true (which means that there is a ship waiting below) and there is no car on the bridge (which is another state), then the bridge will be raised.

### Cable Connection

In out project, we use varnished wire.

1. Solder all 5v and GND together
2. Cut Dupont lines and extent it with varnished wire (remember to remove varnish before connecting)
3. Connect other cables

## Group members

- Dong Yulong
- Li Yixuan
- Shi Xiaotian
- Zhang Yuang
