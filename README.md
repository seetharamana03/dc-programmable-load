# DC Programmable Load Project

## Overview

This project is a DC programmable electronic load designed for students learning embedded systems, analog electronics, PCB design, power electronics, and hardware validation.

A programmable load is a circuit that intentionally draws a controlled amount of current from a power source. It is commonly used to test power supplies, batteries, voltage regulators, USB chargers, and other DC power systems.

The core idea of this project is simple:

```text
The user sets a desired current.
The circuit adjusts a MOSFET gate voltage.
The MOSFET draws current from the source under test.
A sense resistor measures the actual current.
The control loop adjusts until measured current matches the target current.