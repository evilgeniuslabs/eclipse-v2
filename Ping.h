/*
   ESP8266 + FastLED: https://github.com/jasoncoon/esp8266-fastled-webserver
   Copyright (C) 2015-2020 Jason Coon

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Discovery enables you to find your internet-connected ESP8266 devices on your network.
// When enabled, the ESP8266 will "ping" a server every 10 minutes, by default.
// Then, you can go to https://discover.evilgeniuslabs.org to view all of the
// ESP8266 devices at your current location.

// Data submitted to the server: device name, local IP address, MAC address, and current millis (uptime).
// The server logs this, along with the public request IP address.
// When you visit https://discover.evilgeniuslabs.org, the server returns a list of devices
// with the same public request IP address.

// Discovery is "opt-in". It is disabled by default. Enable it if you want, if you find it useful.
// Evil Genius Labs, despite the name, is not actually evil, and will never sell or share
// this data to any 3rd parties, ever.

// This was a feature I needed for my own devices, of which there are dozens.  :)

const bool discovery = true;
const String serverName = "https://ping.evilgeniuslabs.org";                              // address of server to ping
const String fingerPrint = "AD 1F CB D9 A0 BC 17 D5 5B F2 E1 BF 98 D1 06 CD AC 3F B8 33"; // server SSL cert fingerprint

void checkPingTimer()
{
  if (!discovery)
    return;

  const unsigned long pingDelay = 600000; /// 60 * 10 * 1000; // 10 minutes
  static unsigned long lastPingTime = pingDelay;

  if ((millis() - lastPingTime) > pingDelay)
  {
    // Serial.println("Time to ping");

    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connected, ready to ping");
      HTTPClient http;

      http.begin(serverName, fingerPrint);
      http.addHeader("Content-Type", "application/json");
      String deviceName = "\"deviceName\":\"" + nameString;
      String localIp = WiFi.localIP().toString();
      String macAddress = WiFi.macAddress();
      String body = "{" +
                    deviceName +
                    "\",\"localIp\":\"" + localIp +
                    "\",\"macAddress\":\"" + macAddress +
                    "\",\"millis\":" + String(millis()) +
                    "}";
      Serial.print("Pinging ");
      Serial.print(serverName);
      Serial.print(" with: ");
      Serial.println(body);
      Serial.print("Millis: ");
      Serial.println(millis());
      int httpResponseCode = http.POST(body);
      Serial.print("Millis: ");
      Serial.println(millis());
      Serial.print("Ping response code: ");
      Serial.println(httpResponseCode);
      lastPingTime = millis();
    }
  }
}
