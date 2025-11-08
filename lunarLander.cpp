#include "Arduino_LED_Matrix.h"
#undef abs
#include <chrono>
#include <cstdint>
#include <random>

ArduinoLEDMatrix matrix;

/*
 * Mock lunar lander, use tasks and threads
 */

 // Struct for lunar and earth re-entry return values
struct LanderReturnValues {
  float distanceFromSurface; // Height
  float currentSpeed; 
};

// Matrix display constants
const std::uint8_t rows = 8;
const std::uint8_t cols = 12;

std::uint8_t displayScreen[rows][cols] = {
  { 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1 }, 
  { 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1 }, 
  { 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1 }, 
  { 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1 }, 
  { 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1 }, 
  { 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1 }, 
  { 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1 }, 
  { 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1 }  
};

// Engine constants
// Engine Thust is for all engines, front, top, bottom, rear, right, and lift sides
const float engineThrust = 10250; // Pounds or something

// Mission time
std::uint32_t missionTime;

// This function will keep track of the time ellapsed since the missions start, this will also help with 
// scheduleing tasks
std::uint32_t missionTimeTicker(){
  missionTime++;
  return missionTime;
}

// Once initiated via the serial port, it will run for 5 minutes and then break back out into a landed mode
// we will need a landing module that will calculate where we will land on the moon as we are orbiting and going down onto it.
LanderReturnValues lunarLander(LanderReturnValues previousValues){
  // Only a priority once we have gotten close to the moon, will be no greather than 
  float moonGravity = 1.62; // m/s^2
  float orbitSpeed = 5500; // km/h
  // Landing protocol, we are currently in orbit of the moon, and we then begin to produce thust from
  // the front and top engines to slow us down to be able to land. We will slowly slow down the forward
  // motion 
  float currentSpeedInOrbit = previousValues.currentSpeed;
  float currentDistanceFromSurface = previousValues.distanceFromSurface;

  // Do math here to drop the lander safely each time we call the function
  // Using the engines burns fuel, we only have so much fuel wink emoji
  LanderReturnValues values;
  values.distanceFromSurface = currentDistanceFromSurface -= 31.392f;
  values.currentSpeed = currentSpeedInOrbit -= 2.573f;

  return values;
}

// This will run once the mission is over
// We will also need a rentry to earth module that will work similarly to  the moon landing one 
// but we will have much more gravity and wind resistence.
void earthReEntry(){

}

// This will run forever
// We will need a heart beat sensor that will constantly send heart beats every 10ms, if we miss 
// more than 2 heart beats consequtivley the system shall alert the user that an error has occured. 
void heartBeatSensor(){
  Serial.println("HEART BEAT");
}

// This will run forever
// We will also want something to keep track of our oxygen supply and how clean
// our air currently is, this will be done by taking the volume of the room and the amount of C02 that 3 persons will emit.
float oxygenSupplyMonitor(float previousOxygenSupply){
  float newOxygenSupply = previousOxygenSupply;
  newOxygenSupply -= 0.459281; // 15 Liters per person * 3 people per hour, 45 liters, we make the math harder by adding junk at the end
  // Update the displayscreen with the oxygen supply
  return newOxygenSupply;
}

// Called by lunar lander or earth re-entry to slow the ship down
float burnEngines(float currentSpeed){

}

// This will run forever
// We will also need a communication module, this will be  the second highest priority as it will allow 
// the pilots to communicate with the ground team. 
void communicationModule(){

}


// Section off the display to show different status for the different modules
void flipDisplayScreen(std::uint8_t flipBit){
  for(std::uint8_t i = 0; i < rows; i++){
    for(std::uint8_t j = 0; j < cols; j++){
      displayScreen[i][j] = flipBit;
    }
  }
}

// Updates the screen based on the module and the values
template<typename T>
void updateDisplayMatrix(char module, T logicValue){
  for(std::uint8_t i = 0; i < rows; i++){
    for(std::uint8_t j = 0; j < cols; j++){
      switch(module){
        case 'O':
          // Define the boundaries for the oxygen monitor screen
          if(j > 8 ){
            float oxygenLevel = static_cast<float>(logicValue);
            // Have all on at start or at refill:
            if (oxygenLevel >= 99.0f) {
              displayScreen[i][j] = 1;
            }
            if (oxygenLevel <= 90.0f) {
              // Have to do some math to reverse the order that it updates the LEDs
              float threshold = std::abs(((oxygenLevel / 100.0f) * rows) - rows);
              displayScreen[i][j] = (i < threshold) ? 0 : 1;
            }
          }
        break;
        case 'H':
          // Heartbeat monitor in the bottom left of the matrix
          if(j < 2 && i >= 5){
            displayScreen[i][j] = static_cast<std::uint8_t>(logicValue);
          }
        break;
        default:
        break;
      }
    }
  }
  // Update the display
  matrix.renderBitmap(displayScreen, rows, cols);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  matrix.begin();
  missionTime = 0;
  matrix.renderBitmap(displayScreen, rows, cols);
}

void loop() {

  float currentOxygenSupply = 100.0;
  float currentDistanceFromLunarSurface = 313000; // 313 KM == 313000 Meters
  bool flipBit = true;

  LanderReturnValues values;
  values.currentSpeed = 5500; // km/h
  values.distanceFromSurface = 313000; // meters

  while(1){
    Serial.println(missionTimeTicker());
    // Introduce the tasks in their priority order
    if(missionTimeTicker() % 10 == 0) { // Heart beat
      flipBit = !flipBit;
      heartBeatSensor();
      updateDisplayMatrix('H', flipBit);
    } 
    if (missionTimeTicker() % 5 == 0) { // Oxygen supply
      currentOxygenSupply = oxygenSupplyMonitor(currentOxygenSupply);
      Serial.print("currentOxygenSupply: ");
      Serial.println(currentOxygenSupply);
      updateDisplayMatrix('O', currentOxygenSupply);
    } 
     if (missionTimeTicker() >= 1500) { // Start lunar lander protocol
      // This should be recursive but break out so that we can continue to 
      // do the other tasks, similar to how oxygen supply works
      values = lunarLander(values); 
      Serial.print("DISTANCE FROM LUNAR SURFACE: ");
      Serial.println(values.distanceFromSurface);
      Serial.print("CURRENT SPEED: ");
      Serial.println(values.currentSpeed);
    }

    // Once we are more than 5000 mission timer ticks start the lunar lander module
    // Landing takes use of the engines and takes 10_000 ticks, then we land for 2500 ticks
    // Once 2500 ticks has ellapsed, start the exit lunar lander, then begin the earth re-entry
    // program and end.
    delay(250);
  }
}
