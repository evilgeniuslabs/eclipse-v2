uint8_t flipClock = 0;
int timeZone = -6;

unsigned long lastTimeSync = millis();

int oldSecTime = 0;
int oldSec = 0;

IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// send an NTP request to the time server at the given address
void sendNTPpacket()
{
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(timeServerIP, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

time_t getNtpTime()
{
  while (udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket();
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

void drawAnalogClock(byte seconds, byte minutes, byte hours, boolean drawMillis, boolean drawSecond)
{
  if (timeStatus() == timeSet) {
    setSyncInterval(300);
  }
  else {
    setSyncInterval(30);
  }

  if (second() != oldSec) {
    oldSecTime = millis();
    oldSec = second();
  }

  if (hours > 12) hours -= 12;

  int millisecond = millis() - oldSecTime;

  int secondIndex = map(seconds, 0, 59, 0, NUM_LEDS);
  int minuteIndex = map(minutes, 0, 59, 0, NUM_LEDS);
  int hourIndex = map(hours * 5, 5, 60, 0, NUM_LEDS);
  int millisecondIndex = map(secondIndex + millisecond * .06, 0, 60, 0, NUM_LEDS);

  if (millisecondIndex >= NUM_LEDS)
    millisecondIndex -= NUM_LEDS;

  hourIndex += minuteIndex / 12;

  if (hourIndex >= NUM_LEDS)
    hourIndex -= NUM_LEDS;

  // see if we need to reverse the order of the LEDS
  if (flipClock == 1) {
    int max = NUM_LEDS - 1;
    secondIndex = max - secondIndex;
    minuteIndex = max - minuteIndex;
    hourIndex = max - hourIndex;
    millisecondIndex = max - millisecondIndex;
  }

  if (secondIndex >= NUM_LEDS)
    secondIndex = NUM_LEDS - 1;
  else if (secondIndex < 0)
    secondIndex = 0;

  if (minuteIndex >= NUM_LEDS)
    minuteIndex = NUM_LEDS - 1;
  else if (minuteIndex < 0)
    minuteIndex = 0;

  if (hourIndex >= NUM_LEDS)
    hourIndex = NUM_LEDS - 1;
  else if (hourIndex < 0)
    hourIndex = 0;

  if (millisecondIndex >= NUM_LEDS)
    millisecondIndex = NUM_LEDS - 1;
  else if (millisecondIndex < 0)
    millisecondIndex = 0;

  if (drawMillis)
    leds[millisecondIndex] += CRGB(0, 0, 127); // Blue

  if (drawSecond)
    leds[secondIndex] += CRGB(0, 0, 127); // Blue

  leds[minuteIndex] += CRGB::Green;
  leds[hourIndex] += CRGB::Red;
}

void analogClock() {
  dimAll(220);

  drawAnalogClock(second(), minute(), hour(), false, true);
}

void analogClockWithMillis() {
  dimAll(220);

  drawAnalogClock(second(), minute(), hour(), true, true);
}

