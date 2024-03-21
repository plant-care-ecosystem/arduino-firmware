#ifndef REQUEST_H
#define REQUEST_H

#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <map>

/**
  * @enum postType
  * @brief Enumeration for different types of HTTP requests.
  */
enum requestType {
  POST,    /**< POST request */
  GET,     /**< GET request */
  PUT     /**< PUT request */
};

enum requestLocation {
  PLANT,    /**< POST request */
  SENSOR,     /**< GET request */
  DATA,     /**< PUT request */
  LOGIN
};

/**
 * @class HttpHandler
 * @brief Handles HTTP requests and sends them to a server.
 */
class HttpsHandler {
public:
  // This array stores the strings associated with each RequestType
  std::map<requestLocation, String> requestStrings = {
    {PLANT, "/plant"},
    {SENSOR, "/bensor"},
    {DATA, "/sensor/data"},
    {LOGIN, "/login"}
  }; 

  /**
   * @brief Constructs an HttpHandler object with the specified server.
   * @param server The server to send the requests to.
   */
  HttpsHandler() {}

  void setServer(String server) {
    this->server = server;
    // Set insecure for now
    // TODO: CHANGE TO SECURE WITH CERTIFICATES
    client.setInsecure();
    // Connect to the server
    while(!checkConnected()) {
      Serial.println("Failed to connect to server. Retrying...");
      delay(1000);
    }
  }

  /**
   * @brief Handles the HTTP request based on the request type.
   * @param msg The message to be sent in the request.
   * @param httpType The type of the HTTP request (POST, GET, PUT).
   * @param requestLocation The location of the request (PLANT, SENSOR, DATA).
   * @return The result of the request handling, -1 upon error.
   */
  uint32_t requestHandler(String msg, requestType httpType, requestLocation requestLocation) {
    // Check the server connection
    while(!checkConnected()) {
      Serial.println("Failed to connect to server. Retrying...");
      delay(1000);
    }
    
    String method;
    switch (httpType) {
        case PUT:
            method = "PUT";
            break;
        case POST:
            method = "POST";
            break;
        case GET:
            method = "GET";
            break;
        default:
          // Unknown request type, return an error
          return -1;
    }
    // Get requestString from map
    String requestString = requestStrings[requestLocation];

    // Create the proper request string
    String request = createRequestString(method, requestString, msg);
    // Print the request for debugging
    printRequest(request);
    // Send the request to the server
    client.print(request);
    // Read the server response and return the dbId
    return readResponse();
  }

private:
  String server; // The server to send the requests to.
  WiFiClientSecure client; // The client to send the requests with.
  
  /**
   * @brief Creates the request string for the HTTP request.
   * @param method The HTTP method (e.g., GET, POST, PUT).
   * @param requestLocation The location of the request.
   * @param msg The message to be sent in the request.
   * @return The request string.
   */
  String createRequestString(String method, String requestLocation, String msg) {
    return String(method + " ") + requestLocation + String(" HTTP/1.1\r\n") +
          String("Host: ") + server + String("\r\n") +
          String("Content-Type: application/json\r\n") +
          String("Content-Length: ") + String(msg.length()) + String("\r\n\r\n") +
          msg + String("\r\n\r\n");
  }

  /**
   * @brief Print the HTTP request string to the serial monitor for debugging.
   * @param request The HTTP request string to be printed.
   */
  void printRequest(String request) {
    Serial.println();
    Serial.println("HTTP Request String:====================");
    Serial.println(request);
    Serial.println("========================================");
    Serial.println();
  }

  /**
   * @brief Checks if the client is connected to the server.
   * 
   * This function checks if the client is connected to the server. If the client is not connected,
   * it attempts to establish a connection with the server. The server address and port number are
   * specified by the `server` variable.
   * 
   * @return 1 if the client is connected to the server, 0 otherwise.
   */
  int checkConnected() {
    if (!client.connected()) {
      if (!client.connect(server.c_str(), 443)) {
          return 0;
      }
    }
    return 1;
  }

  // Read response from server and return dbId
  /**
   * @brief Reads the response from the server after sending a request.
   * 
   * This function reads the HTTP response status line and the full JSON response from the server.
   * It checks if the request was successful and prints the response for debugging purposes.
   * It also parses the response and retrieves the dbId.
   * 
   * @return The dbId retrieved from the response.
   */
  uint32_t readResponse() {
    // Read HTTP response status line
    String statusLine = client.readStringUntil('\n');
    statusLine.trim();  // remove trailing whitespace

    // Check if request was successful for debugging
    if (statusLine.startsWith("HTTP/1.1 200")) {
      Serial.println("Request successful!");
    } else {
      Serial.println("Request failed!");
      Serial.println("Response: " + statusLine);
    }

    // Read full JSON response from server
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
    response.trim(); // Remove trailing whitespace
    // Print the response for debugging
    Serial.println("Response: " + response);

    // Remove brackets from response
    if (response.startsWith("[") && response.endsWith("]")) {
      response = response.substring(1, response.length() - 1);
    }

    // Parse the response and get the dbId
    StaticJsonDocument<200> doc;
    deserializeJson(doc, response);
    uint32_t dbId = doc["id"];
    // Print dbId for debugging
    // Serial.print("DBID: ");
    // Serial.println(dbId);

    return dbId;
  }
};

#endif REQUEST_H







// enum postType {
//   POST,
//   GET,
//   PUT,
//   UNKNOWN
// };

// uint32_t postHandler(String msg, postType httptype, String postLocation) {
  
  
//   // Serial.println("\nStarting connection to server...");
//   // if (!client.connect(server, 443))
//   //   Serial.println("Connection failed!");
//   // else {
//   //   Serial.println("Connected to server!");
//   // }

//   switch (httptype)
//     case PUT:
//     {
//       Serial.println();
//       Serial.println("HTTPS Request String:");
//       // Create request string
//       String request = String("PUT ") + postLocation + String(" HTTP/1.1\r\n") +
//                   String("Host: ") + String(server) + String("\r\n") +
//                   String("Content-Type: application/json\r\n") +
//                   String("Content-Length: ") + String(msg.length()) + String("\r\n") + String("\r\n") +
//                   // String("Connection: close\r\n\r\n") +
//                   msg;
//       // Print request to serial monitor
//       Serial.println(request);
//       // Print request to server
//       client.print(request);
//       client.print("\r\n\r\n");
//       // // Read HTTP response status line
//       // // String statusLine = client.readStringUntil('\n');
//       // // statusLine.trim();  // remove trailing whitespace
//       // String statusLine = "";
//       // while (client.available()) {
//       //   statusLine += client.readStringUntil('\n');
//       // }

//       // // Check if POST was successful
//       // if (statusLine.startsWith("HTTP/1.1 200")) {
//       //   Serial.println("POST successful");
//       //   Serial.println(statusLine);
//       // } else {
//       //   Serial.println("POST failed");
//       //   Serial.println("Response: " + statusLine);
//       // }

//       // Read HTTP response status line
//       String statusLine = client.readStringUntil('\n');
//       statusLine.trim();  // remove trailing whitespace

//       // Check if POST was successful
//       if (statusLine.startsWith("HTTP/1.1 200")) {
//         Serial.println("PUT successful");
//       } else {
//         Serial.println("PUT failed");
//         Serial.println("Response: " + statusLine);
//       }

//       // // Read the rest of the response and store it in a variable
//       // String response = "";
//       // while (client.available()) {
//       //   response += client.readStringUntil('\n');
//       // }
//       // Serial.println("Response: " + response);

//       String response = "";
//       bool headerFinished = false;
//       while (client.available()) {
//         String line = client.readStringUntil('\n');
//         if (!headerFinished && line == "\r") {
//           headerFinished = true;
//         } else if (headerFinished) {
//           response += line;
//         }
//       }
//       response.trim(); // Remove any trailing whitespace
//       Serial.println("Response: " + response);

//       if (response.startsWith("[") && response.endsWith("]")) {
//         response = response.substring(1, response.length() - 1);
//       }

//       StaticJsonDocument<200> doc;
//       deserializeJson(doc, response);
//       uint32_t dbId = doc["id"];
//       Serial.print("DBID: ");
//       Serial.println(dbId);

//       return dbId;
//     }
