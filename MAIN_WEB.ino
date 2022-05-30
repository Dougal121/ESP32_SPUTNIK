void SerialOutParams() {
  String message ;

  message = "Web Request URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  Serial.println(message);
  message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  Serial.println(message);
}

void SendHTTPHeader() {
  String message ;
  String strOnOff ;
  
  server.sendHeader(F("Server"), F("ESP32-tinfoil-hats"), false);
  server.sendHeader(F("X-Powered-by"), F("Dougal-filament-7"), false);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(F(Styles));
  server.sendContent(F("<!DOCTYPE HTML>"));
  server.sendContent("<head><title>LighNow" + String(Toleo) + "</title>");
/*
  if ( bDoSendSerialCommand ) {
    server.sendContent(F("<meta http-equiv='Refresh' content='1'>"));
  }
*/  
  server.sendContent(F("<meta name=viewport content='width=150, auto inital-scale=1'>"));
//  server.sendContent(F("<link rel='icon' type='image/png' href='download?file=/favorite.ico'>"));
  server.sendContent(F("</head><body><html lang='en'><center><h3>"));
  server.sendContent("<a title='click for home / refresh' href='/'>" + String(ghks.NodeName) + "</a></h3>");
}


void SendHTTPPageFooter() {
  server.sendContent(F("<br><a href='/?command=1'>Load Parameters from EEPROM</a><br><br><a href='/?command=667'>Reset Memory to Factory Default</a><br><a href='/?command=665'>Sync UTP Time</a><br><a href='/stime'>Manual Time Set</a><br><a href='/scan'>I2C Scan</a><br>")) ;
  server.sendContent("<a href='/?reboot=" + String(lRebootCode) + "'>Reboot</a><br>");
  server.sendContent(F("<a href='/eeprom'>EEPROM Memory Contents</a><br>"));
  server.sendContent(F("<a href='/setup'>Node Setup</a><br>"));
  server.sendContent(F("<a href='/info'>Node Infomation</a><br>"));  
  server.sendContent(F("<a href='/vsss'>view volatile memory structures</a><br>"));
  if (!WiFi.isConnected()) {
    snprintf(buff, BUFF_MAX, "%u.%u.%u.%u", MyIPC[0], MyIPC[1], MyIPC[2], MyIPC[3]);
  } else {
    snprintf(buff, BUFF_MAX, "%u.%u.%u.%u", MyIP[0], MyIP[1], MyIP[2], MyIP[3]);
  }
  server.sendContent("<a href='http://" + String(buff) + "/update'>OTA Firmware Update</a><br>");
  server.sendContent("<a href='https://github.com/Dougal121/ESP32_SPUTNIK'>Source at GitHub</a><br>");
  server.sendContent("<a href='http://" + String(buff) + "/backup'>Backup / Restore Settings</a><br>");
  if (hasSD){
    server.sendContent("<a href='http://" + String(buff) + "/list'>List log files on SD Card</a><br>");    
  }
  server.sendContent(F("</body></html>\r\n"));
}


void handleNotFound() {
  String message = F("Seriously - No way DUDE !!!\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, F("text/plain"), message);
  //  Serial.print(message);
}


void handleRoot() {
  boolean currentLineIsBlank = true;
  tmElements_t tm;
  long  i = 0 ;
  int ii  ;
  int iProgNum = 0;
  int j ;
  int k , kk , iTmp ;
  boolean bExtraValve = false ;
  uint8_t iPage = 0 ;
  int iAType = 0 ;
  boolean bDefault = true ;
  //  int td[6];
  long lTmp ;
  String MyCheck , MyColor , MyNum , pinname ;
  byte mac[6];
  String message ;

//  SerialOutParams();

  for (uint8_t j = 0; j < server.args(); j++) {
    //    bSaveReq = 1 ;
    i = String(server.argName(j)).indexOf("command");
    if (i != -1) { //
      switch (String(server.arg(j)).toInt()) {
        case 1:  // load values
          LoadParamsFromEEPROM(true);
          //          Serial.println("Load from EEPROM");
          break;
        case 2: // Save values
          LoadParamsFromEEPROM(false);
          //          Serial.println("Save to EEPROM");
          break;
        case 3:
        break;
        case 4:
        break;
        case 5:
        break;
        case 6:
        break;
        case 8: //  Cold Reboot
          ESP.restart() ;  // ESP.reset();
          break;
        case 9: //  Warm Reboot
          ESP.restart() ;
        break;
        case 42:
        break;
        case 667: // wipe the memory to factory default
          BackInTheBoxMemory();
          break;
        case 665:
          bManSet = true  ;
          bDoTimeUpdate = false ;          
          sendNTPpacket(ghks.timeServer); // send an NTP packet to a time server  once and hour
          break;
      }
    }
    i = String(server.argName(j)).indexOf("reboot");
    if (i != -1) { //
      if (( lRebootCode == String(server.arg(j)).toInt() ) && (lRebootCode > 0 )) { // stop the phone browser being a dick and retry resetting !!!!
        ESP.restart() ;
      }
    }

    i = String(server.argName(j)).indexOf("atoff");
    if (i != -1){  // have a request to request a time update
      tm.Year = (String(server.arg(j)).substring(0,4).toInt()-1970) ;
      tm.Month =(String(server.arg(j)).substring(5,7).toInt()) ;
      tm.Day = (String(server.arg(j)).substring(8,10).toInt()) ;
      tm.Hour =(String(server.arg(j)).substring(11,13).toInt()) ;
      tm.Minute = (String(server.arg(j)).substring(14,16).toInt()) ;
      tm.Second = 0 ;
      ghks.AutoOff_t = makeTime(tm);
    }  

/*
    i = String(server.argName(j)).indexOf("nurl");
    if (i != -1){                                   
     String(server.arg(j)).toCharArray( SensorApp.nURL , sizeof(SensorApp.nURL)) ;     // URL
    }
*/

    i = String(server.argName(j)).indexOf("stime");
    if (i != -1) { //
      tm.Year = (String(server.arg(j)).substring(0, 4).toInt() - 1970) ;
      tm.Month = (String(server.arg(j)).substring(5, 7).toInt()) ;
      tm.Day = (String(server.arg(j)).substring(8, 10).toInt()) ;
      tm.Hour = (String(server.arg(j)).substring(11, 13).toInt()) ;
      tm.Minute = (String(server.arg(j)).substring(14, 16).toInt()) ;
      tm.Second = 0 ;
      setTime(makeTime(tm));
      if ( hasRTC ) {
        tc.sec = second();
        tc.min = minute();
        tc.hour = hour();
        tc.wday = dayOfWeek(makeTime(tm));
        tc.mday = day();
        tc.mon = month();
        tc.year = year();
        DS3231_set(tc);                       // set the RTC as well
        rtc_status = DS3231_get_sreg();       // get the status
        DS3231_set_sreg(rtc_status & 0x7f ) ; // clear the clock fail bit when you set the time
      }
    }

    i = String(server.argName(j)).indexOf("ndadd");
    if (i != -1) { //
      ghks.lNodeAddress = String(server.arg(j)).toInt() ;
      ghks.lNodeAddress = constrain(ghks.lNodeAddress, 0, 32768);
    }
    i = String(server.argName(j)).indexOf("tzone");
    if (i != -1) { //
      ghks.fTimeZone = String(server.arg(j)).toFloat() ;
      ghks.fTimeZone = constrain(ghks.fTimeZone, -12, 12);
      bDoTimeUpdate = true ; // trigger and update to fix the time
    }
    
    

    i = String(server.argName(j)).indexOf("netop");
    if (i != -1) { //
      ghks.lNetworkOptions = String(server.arg(j)).toInt() ;
      ghks.lNetworkOptions = constrain(ghks.lNetworkOptions, 0, 255);
    }


    i = String(server.argName(j)).indexOf("lpntp");
    if (i != -1) { //
      ghks.localPort = String(server.arg(j)).toInt() ;
      ghks.localPort = constrain(ghks.localPort, 1, 65535);
    }
    i = String(server.argName(j)).indexOf("lpctr");
    if (i != -1) { //
      ghks.localPortCtrl = String(server.arg(j)).toInt() ;
      ghks.localPortCtrl = constrain(ghks.localPortCtrl, 1, 65535);
    }
    i = String(server.argName(j)).indexOf("rpctr");
    if (i != -1) { //
      ghks.RemotePortCtrl = String(server.arg(j)).toInt() ;
      ghks.RemotePortCtrl = constrain(ghks.RemotePortCtrl, 1, 65535);
    }
    i = String(server.argName(j)).indexOf("dontp");
    if (i != -1) { // have a request to request a time update
      bDoTimeUpdate = true ;
    }
    i = String(server.argName(j)).indexOf("cname");
    if (i != -1) { // have a request to request a time update
      String(server.arg(j)).toCharArray( ghks.NodeName , sizeof(ghks.NodeName)) ;
    }
    i = String(server.argName(j)).indexOf("rpcip");
    if (i != -1) { // have a request to request an IP address
      ghks.RCIP[0] = String(server.arg(j)).substring(0, 3).toInt() ;
      ghks.RCIP[1] = String(server.arg(j)).substring(4, 7).toInt() ;
      ghks.RCIP[2] = String(server.arg(j)).substring(8, 11).toInt() ;
      ghks.RCIP[3] = String(server.arg(j)).substring(12, 15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("staip");
    if (i != -1) { // have a request to request an IP address
      ghks.IPStatic[0] = String(server.arg(j)).substring(0, 3).toInt() ;
      ghks.IPStatic[1] = String(server.arg(j)).substring(4, 7).toInt() ;
      ghks.IPStatic[2] = String(server.arg(j)).substring(8, 11).toInt() ;
      ghks.IPStatic[3] = String(server.arg(j)).substring(12, 15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("gatip");
    if (i != -1) { // have a request to request an IP address
      ghks.IPGateway[0] = String(server.arg(j)).substring(0, 3).toInt() ;
      ghks.IPGateway[1] = String(server.arg(j)).substring(4, 7).toInt() ;
      ghks.IPGateway[2] = String(server.arg(j)).substring(8, 11).toInt() ;
      ghks.IPGateway[3] = String(server.arg(j)).substring(12, 15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("mskip");
    if (i != -1) { // have a request to request an IP address
      ghks.IPMask[0] = String(server.arg(j)).substring(0, 3).toInt() ;
      ghks.IPMask[1] = String(server.arg(j)).substring(4, 7).toInt() ;
      ghks.IPMask[2] = String(server.arg(j)).substring(8, 11).toInt() ;
      ghks.IPMask[3] = String(server.arg(j)).substring(12, 15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("dnsip");
    if (i != -1) { // have a request to request an IP address
      ghks.IPDNS[0] = String(server.arg(j)).substring(0, 3).toInt() ;
      ghks.IPDNS[1] = String(server.arg(j)).substring(4, 7).toInt() ;
      ghks.IPDNS[2] = String(server.arg(j)).substring(8, 11).toInt() ;
      ghks.IPDNS[3] = String(server.arg(j)).substring(12, 15).toInt() ;
    }

    i = String(server.argName(j)).indexOf("atoff");
    if (i != -1) { // have a request to request a time update
      tm.Year = (String(server.arg(j)).substring(0, 4).toInt() - 1970) ;
      tm.Month = (String(server.arg(j)).substring(5, 7).toInt()) ;
      tm.Day = (String(server.arg(j)).substring(8, 10).toInt()) ;
      tm.Hour = (String(server.arg(j)).substring(11, 13).toInt()) ;
      tm.Minute = (String(server.arg(j)).substring(14, 16).toInt()) ;
      tm.Second = 0 ;
      ghks.AutoOff_t = makeTime(tm);
    }
    i = String(server.argName(j)).indexOf("nssid");
    if (i != -1) {                                   // SSID
      //    Serial.println("SookyLala 1 ") ;
      String(server.arg(j)).toCharArray( ghks.nssid , sizeof(ghks.nssid)) ;
    }

    i = String(server.argName(j)).indexOf("npass");
    if (i != -1) {                                   // Password
      String(server.arg(j)).toCharArray( ghks.npassword , sizeof(ghks.npassword)) ;
    }

    i = String(server.argName(j)).indexOf("cpass");
    if (i != -1) {                                   // Password
      String(server.arg(j)).toCharArray( ghks.cpassword , sizeof(ghks.cpassword)) ;
    }

    i = String(server.argName(j)).indexOf("timsv");
    if (i != -1) {                                   // timesvr
      String(server.arg(j)).toCharArray( ghks.timeServer , sizeof(ghks.timeServer)) ;
    }
  }

  SendHTTPHeader();   //  ################### START OF THE RESPONSE  ######

  if ( bSaveReq != 0 ) {
    server.sendContent(F("<blink>"));
  }
  server.sendContent(F("<a href='/?command=2'>Save Parameters to EEPROM</a><br>")) ;
  if ( bSaveReq != 0 ) {
    server.sendContent(F("</blink><font color='red'><b>Changes Have been made to settings.<br>Make sure you save if you want to keep them</b><br></font><br>")) ;
  }


  snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());
  if (ghks.fTimeZone > 0 ) {
    server.sendContent("<b>" + String(buff) + " UTC +" + String(ghks.fTimeZone, 1) ) ;
  } else {
    server.sendContent("<b>" + String(buff) + " UTC " + String(ghks.fTimeZone, 1) ) ;
  }
  if ( year() < 2020 ) {
    server.sendContent(F("<font color=red><b>  --- CLOCK NOT SET ---</b></font>")) ;
  }
  server.sendContent(F("</b><br>")) ;
  if ( ghks.AutoOff_t > now() )  {
    snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(ghks.AutoOff_t), month(ghks.AutoOff_t), day(ghks.AutoOff_t) , hour(ghks.AutoOff_t), minute(ghks.AutoOff_t), second(ghks.AutoOff_t));
    server.sendContent(F("<b><font color=red>Automation OFFLINE Untill ")) ;
    server.sendContent(String(buff)) ;
    server.sendContent(F("</font></b><br>")) ;
  } else {
    if (( now() > ghks.AutoOff_t) &&(  year() > 2019 )) {
      server.sendContent(F("<b><font color=green>Automation ONLINE</font></b><br>")) ;
    } else {
      server.sendContent(F("<b><font color=red>Automation OFFLINE Invalid time</font></b><br>")) ;
    }
  }

  if (String(server.uri()).indexOf("stime") > 0) { // ################   SETUP TIME    #######################################
    bDefault = false ;
    snprintf(buff, BUFF_MAX, "%04d/%02d/%02d %02d:%02d", year(), month(), day() , hour(), minute());
    server.sendContent("<br><br><form method=post action=" + server.uri() + "><br>Set Current Time: <input type='text' name='stime' value='" + String(buff) + "' size=12>");
    server.sendContent(F("<input type='submit' value='SET'><br><br></form>"));
  }


  if (String(server.uri()).indexOf("setup") > 0) { // ################  SETUP OF THE NODE #####################################
    bDefault = false ;
    server.sendContent("<form method=post action=" + server.uri() + "><table border=1 title='Node Settings'>");
    server.sendContent(F("<tr><th>Parameter</th><th>Value</th><th><input type='submit' value='SET'></th></tr>"));

    server.sendContent(F("<tr><td>Controler Name</td><td align=center>")) ;
    server.sendContent("input type='text' name='cname' value='" + String(ghks.NodeName) + "' maxlength=15 size=12></td><td></td></tr>");

    snprintf(buff, BUFF_MAX, "%04d/%02d/%02d %02d:%02d", year(ghks.AutoOff_t), month(ghks.AutoOff_t), day(ghks.AutoOff_t) , hour(ghks.AutoOff_t), minute(ghks.AutoOff_t));
    if (ghks.AutoOff_t > now()) {
      MyColor =  F("bgcolor=red") ;
    } else {
      MyColor =  "" ;
    }
    server.sendContent("<tr><td " + String(MyColor) + ">Auto Off Until</td><td align=center>") ;
    server.sendContent("<input type='text' name='atoff' value='" + String(buff) + "' size=12></td><td>(yyyy/mm/dd)</td></tr>");

    server.sendContent(F("<tr><td>Node Address</td><td align=center>")) ;
    server.sendContent("<input type='text' name='ndadd' value='" + String(ghks.lNodeAddress) + "' size=12></td><td>" + String(ghks.lNodeAddress & 0xff) + "</td></tr>");

    server.sendContent(F("<tr><td>Time Zone</td><td align=center>")) ;
    server.sendContent("<input type='text' name='tzone' value='" + String(ghks.fTimeZone, 1) + "' size=12></td><td>(Hours)</td></tr>");


    server.sendContent("<form method=post action=" + server.uri() + "><tr><td></td><td></td><td></td></tr>") ;

    server.sendContent(F("<tr><td>Local UDP Port NTP</td><td align=center>")) ;
    server.sendContent("<input type='text' name='lpntp' value='" + String(ghks.localPort) + "' size=12></td><td><input type='submit' value='SET'></td></tr>");

    server.sendContent(F("<tr><td>Local UDP Port Control</td><td align=center>")) ;
    server.sendContent("<input type='text' name='lpctr' value='" + String(ghks.localPortCtrl) + "' size=12></td><td></td></tr>");

    server.sendContent(F("<tr><td>Remote UDP Port Control</td><td align=center>")) ;
    server.sendContent("<input type='text' name='rpctr' value='" + String(ghks.RemotePortCtrl) + "' size=12></td><td></td></tr>");

    server.sendContent(F("<tr><td>Network SSID</td><td align=center>")) ;
    server.sendContent("<input type='text' name='nssid' value='" + String(ghks.nssid) + "' maxlength=15 size=12></td><td></td></tr>");

    server.sendContent(F("<tr><td>Network Password</td><td align=center>")) ;
    server.sendContent("<input type='text' name='npass' value='" + String(ghks.npassword) + "' maxlength=15 size=12></td><td></td></tr>");

    server.sendContent(F("<tr><td>Configure Password</td><td align=center>")) ;
    server.sendContent("<input type='text' name='cpass' value='" + String(ghks.cpassword) + "' maxlength=15 size=12></td><td></td></tr>");

    server.sendContent(F("<tr><td>Time Server</td><td align=center>")) ;
    server.sendContent("<input type='text' name='timsv' value='" + String(ghks.timeServer) + "' maxlength=23 size=12></td><td></td></tr>");

    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.RCIP[0], ghks.RCIP[1], ghks.RCIP[2], ghks.RCIP[3]);
    server.sendContent(F("<tr><td>Remote IP Address Control</td><td align=center>")) ;
    server.sendContent("<input type='text' name='rpcip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr></form>");

    server.sendContent("<form method=post action=" + server.uri() + "><tr><td></td><td></td><td></td></tr>") ;

    server.sendContent(F("<tr><td>Network Options</td><td align=center>")) ;
    server.sendContent(F("<select name='netop'>")) ;
    if (ghks.lNetworkOptions == 0 ) {
      server.sendContent(F("<option value='0' SELECTED>0 - DHCP"));
      server.sendContent(F("<option value='1'>1 - Static"));
    } else {
      server.sendContent(F("<option value='0'>0 - DHCP"));
      server.sendContent(F("<option value='1' SELECTED>1 - Static IP"));
    }
    server.sendContent(F("</select></td><td><input type='submit' value='SET'></td></tr>"));
    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPStatic[0], ghks.IPStatic[1], ghks.IPStatic[2], ghks.IPStatic[3]);
    server.sendContent(F("<tr><td>Static IP Address</td><td align=center>")) ;
    server.sendContent("<input type='text' name='staip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPGateway[0], ghks.IPGateway[1], ghks.IPGateway[2], ghks.IPGateway[3]);
    server.sendContent(F("<tr><td>Gateway IP Address</td><td align=center>")) ;
    server.sendContent("<input type='text' name='gatip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPMask[0], ghks.IPMask[1], ghks.IPMask[2], ghks.IPMask[3]);
    server.sendContent(F("<tr><td>IP Mask</td><td align=center>")) ;
    server.sendContent("<input type='text' name='mskip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPDNS[0], ghks.IPDNS[1], ghks.IPDNS[2], ghks.IPDNS[3]);
    server.sendContent(F("<tr><td>DNS IP Address</td><td align=center>")) ;
    server.sendContent("<input type='text' name='dnsip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

    server.sendContent("<tr><td>Last Scan Speed</td><td align=center>" + String(lScanLast) + "</td><td>(per second)</td></tr>" ) ;
    if ( hasRTC ) {
      rtc_status = DS3231_get_sreg();
      if (( rtc_status & 0x80 ) != 0 ) {
        server.sendContent(F("<tr><td>RTC Battery</td><td align=center bgcolor='red'>DEPLETED</td><td></td></tr>")) ;
      } else {
        server.sendContent(F("<tr><td>RTC Battery</td><td align=center bgcolor='green'>-- OK --</td><td></td></tr>")) ;
      }
      server.sendContent("<tr><td>RTC Temperature</td><td align=center>" + String(rtc_temp, 1) + "</td><td>(C)</td></tr>") ;
    }
    server.sendContent(F("</form></table>"));
  }


  if (String(server.uri()).indexOf("vsss") > 0) { // ############################  volitile status - all structures status  ###############################
    bDefault = false ;
    server.sendContent(F("<br><b>Internal Variables</b><br>"));
    server.sendContent(F("<table border=1 title='Internal Memory Values'>"));
    server.sendContent(F("<tr><th><b>Parameter</td><td align=center><b>Value</b></td></tr>")) ;
//    server.sendContent("<tr><td>Off Time</td><td align=center>" + String(esui.iOffTime) + "</td></tr>" ) ;
    server.sendContent(F("</table>"));
  }

  if (bDefault) {     // #####################################   default   ##############################################

  }

  SendHTTPPageFooter();
}



