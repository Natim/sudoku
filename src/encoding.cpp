#include "encoding.h"

std::string utf8ToLatin1(const std::string & utf8){
  std::string out;
  out.reserve(utf8.size());

  for(size_t i = 0; i < utf8.size(); ){
    unsigned char c = utf8[i];
    if(c < 0x80){
      out += (char) c;
      i++;
    } else if((c & 0xE0) == 0xC0 && i + 1 < utf8.size()){
      unsigned code = ((c & 0x1F) << 6) | (utf8[i + 1] & 0x3F);
      out += (code <= 0xFF) ? (char) code : '?';
      i += 2;
    } else {
      out += '?';
      i += (c & 0xF0) == 0xE0 ? 3 : (c & 0xE0) == 0xC0 ? 2 : 1;
    }
  }

  return out;
}
