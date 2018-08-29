# wyostat
Open source thermostat

* install node-red
           * https://nodered.org/docs/hardware/raspberrypi
* install node-red-dashboard
   	   * (from the ~/.node-red)
	   * https://flows.nodered.org/node/node-red-dashboard
* install node-red start up trigger: npm install node-red-contrib-startup-trigger
* install mosquitto (steps 1 & 2
           *https://learn.adafruit.com/diy-esp8266-home-security-with-lua-and-mqtt/configuring-mqtt-on-the-raspberry-pi)
* start node-red (from the command-line type $node-red)
* start mosquitto (from the command line type $moquitto)
* edit OpenThermostat.json from within node-red (update mqtt-broker ip address)
* deploy OpenThermostat.json from within node-red (update mqtt-broker ip address)
* Copy and edit arduino/OpenThermostat/README to arduino/OpenThermostat/config.h
* upload OpenThermostat.ino
