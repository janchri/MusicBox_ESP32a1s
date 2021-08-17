# MusicBox_ESP32a1s
A nice and fancy music box, based on the esp32a1s ai-thinker board using common arduino libaries. closely related to the tonio/espunio projects;

Hardware:
- ESP32a1s AI-Thinker v2.2
- MFRC522 with SPI

Howto:
- Remove the resistors above buttons to get access to SPI bus (VSPI) and connect a common MFRC522

Status:
- NOT FULLY WORKING at the moment, due to migrating to arduino only libraries

ToDos:
- Migrate to ESP8266 Lib
- Add ID3 Tags to Events/Outpus
