# ESP8266- NodeMCU - MASTER

Source code based on arduino firmware for ESP8266.

How it works

  - 1 - Master receive commands from a web request (via GET or POST)
  - 2 - Send to a slave ESP board
  - 3 - Slave receives the command and execute something

# Master endpoints
Firt of all, you need to connect directly on the ESP wifi (access point) 

> Default network name: Central

> Default network password: 123456789

> Default ESP Board address: 192.168.4.1

After, this endpoints can be called:
   - GET | http://{MASTER_IP_ON_NETWORK}/networks 
      - list all networks seeing by ESP board
    - GET | http://{MASTER_IP_ON_NETWORK}/connect?ssid={SSID}&password={PASSWORD} 
      - Connect ESP to your network
    - GET | http://{MASTER_IP_ON_NETWORK}/credentials 
      - Read the credientials from the EEPROM Memory (just for tests)
    - GET | http://{MASTER_IP_ON_NETWORK}/cmd?to={SLAVE_IP}&cmd={COMMAND_TO_EXECUTE}&value={COMMAND_PARAMS}&extra={EXTRA_PARAMS}
      - Send command to slave ESP connect on the Master
