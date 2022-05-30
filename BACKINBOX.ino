void BackInTheBoxMemory(){
  uint8_t i , j ;

  sprintf(ghks.nssid,"************\0");  // put your default credentials in here if you wish
  sprintf(ghks.npassword,"********\0");  // put your default credentials in here if you wish
  sprintf(ghks.NodeName,"LightNow\0") ;



  sprintf(ghks.cpassword,"\0");
  
  ghks.fTimeZone = 10.0 ;
  ghks.lNodeAddress = (long)chipid & 0xff ;
  sprintf(ghks.timeServer ,"au.pool.ntp.org\0"); 
  ghks.AutoOff_t = 0 ;
  ghks.localPortCtrl = 8088 ;
  ghks.RemotePortCtrl= 8088 ;
  ghks.lVersion = MYVER ;
  
/*  ghks.RCIP[0] = 192 ;
  ghks.RCIP[1] = 168 ; 
  ghks.RCIP[2] = 2 ;
  ghks.RCIP[3] = 255 ;*/
  sprintf(ghks.RCIP ,"192.168.2.255\0"); 
  
  ghks.lNetworkOptions = 0 ;     // DHCP 
  ghks.IPStatic[0] = 192 ;
  ghks.IPStatic[1] = 168 ;
  ghks.IPStatic[2] = 0 ;
  ghks.IPStatic[3] = 123 ;

  ghks.IPGateway[0] = 192 ;
  ghks.IPGateway[1] = 168 ;
  ghks.IPGateway[2] = 0 ;
  ghks.IPGateway[3] = 1 ;

  ghks.IPDNS = ghks.IPGateway ;

  ghks.IPMask[0] = 255 ;
  ghks.IPMask[1] = 255 ;
  ghks.IPMask[2] = 255 ;
  ghks.IPMask[3] = 0 ;

  ghks.IPPing[0] = 192 ;
  ghks.IPPing[1] = 168 ;
  ghks.IPPing[2] = 1 ;
  ghks.IPPing[3] = 1 ;

  ghks.PingMax = 1500 ;
  ghks.PingFreq = 10 ; 
  ghks.SelfReBoot = 0 ; 


  ghks.latitude = -34.051219 ;
  ghks.longitude = 142.013618 ;

  for (i = 0 ; i < MAX_SEGSETTINGS ; i++){
    for (j = 0 ; j < MAX_SEGMENTS ; j++){
      lnas.SegMem[i].seg[j].iBright = DEFBRIGHTNESS;
      lnas.SegMem[i].seg[j].iRed = 0 ;
      lnas.SegMem[i].seg[j].iGreen = 0 ;
      lnas.SegMem[i].seg[j].iBlue = 0 ;
      lnas.SegMem[i].seg[j].iWhite = 255 ;                    // default WRGB on turn on 
      lnas.SegMem[i].seg[j].iStart = 0 ;                      // position inside
      lnas.SegMem[i].seg[j].iStop = NUM_PIXELS[i]-1 ;         // position outside
      lnas.SegMem[i].seg[j].iGrad = 0 ;                       // gradient  
    }
  }  
  


}


void LoadParamsFromEEPROM(bool bLoad){
long lTmp ;  
int i ;
int j ;
int bofs ,ofs ;
int eeAddress ;

  if ( bLoad ) {
    EEPROM.get(0,ghks);
    eeAddress = sizeof(ghks) ;
    Serial.println("read - ghks structure size " +String(eeAddress));   

    ghks.lNodeAddress = constrain(ghks.lNodeAddress,0,32768);
    ghks.fTimeZone = constrain(ghks.fTimeZone,-12,12);
    ghks.localPort = constrain(ghks.localPort,1,65535);
    ghks.localPortCtrl = constrain(ghks.localPortCtrl,1,65535);
    ghks.RemotePortCtrl = constrain(ghks.RemotePortCtrl,1,65535);
    if ( year(ghks.AutoOff_t) < 2000 ){
       ghks.AutoOff_t = now();
    }
    ghks.lDisplayOptions = constrain(ghks.lDisplayOptions,0,1);

    eeAddress = PROG_BASE ;  

    EEPROM.get(eeAddress,lnas);
    eeAddress += sizeof(lnas) ;
/*
    EEPROM.get(eeAddress,SMTP);
    eeAddress += sizeof(SMTP) ;
*/
    
    Serial.println("Final VPFF EEPROM adress " +String(eeAddress));   
    
  }else{
    ghks.lVersion  = MYVER ;
    EEPROM.put(0,ghks);
    eeAddress = sizeof(ghks) ;
    Serial.println("write - ghks structure size " +String(eeAddress));   

    eeAddress = PROG_BASE ;
    
    EEPROM.put(eeAddress,lnas);
    eeAddress += sizeof(lnas) ;
/*    
    EEPROM.put(eeAddress,SMTP);
    eeAddress += sizeof(SMTP) ;
*/

    Serial.println("Final EEPROM Save adress " +String(eeAddress));   

    EEPROM.commit();                                                       // save changes in one go ???
    bSaveReq = 0 ;
  }
}


