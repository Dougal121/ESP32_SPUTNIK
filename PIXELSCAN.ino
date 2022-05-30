void pixelscope() {
  uint8_t i, r, g , b;
  int address ;
  char buff[10];
  uint32_t pcol ;
  uint32_t xcol ;
  uint8_t j ;
  uint8_t s = 0 ;
  SendHTTPHeader();

  server.sendContent(F("<center><b>Pixel Scope</b><br><table border=1 title='Pixel Scope'><tr><th>.</th>"));
 // table header
  switch(strip[s].numPixels()){
    case 64:
      j = 8 ;
    break;
    case 600:
      j = 20 ;
    break;
    default:
      j = 16 ;
    break;
  }
  for (i = 0; i < j; i++) {
    if (strip[s].numPixels() == 600 ) {      
      server.sendContent("<th>"+String(i)+"</th>");    
    }else{
      server.sendContent("<th>"+String(i,HEX)+"</th>");    
    }
  }
  server.sendContent(F("</tr>"));
  
  for (address = 0; address < strip[s].numPixels(); address++) {
    if (address % j == 0) {
      server.sendContent(F("<tr>"));
      if (strip[s].numPixels() == 600 ) {            
        server.sendContent("<td align=center><b>"+String((address ))+"</b></td>");
      }else{
        server.sendContent("<td align=center><b>"+String((address ),HEX)+"</b></td>");
      }
    }
    pcol = strip[s].getPixelColor(address) ;    //strip[0].getPixelColor(address)
    xcol = pcol ^ 0xffffff ;
    r = ( pcol >> 16 ) & 0xff ;
    g = ( pcol >> 8 ) & 0xff ;
    b = ( pcol  ) & 0xff ;
    snprintf(buff, BUFF_MAX, "%02X%02X%02X", r, g ,b );
    
    server.sendContent("<td bgcolor='"+String(buff)+"'><font color='"+String(xcol,HEX)+"'>"+String(buff)+"</font></td>");
    if (address % j == (j-1)) {
      server.sendContent(F("</tr>"));
    }
  }
  server.sendContent(F("</tr></table></body></html>\r\n"));
}


