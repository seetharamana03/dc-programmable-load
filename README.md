# DC Programmable Load

A programmable DC electronic load designed to regulate the current drawn from an external power source. The system combines an Arduino Nano, current-sensing circuitry, a power MOSFET, analog signal conditioning, digital measurement, and closed-loop control on a custom PCB.

This project was developed as a hands-on platform for learning power electronics, embedded control, PCB design, sensing, calibration, thermal management, and circuit protection.

## Project Overview

A DC electronic load acts as a controllable current sink. Instead of powering a circuit, it draws a commanded amount of current from a device under test.

This makes the system useful for testing:

* Batteries
* DC power supplies
* Voltage regulators
* USB power modules
* Solar power systems
* Embedded power rails
* Power-conversion circuits

The current version is primarily designed to operate in **constant-current mode**.

The user selects a target current, and the controller continuously adjusts the MOSFET gate drive until the measured current matches the requested value.

## Key Features

* Constant-current load control
* Arduino Nano-based firmware
* Closed-loop PI control
* Programmable analog reference using an MCP4725 DAC
* High-resolution measurements using an ADS1115 ADC
* MOSFET-based current sinking
* LM358 analog signal conditioning
* Current, voltage, and power monitoring
* MCP9808 temperature sensing
* Serial telemetry for testing and calibration
* Custom KiCad schematic and PCB
* External 12 V input
* Thermal and overcurrent protection architecture
* Reverse-polarity protection architecture
* USB connection for programming and serial communication

## System Architecture

```text
                  External DC Source
                         |
                         v
              Device Under Test Input
                         |
                         v
                 Power MOSFET Stage
                         |
                         v
               Current-Sense Circuit
                         |
                         v
                   ADS1115 ADC
                         |
                         v
                   Arduino Nano
                  /      |       \
                 /       |        \
                v        v         v
          MCP4725 DAC  MCP9808   Serial Output
                |
                v
        Analog Control / Gate Drive
                |
                +------> Power MOSFET
```

## How It Works

The load controls how much current flows through a power MOSFET.

The basic control loop is:

1. The user provides a current setpoint.
2. The Arduino reads the measured load current.
3. The controller calculates the difference between the requested and measured current.
4. A PI controller updates the control command.
5. The MCP4725 DAC produces an analog reference voltage.
6. The analog control stage adjusts the MOSFET gate voltage.
7. The MOSFET changes the current drawn from the device under test.
8. The loop repeats until the measured current approaches the setpoint.

## Constant-Current Control

The current error is defined as:

```text
error = requested current - measured current
```

A proportional-integral controller calculates the output command:

```text
control output = Kp × error + Ki × accumulated error
```

The proportional term responds immediately to current error.

The integral term removes persistent steady-state error caused by device tolerances, MOSFET behavior, measurement offset, and analog circuit losses.

A practical implementation should also include:

* Integrator clamping
* DAC output limits
* Maximum current limits
* Temperature-based shutdown
* Sensor-fault detection
* Safe startup behavior

## Hardware

### Arduino Nano

The Arduino Nano serves as the primary controller.

Responsibilities include:

* Reading voltage and current measurements
* Reading temperature
* Calculating power
* Running the PI control loop
* Updating the DAC command
* Enforcing protection limits
* Reporting telemetry over serial

### IRF3205 Power MOSFET

The IRF3205 acts as the primary current-sinking device.

In this application, the MOSFET may operate in its linear region rather than functioning only as an on/off switch. This causes it to dissipate significant power as heat.

The approximate MOSFET dissipation is:

```text
P_MOSFET ≈ V_DS × I_LOAD
```

For example:

```text
12 V × 2 A = 24 W
```

That level of power requires substantial thermal management.

A heatsink, thermal interface material, airflow, and conservative operating limits are essential.

> The IRF3205 is commonly specified for switching applications. Its safe operating area must be checked carefully before using it continuously in linear mode.

### MCP4725 DAC

The MCP4725 is a 12-bit digital-to-analog converter controlled over I²C.

It converts the Arduino’s digital control command into an analog reference voltage for the MOSFET control circuitry.

The ideal DAC voltage is:

```text
V_DAC = DAC_code / 4095 × V_REF
```

For a 5 V reference:

```text
V_DAC = DAC_code / 4095 × 5.0
```

The DAC output should always be limited in software to prevent the load from requesting an unsafe current.

### ADS1115 ADC

The ADS1115 is a high-resolution analog-to-digital converter used to measure system signals.

It may be used for:

* Current-sense voltage
* Device-under-test voltage
* MOSFET voltage
* Additional diagnostic measurements

Using an external ADC provides better measurement resolution than the Arduino Nano’s built-in ADC.

### LM358 Operational Amplifier

The LM358 provides analog signal conditioning and may be used to:

* Amplify the shunt voltage
* Buffer the DAC reference
* Compare measured current with the command
* Drive or condition the MOSFET gate signal

The op-amp output range, input common-mode range, bandwidth, and supply voltage must be considered when determining the usable control range.

### MCP9808 Temperature Sensor

The MCP9808 monitors board or heatsink temperature.

Temperature data can be used to implement:

* Fan control
* Thermal warnings
* Current derating
* Emergency shutdown
* Recovery hysteresis

The sensor should be mounted near the MOSFET or heatsink for meaningful thermal protection.

### Current-Sense Resistor

The load current is measured using the voltage across a low-value shunt resistor.

Ohm’s law gives:

```text
I_LOAD = V_SHUNT / R_SHUNT
```

The shunt power dissipation is:

```text
P_SHUNT = I_LOAD² × R_SHUNT
```

The resistor must be rated for both the expected power and any transient overload conditions.

A Kelvin connection is recommended to reduce measurement error caused by PCB trace resistance.

## Power Measurement

The system can calculate input power using:

```text
P_LOAD = V_INPUT × I_LOAD
```

Serial telemetry can report:

* Input voltage
* Load current
* Load power
* Current setpoint
* Control error
* DAC command
* MOSFET or board temperature
* Fault state

## Example Serial Output

```text
Voltage: 11.98 V
Current: 1.50 A
Power: 17.97 W
Setpoint: 1.50 A
Error: 0.00 A
Temperature: 48.2 C
Status: ACTIVE
```

A comma-separated output format can also be used with the Arduino Serial Plotter:

```text
setpoint,current,voltage,power,temperature,dac_output
```

## PCB Design

The repository includes a custom KiCad design for the programmable load.

The design integrates the primary control, sensing, and power components onto one board.

The PCB should be reviewed for:

* High-current trace width
* MOSFET heat dissipation
* Shunt-resistor placement
* Kelvin sensing
* Analog and digital grounding
* Power-ground return paths
* Op-amp signal routing
* I²C pull-up resistors
* Decoupling capacitors
* Connector current ratings
* Creepage and clearance
* Reverse-polarity protection
* Test-point accessibility
* Thermal relief and copper area
* Mounting-hole placement

## Suggested Repository Structure

```text
dc-programmable-load/
├── firmware/
│   └── dc_programmable_load.ino
│
├── kicad-files/
│   └── dc_programmable_load/
│       ├── dc_programmable_load.kicad_pro
│       ├── dc_programmable_load.kicad_sch
│       └── dc_programmable_load.kicad_pcb
│
├── gerbers/
│   └── manufacturing outputs
│
├── documentation/
│   ├── development-notes.md
│   └── calibration.md
│
├── images/
│   ├── pcb-render.png
│   └── assembled-load.jpg
│
├── bom/
│   └── bill-of-materials.csv
│
└── README.md
```

## Firmware Dependencies

Depending on the current firmware implementation, the Arduino project may require libraries for:

* MCP4725 DAC
* ADS1115 ADC
* MCP9808 temperature sensor
* I²C communication

Typical Arduino libraries include:

```text
Adafruit MCP4725
Adafruit ADS1X15
Adafruit MCP9808
Wire
```

Install the required libraries through the Arduino Library Manager.

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/seetharamana03/dc-programmable-load.git
cd dc-programmable-load
```

### 2. Review the Hardware

Before applying power:

* Inspect the schematic
* Confirm component orientation
* Check for solder bridges
* Verify MOSFET pinout
* Verify op-amp pinout
* Check the shunt-resistor value
* Confirm the regulator output
* Confirm the I²C pull-ups
* Check continuity between power and ground
* Check that the heatsink is installed

### 3. Install Arduino Libraries

Install the required sensor, DAC, and ADC libraries through the Arduino IDE.

### 4. Connect the Arduino

Use the USB connection for:

* Firmware upload
* Serial commands
* Telemetry
* Debugging

USB should not be treated as the main power source for the load’s power stage.

### 5. Apply Auxiliary Power

Connect the correct external supply to the board’s power input.

Confirm all regulated voltage rails before connecting a device under test.

### 6. Upload the Firmware

Open the Arduino firmware and select:

```text
Board: Arduino Nano
Processor: Correct bootloader version
Port: Connected serial port
```

Upload the firmware and open the Serial Monitor at the configured baud rate.

## Calibration

Calibration is required for accurate current and voltage readings.

### Current Calibration

1. Connect a known load or precision current meter.
2. Command several current values across the intended range.
3. Record the ADC reading and measured current.
4. Calculate gain and offset correction.
5. Store the calibration coefficients in firmware.
6. Repeat at low and high current.

A basic correction model is:

```text
I_actual = current_gain × I_measured + current_offset
```

### Voltage Calibration

1. Apply several known input voltages.
2. Compare the reported voltage with a calibrated multimeter.
3. Calculate gain and offset corrections.
4. Store the resulting coefficients.

```text
V_actual = voltage_gain × V_measured + voltage_offset
```

### DAC Calibration

The DAC output may differ slightly from its ideal value because of:

* Supply variation
* DAC tolerance
* Op-amp offset
* Resistor tolerance
* Ground offset

Measure the actual DAC output for several codes and compensate in firmware when necessary.

## PI Controller Tuning

A practical tuning process is:

1. Set `Ki` to zero.
2. Begin with a very small `Kp`.
3. Apply a low current setpoint.
4. Increase `Kp` until the system responds quickly without excessive oscillation.
5. Add a small `Ki` value to remove steady-state error.
6. Test across different input voltages and load currents.
7. Confirm that the controller remains stable near zero current.
8. Test response to abrupt setpoint changes.
9. Test response to input-voltage changes.
10. Verify that current remains within safe limits during startup and faults.

Do not begin controller tuning at the system’s maximum current or power.

## Recommended Protection States

The firmware should use explicit system states such as:

```text
DISABLED
STARTUP
ACTIVE
OVERCURRENT
OVERTEMPERATURE
UNDERVOLTAGE
SENSOR_FAULT
EMERGENCY_STOP
```

In every fault state, the DAC output should be forced to zero or another verified safe value.

## Safety

This circuit intentionally converts electrical energy into heat. Improper operation can damage the device under test, MOSFET, PCB, wiring, or power supply.

Use the following precautions:

* Begin testing with a current-limited bench supply
* Start with low voltage and low current
* Install an appropriately rated heatsink
* Use forced-air cooling when required
* Verify the MOSFET safe operating area
* Do not touch the MOSFET or heatsink during operation
* Use properly rated connectors and wiring
* Add a fuse at the input
* Add hardware overcurrent protection
* Add reverse-polarity protection
* Add thermal shutdown
* Ensure the control output defaults to zero during reset
* Never rely exclusively on software for protection
* Keep flammable materials away from the load
* Do not leave the system unattended under high power
* Disconnect power before changing wiring
* Verify polarity before connecting a battery

## Important Thermal Limitation

The system’s safe operating range is determined by **power dissipation**, not only current.

A load that can safely sink 3 A at 2 V may not safely sink 3 A at 12 V.

Always calculate:

```text
P ≈ V × I
```

The maximum permissible power depends on:

* MOSFET safe operating area
* Junction-to-case thermal resistance
* Heatsink thermal resistance
* Ambient temperature
* Airflow
* PCB copper area
* Shunt-resistor rating
* Connector rating
* Enclosure design

## Current Limitations

* Constant-current mode is the primary operating mode
* Safe power depends heavily on cooling
* MOSFET linear-mode capability must be validated
* Calibration is required for accurate measurements
* No isolated input or communication interface
* Protection behavior may still be under development
* Current capacity is limited by the MOSFET, shunt, PCB, and thermal system
* The Arduino Nano does not provide deterministic hard real-time protection
* Board revisions may differ from the firmware assumptions
* The system is not a certified commercial test instrument

## Future Improvements

* Add constant-power mode
* Add constant-resistance mode
* Add constant-voltage mode
* Add an OLED or LCD interface
* Add rotary-encoder control
* Add USB serial commands
* Add programmable load profiles
* Add data logging
* Add automatic calibration
* Add MOSFET temperature feedback
* Add closed-loop fan control
* Add hardware current cutoff
* Add input fuse protection
* Add reverse-polarity MOSFET protection
* Add transient suppression
* Add multiple parallel MOSFET channels
* Add isolated measurement
* Add a dedicated gate-control amplifier
* Add an enclosure and front panel
* Add a desktop control application
* Add automated characterization tests
* Add a physics-based or SPICE simulation

## Educational Goals

This project can be used to study:

* Power MOSFET operation
* MOSFET linear-region behavior
* Safe operating area
* Current sensing
* Operational-amplifier circuits
* ADC and DAC interfacing
* I²C communication
* PI control
* Feedback-loop stability
* Embedded firmware
* Sensor calibration
* Thermal design
* PCB layout
* Circuit protection
* Hardware testing and debugging

## Disclaimer

This project is an experimental educational electronic load and is not a calibrated or certified commercial instrument.

Verify all voltage, current, thermal, and power limits independently before connecting expensive equipment or batteries.

## License

No license has currently been specified.

Add a license before allowing others to reuse, modify, manufacture, or distribute the design.