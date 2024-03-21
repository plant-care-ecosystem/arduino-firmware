enum postType {
  POST,
  GET,
  PUT,
  UNKNOWN
};

uint32_t postHandler(String msg, postType httptype, String postLocation) {
  
  
  // Serial.println("\nStarting connection to server...");
  // if (!client.connect(server, 443))
  //   Serial.println("Connection failed!");
  // else {
  //   Serial.println("Connected to server!");
  // }

  switch (httptype)
    case PUT:
    {
      Serial.println();
      Serial.println("HTTPS Request String:");
      // Create request string
      String request = String("PUT ") + postLocation + String(" HTTP/1.1\r\n") +
                  String("Host: ") + String(server) + String("\r\n") +
                  String("Content-Type: application/json\r\n") +
                  String("Content-Length: ") + String(msg.length()) + String("\r\n") + String("\r\n") +
                  // String("Connection: close\r\n\r\n") +
                  msg;
      // Print request to serial monitor
      Serial.println(request);
      // Print request to server
      client.print(request);
      client.print("\r\n\r\n");
      // // Read HTTP response status line
      // // String statusLine = client.readStringUntil('\n');
      // // statusLine.trim();  // remove trailing whitespace
      // String statusLine = "";
      // while (client.available()) {
      //   statusLine += client.readStringUntil('\n');
      // }

      // // Check if POST was successful
      // if (statusLine.startsWith("HTTP/1.1 200")) {
      //   Serial.println("POST successful");
      //   Serial.println(statusLine);
      // } else {
      //   Serial.println("POST failed");
      //   Serial.println("Response: " + statusLine);
      // }

      // Read HTTP response status line
      String statusLine = client.readStringUntil('\n');
      statusLine.trim();  // remove trailing whitespace

      // Check if POST was successful
      if (statusLine.startsWith("HTTP/1.1 200")) {
        Serial.println("PUT successful");
      } else {
        Serial.println("PUT failed");
        Serial.println("Response: " + statusLine);
      }

      // // Read the rest of the response and store it in a variable
      // String response = "";
      // while (client.available()) {
      //   response += client.readStringUntil('\n');
      // }
      // Serial.println("Response: " + response);

      String response = "";
      bool headerFinished = false;
      while (client.available()) {
        String line = client.readStringUntil('\n');
        if (!headerFinished && line == "\r") {
          headerFinished = true;
        } else if (headerFinished) {
          response += line;
        }
      }
      response.trim(); // Remove any trailing whitespace
      Serial.println("Response: " + response);

      if (response.startsWith("[") && response.endsWith("]")) {
        response = response.substring(1, response.length() - 1);
      }

      StaticJsonDocument<200> doc;
      deserializeJson(doc, response);
      uint32_t dbId = doc["id"];
      Serial.print("DBID: ");
      Serial.println(dbId);

      return dbId;
    }
