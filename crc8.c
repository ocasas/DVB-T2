#define GP  0x1D5     /* x^8 + x^7 + x^6 + x^4 + x^2 + 1 */
#define DI  0xAB      //primer dato


static unsigned char crc8_table[256];   // tabla de 8-bit
static int made_table=0;                // Flag de realizacion de tabla


//------------------------------------------------------------
//Funcion de inicializacion de tabla de 8-bit
static void init_crc8()
{
  int i,j;
  unsigned char crc;

  if (!made_table) {
    for (i=0; i<256; i++) {
      crc = i;
      for (j=0; j<8; j++)
        crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
      crc8_table[i] = crc & 0xFF;
      //printf("table[%d] = \t%d \t(0x%X)\n", i, crc, crc);
    }
    made_table=1;
  }
}


//------------------------------------------------------------
//Calculo de CRC8 a m. Se añade en *crc
void crc8(unsigned char *crc, unsigned char m)
{
  if (!made_table)
    init_crc8();

  *crc = crc8_table[(*crc) ^ m];
  *crc &= 0xFF;
}
