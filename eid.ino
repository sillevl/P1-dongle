boolean eidUpload(){
  boolean eidSuccess = false;
  if(!clientSecureBusy){
    boolean mqttPaused;
    if(mqttclientSecure.connected()){
      Serial.println("Disconnecting TLS MQTT connection");
      mqttclientSecure.disconnect();
      mqttPaused = true;
    }
    if(bundleLoaded){
      Serial.print("Making EID data push, channel ");
      for(int chan = 0; chan < 5; chan++){
        Serial.print(chan);
        Serial.print(" ");
        if (https.begin(*client, eid_webhook)) {  // HTTPS
          https.addHeader("Content-Type", "application/json");
          // Send HTTP POST request
          int httpCode = https.POST(eidData(chan));
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            if (httpCode == HTTP_CODE_OK ) {
              eidSuccess = true;
              eidError = false;
            }
            else {
              Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
              eidError = true;
            }
          } 
          else {
            Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
            eidError = true;
          }
          https.end();
          client->stop();
        } 
        else {
          Serial.printf("[HTTPS] Unable to connect\n");
          eidError = true;
        }
      }
      if(!eidError) Serial.println("");
    }
    //client->stop();
    if(mqttPaused){
      sinceConnCheck = 10000;
    }
  }
  return eidSuccess;
}
String eidData(int channel) {
  DynamicJsonDocument postData(1024);
  JsonArray dataArray = postData.createNestedArray("data");
  JsonArray dataPoint = dataArray.createNestedArray();
  char timeStringBuff[50]; 

  if(channel < 4){
    postData["unit"] = "kWh";
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%dT%H:%M:%SZ", &dm_time);
  }
  else{
    postData["unit"] = "m³";
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%dT%H:%M:%SZ", &mb1_time);
  }
  String timeString(timeStringBuff);
  dataPoint.add(timeString);
  if(channel == 0){
    postData["remoteId"] = "p1-dongle-total-consumption-day";
    postData["remoteName"] = "Electricity consumption day";
    dataPoint.add(totConDay);
  }
  else if(channel == 1){
    postData["remoteId"] = "p1-dongle-total-consumption-night";
    postData["remoteName"] = "Electricity consumption night";
    dataPoint.add(totConNight);
  }
  else if(channel == 2){
    postData["remoteId"] = "p1-dongle-total-injection-day";
    postData["remoteName"] = "Electricity injection day";
    dataPoint.add(totInDay);
  }
  else if(channel == 3){
    postData["remoteId"] = "p1-dongle-total-injection-night";
    postData["remoteName"] = "Electricity injection night";
    dataPoint.add(totInNight);
  }
  else if(channel == 4){
    postData["remoteId"] = "p1-dongle-total-gas-consumption";
    postData["remoteName"] = "Natural gas consumption";
    dataPoint.add(totGasCon);
  }
  if(channel < 2)  postData["metric"] = "electricityImport";
  else if (channel >= 2 && channel < 4) postData["metric"] = "electricityExport";
  else postData["metric"] = "naturalGasImport";
  postData["readingType"] = "counter";
  String output;
  serializeJson(postData, output);
  return output;
}
