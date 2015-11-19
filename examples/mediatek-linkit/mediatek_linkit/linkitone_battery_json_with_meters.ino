//
// Copyright 2015, Evothings AB
//
// Licensed under the Apache License, Version 2.0 (the "License")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LinkIt One - Position
//
// Created February, 2015
// 
// Modified by github user stirobot in November, 2015 to expose battery status instead of GPS (for a simpler example)
// This example shows you how to fetch the position and share it using
// a simple webserver.
//

#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiServer.h>
#include <LWiFiClient.h>
#include <LBattery.h>

// Configuration of the WiFi AP settings.
#define WIFI_AP "biz"
#define WIFI_PASSWORD "the dog sat on the fat cat"

// LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define WIFI_AUTH LWIFI_WPA

// Configure the timeout of a http request (ms).
const uint32_t requestTimeout = 1000;

// Global variables
LWiFiServer server(80);
char buff[256];

void setup()
{
  LTask.begin();
  LWiFi.begin();
  Serial.begin(115200);
  //delay(2000);
}

void loop()
{
  static bool wifiStatusPrinted = false;

  connectToAccessPoint(); //straightforward

  if (Serial && wifiStatusPrinted == false)
  {
    printWifiStatus();
    wifiStatusPrinted = true;
  }
  else if(!Serial)
  {
    wifiStatusPrinted = false;
  }

  LWiFiClient client = server.available();

  if (client)
  {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    uint32_t lastReceptionTime = millis();
    while (client.connected())
    {
      if (client.available())
      {
        // we basically ignores client request, but wait for HTTP request end
        int c = client.read();
        lastReceptionTime = millis();

        Serial.print((char)c);
        String batteryLevel = String(LBattery.level());
        String batteryCharging = String(LBattery.isCharging());
        if (c == '\n' && currentLineIsBlank)
        {
          Serial.println("send response");
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Access-Control-Allow-Origin: *");
          client.println();
          client.println("<!DOCTYPE html>");
          client.println("<html><body>");
          client.println("<h3>Simple Sensor Meters Demo</p>");
          client.println("<p>battery(using meter element):  " + batteryLevel + "<meter id=\"bat\" min=\"0\" max=\"100\" value=\"" + batteryLevel + "\"></meter></p>");
          client.println("<p>battery(using progress element): " + batteryLevel + "<progress id=\"batp\" min=\"0\" max=\"100\" value=\"" + batteryLevel + "\"></meter></p>");
          client.println("<p>charging: " + batteryCharging + "</p>"); 
          client.println("<meta http-equiv=\"refresh\" content=\"30\" /></body></html>");
          client.println();
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
      else
      {
        if (millis() - lastReceptionTime > requestTimeout)
        {
          Serial.println("Error - client timeout, dropping connection...");
          break;
        }
      }
    }
    // give the web browser time to receive the data
    delay(500);

    // close the connection:
    Serial.println("close connection");
    client.stop();
    Serial.println("client disconnected");
  }
}


// Helper functions

void connectToAccessPoint()
{

  while (LWiFi.status() != LWIFI_STATUS_CONNECTED)
  {
    if (LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
    {
      server.begin();
      printWifiStatus();
    }
    else
    {
      Serial.println("Error - failed to connect to WiFi");
    }
  }
}

// Helper functions from the WifiWebServer.ino example developed by MediaTek
void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(LWiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = LWiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.print("subnet mask: ");
  Serial.println(LWiFi.subnetMask());

  Serial.print("gateway IP: ");
  Serial.println(LWiFi.gatewayIP());

  // print the received signal strength:
  long rssi = LWiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm\n");
}

