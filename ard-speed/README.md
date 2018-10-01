# Speed sensor

Using information from the IR-sensor located currently on the front left-wheel of the car we can calculate its speed. The wheel has five strips of relctive tape which when the IR-sensor is focused on emits a high-signal. This generates an interrupt in the arduino which icrements a counter (further refered to as a step). To do the speed calculations we first calculate the distance traveled. This is done by taking the amount of steps times the distance of one step. Five steps on one rotation of the wheel gives us the circumference of the wheel divided by five. The speed is then calculated by dividing the distance by time. The general formula can be expressed as follows.

```
speed = (steps * DISTANCE_PER_STEP) / elapsedTime
```


## Dependencies

The code depends on an arudino library called "SimpleTimer", the download can be found [here](https://playground.arduino.cc/code/SimpleTimer
)

**Optional dependencies**

To be able to use the Output tool _cat_ is needed.

To be able to use the Communication tool _screen_ is needed.

To be able to use the Plot tool _feedgnuplot_ is required and can be aquired [here](https://github.com/dkogan/feedgnuplot).

## Building

To build and run the program a makefile is provided. An important note is that the PORT variable may need to be changed to the specific port on your system that the arudino is connected to.

| Tool          | Command       | Desription |
|---------------|---------------|------------|
| Verify        | make verfiy   | Compiles and verifies the file without uploading to board (doesnt require a board connection) |
| Upload        | make upload   | Compiles and verifies the file and then procedes to upload it to the board |
| Output        | make output   | Outputs the serial communications received from the arduino in |
| Plot          | make plot     | Outputs an ascii-plot in the terminal of numbers received on the serial communication line |
| Communication | make communication | Opens a screen-session to enable for 2-way serial communication with the arduino |

