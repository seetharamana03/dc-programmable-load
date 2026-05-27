// #include <Wire.h>
// #include <Adafruit_ADS1X15.h>
// #include <Adafruit_MCP4725.h>

// Adafruit_ADS1115 ads;
// Adafruit_MCP4725 dac;

// // Constants
// const float SHUNT_RESISTANCE = 1.0;   // 1 ohm sense resistor
// const float MAX_CURRENT = 2.0;         // Safety limit: 2A max
// const float VCC = 5.0;                 // Arduino supply voltage
// const int   DAC_MAX = 4095;

// // // Safety: Max DAC value allowed (limits gate drive = limits current)
// // // Tune this based on your MOSFET characterisation
// // const int DAC_SAFE_LIMIT = 2000;

// // void setup() {
// //   Serial.begin(9600);
// //   Serial.println("Programmable DC Load - Phase 2b: MOSFET Gate Control");

// //   // Initialize ADS1115
// //   if (!ads.begin()) {
// //     Serial.println("ERROR: ADS1115 not found!");
// //     while (1);
// //   }
// //   ads.setGain(GAIN_ONE);
// //   Serial.println("ADS1115 ready.");

// //   // Initialize MCP4725 DAC
// //   if (!dac.begin(0x60)) {
// //     Serial.println("ERROR: MCP4725 not found!");
// //     while (1);
// //   }
// //   Serial.println("MCP4725 ready.");

// //   // Start with gate off (DAC = 0, MOSFET off)
// //   dac.setVoltage(0, false);
// //   Serial.println("Gate set to 0. MOSFET OFF.");
// //   Serial.println("-------------------------------------------------------");
// //   Serial.println("DAC Val | Gate V | Current(A) | Voltage(V) | Power(W)");
// //   Serial.println("-------------------------------------------------------");

// //   delay(1000);
// // }

// // void loop() {
// //   // Gradually increase DAC value to increase gate voltage and sink more current
// //   for (int dacValue = 0; dacValue <= DAC_SAFE_LIMIT; dacValue += 100) {

// //     // Set DAC output -> goes to LM358 IN+ -> LM358 OUT -> 100ohm -> MOSFET Gate
// //     dac.setVoltage(dacValue, false);

// //     delay(100); // Allow circuit to settle

// //     // Read shunt voltage (differential A0-A1)
// //     int16_t shuntRaw = ads.readADC_Differential_0_1();
// //     float shuntVoltage = ads.computeVolts(shuntRaw);
// //     float current = shuntVoltage / SHUNT_RESISTANCE;

// //     // Read supply voltage (single ended A2)
// //     int16_t voltRaw = ads.readADC_SingleEnded(2);
// //     float supplyVoltage = ads.computeVolts(voltRaw);

// //     // Calculate power
// //     float power = supplyVoltage * current;

// //     // Approximate gate voltage from DAC value
// //     float gateVoltage = (dacValue / 4095.0) * VCC;

// //     // Print readings
// //     Serial.print(dacValue);
// //     Serial.print("    | ");
// //     Serial.print(gateVoltage, 2);
// //     Serial.print("V   | ");
// //     Serial.print(current, 4);
// //     Serial.print("A     | ");
// //     Serial.print(supplyVoltage, 3);
// //     Serial.print("V      | ");
// //     Serial.print(power, 4);
// //     Serial.println("W");

// //     // Safety check - cut off if current exceeds limit
// //     if (current > MAX_CURRENT) {
// //       Serial.println("!!! OVERCURRENT DETECTED - Shutting down gate !!!");
// //       dac.setVoltage(0, false);
// //       while (1); // Halt
// //     }
// //   }

// //   // Ramp back down safely
// //   dac.setVoltage(0, false);
// //   Serial.println("-------------------------------------------------------");
// //   Serial.println("Sweep complete. MOSFET OFF. Restarting in 3 seconds...");
// //   delay(3000);
// // }

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_MCP4725.h>

Adafruit_ADS1115 ads;
Adafruit_MCP4725 dac;

const float SHUNT_RESISTANCE = 1.0;
const float MAX_CURRENT      = 1.8;    // lower than 2.0 for safety margin
const float DIVIDER_RATIO    = 2.0;    // R1=R2, so supply = reading * 2
const int   DAC_SAFE_LIMIT   = 4000;   // START LOW — raise after seeing the curve
const int   DAC_STEP         = 50;     // finer steps = better curve

void emergencyStop(const char* reason, int dacValue, float current) {
    dac.setVoltage(0, false);
    delay(10);
    dac.setVoltage(0, false);
    Serial.print("!!! ");
    Serial.print(reason);
    Serial.print(" at DAC=");
    Serial.print(dacValue);
    Serial.print(" I=");
    Serial.print(current, 3);
    Serial.println("A");
    Serial.println("Cooling 5s before retry...");
    delay(5000);
}

float readCurrent() {
    long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += ads.readADC_Differential_0_1();
    }
    float vshunt = ads.computeVolts(sum / 8);
    return vshunt / SHUNT_RESISTANCE;
}

float readSupplyVoltage() {
    long sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += ads.readADC_SingleEnded(2);
    }
    return ads.computeVolts(sum / 4) * DIVIDER_RATIO;
}

void setup() {
    Serial.begin(9600);
    Serial.println("Phase 2b - MOSFET Characterisation Sweep");

    if (!ads.begin())     { Serial.println("ERROR: ADS1115"); while(1); }
    ads.setGain(GAIN_ONE);

    if (!dac.begin(0x60)) { Serial.println("ERROR: MCP4725"); while(1); }
    dac.setVoltage(0, false);

    Serial.println("Ready. Starting in 2s.");
    delay(2000);

    Serial.println("DAC,Current_A,Supply_V,Power_W");  // CSV header
}

void loop() {
    for (int dacValue = 0; dacValue <= DAC_SAFE_LIMIT; dacValue += DAC_STEP) {
        dac.setVoltage(dacValue, false);

        // Fast safety check
        delay(15);
        float quickI = readCurrent();
        if (quickI > MAX_CURRENT) {
            emergencyStop("OVERCURRENT (fast)", dacValue, quickI);
            return;
        }

        // Settle, then proper measurement
        delay(85);
        float current = readCurrent();
        float supplyV = readSupplyVoltage();
        float power   = supplyV * current;

        if (current > MAX_CURRENT) {
            emergencyStop("OVERCURRENT", dacValue, current);
            return;
        }

        // CSV-formatted output (easy to parse)
        Serial.print(dacValue);  Serial.print(",");
        Serial.print(current, 4); Serial.print(",");
        Serial.print(supplyV, 3); Serial.print(",");
        Serial.println(power, 4);
    }

    dac.setVoltage(0, false);
    Serial.println("# Sweep complete. Restarting in 5s.");
    delay(5000);
}