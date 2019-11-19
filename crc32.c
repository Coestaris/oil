//
// Created by maxim on 3/10/19.
//

#include "crc32.h"

uint32_t CRC32(uint8_t* buff, size_t buffLen, uint32_t init)
{
   uint32_t crc = init;
   for (size_t i = 0; i < buffLen; i++)
   {
      crc = updateCRC32(*buff++, crc);
   }

   return crc;
}

uint32_t updateCRC32(uint8_t ch, uint32_t crc)
{
   uint32_t idx = ((crc) ^ (ch)) & 0xff;
   uint32_t tab_value = crc_32_tab[idx];
   return tab_value ^ ((crc) >> 8);
}