/* Copyright 2020, Edwin Chiu

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*
  Basic test and demo software for COVID-19/ARDS Ventilator

  Objective is to make a minimum-viable ARDS ventilator that can be deployed
  or constructed on-site in countries underserved by commecial global supply
  chains during the COVID-19 outbreak.

  Currently consists of a (1) CPAP-style blower with speed control.

  (2)Feedback from a differential pressure sensor with one side
  measuring delivered pressure to patient, other side ambient.


  created 16 Mar 2020
  Edwin Chiu
  Frost Methane Labs/Fix More Lungs
  Based on example code by Tom Igoe and Brett Beauregard

  Project Description: http://bit.ly/2wYqj3X
  git: https://github.com/RespiraWorks/VentilatorSoftware
  http://respira.works

  Outputs can be plotted with Cypress PSoC Programmer (Bridge Control Panel
  Tool) Download and install, connect serial Tools > Protocol Configuration >
  serial 115200:8n1 > hit OK In editor, use command RX8 [h=43] @1Key1 @0Key1
  @1Key2 @0Key2 Chart > Variable Settings Tick both Key1 and Key2, configure as
  int, and choose colors > hit OK Press >|< icon to connect to com port if
  necessary Click Repeat button, go to Chart tab both traces should now be
  plotting

*/

#include "alarm.h"
#include "blower_fsm.h"
#include "blower_pid.h"
#include "comms.h"
#include "hal.h"
#include "network_protocol.pb.h"
#include "sensors.h"
#include "solenoid.h"

// Current controller status.  Updated when we receive data from the GUI, when
// sensors read data, etc.
static ControllerStatus controller_status = ControllerStatus_init_zero;

// Last-received status from the GUI.
static GuiStatus gui_status = GuiStatus_init_zero;

static void controller_loop() {
  while (true) {
    controller_status.uptime_ms = Hal.millis();
    comms_handler(controller_status, &gui_status);
    controller_status.active_params = gui_status.desired_params;

    Pressure setpoint =
        blower_fsm_get_setpoint(controller_status.active_params);
    controller_status.fan_setpoint_cm_h2o = setpoint.cmH2O();

    blower_pid_execute(setpoint, &controller_status.sensor_readings,
                       &controller_status.fan_power);
    Hal.watchdog_handler();
  }
}

void setup() {
  // Initialize Hal first because it initializes the watchdog. See comment on
  // HalApi::init().
  Hal.init();

  comms_init();
  alarm_init();
  sensors_init();
  solenoid_init();
  blower_fsm_init();
  blower_pid_init();

  controller_loop();
}

void loop() {
  // Dummy placeholder as Arduino framework requires it to compile
}
