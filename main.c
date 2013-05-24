#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc8.c"

#define MAX 1 
#define MAXNUM 400 


//------------------------------------------------------------
//Estructura/union para TS_packet_field con cada item separado
typedef struct 
{
    unsigned char   sync_byte;
    unsigned char   transport_error_indicator:      1;
    unsigned char   payload_unit_start_indicator:   1;
    unsigned char   transport_priority:             1;
    unsigned short  PID:                            13;
    
    unsigned char   transport_scrambling_control:   2;
    unsigned char   adaption_field_control:         2;
    unsigned char   continuity_counter:             4;
    
    unsigned char   data_byte[184];
}TS_packet_field_indep;


//------------------------------------------------------------
//Estructura TS_packet_field
typedef struct 
{
    unsigned char   sync_byte;
    unsigned char   PID[2];
    unsigned char   counter;    
    unsigned char   data_byte[184];
}TS_packet_field;


//------------------------------------------------------------
//Estructura BB_header
typedef struct 
{
    unsigned short  matype;
    unsigned short  upl;
    unsigned short  dfl;
    unsigned char   sync;
    unsigned short  syncd;
    unsigned char   crc8;
    
}BB_header;


//------------------------------------------------------------
//Función para imprimier un TS_packet_field
void imprimir_TS_packet_field(TS_packet_field x)
{
    int i;
       
       printf("sync_byte: \t\t\t\t%x\n", x.sync_byte);
       printf("transport_error_indicator: \t\t%x\n", (x.PID[0]&0x80)>>7);
       printf("payload_unit_start_indicator: \t\t%x\n", (x.PID[0]&0x40)>>6);
       printf("transport_priority: \t\t\t%x\n", (x.PID[0]&0x20)>>5);       
       printf("PID: \t\t\t\t\t%x\n", ((unsigned short)(x.PID[0]&0x1F)<<8 )+ x.PID[1]);
       printf("transport_scrambling_control: \t\t%x\t\n", (x.counter&0xC0)>>6);
       printf("adaption_field_control: \t\t%x\t\n", (x.counter&0x30)>>4);
       printf("continuity_counter: \t\t\t%x\t\n", (x.counter&0x0F));
       printf("data_byte:\n");
       for(i=0;i<184;i++)
            printf("%x", x.data_byte[i]);
        
        printf("\n\n");
    
    }


//------------------------------------------------------------
//Convierte un char a hexadecimal
unsigned char convHex(char a)
{
    char r=0;
    switch(a)
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':   r = a-'0';
                    break;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':   r = a-'A'+10;
                    break;
    }
    
    return r;
}


////////////////////////////////////////////////////////////////////////////////////////////
unsigned char convHex2(char a, char b)
{
    char r=0;
    
    r = (convHex(a)<<4) | convHex(b);

    return r;
}


////////////////////////////////////////////////////////////////////////////////////////////
int main ()
{
  FILE * pFile;
  unsigned char crc_8=0, *crc;
  //char x[MAX][MAXNUM];
  TS_packet_field x;
  char y[188*2];
  unsigned char temp[100],a ,b;
  short c;
  
  int i,j;
  
  crc=&crc_8;
  
  ///////////////////////////////////////////////////////////////////////////////////////////
  //Lectura de datos de archivo
  
  pFile = fopen ("test_vector.txt","r");
  
  if (pFile==NULL) perror ("Error opening file");   
  else
  {
        
        //leer caracter, si es #, botar resto
        
       for(j=0;j<MAX;j++)     
       {
            
            //eliminar comentarios que inician con #
            do{
                temp[0] = fgetc (pFile);
            }while(temp[0]!='#');
            fgets (temp+1 , 100 , pFile);
    
            //leer 188 bytes
            for(i=0;i<188*2;i+=2)
            {
                do{  y[i] = fgetc (pFile);} while(y[i]=='\n');
                do{  y[i+1] = fgetc (pFile);} while(y[i+1]=='\n');                    
            }
                
                
            //'mapear' los 188 bytes en la estructura TS_packet_field
            x.sync_byte = convHex2(y[0], y[1]);
            x.PID[0]    = convHex2(y[2], y[3]);
            x.PID[1]    = convHex2(y[4], y[5]);
            x.counter   = convHex2(y[6], y[7]);                
            for(i=0, j=8;i<184;i++, j+=2)            
                x.data_byte[i] = convHex2(y[j], y[j+1]);
                
        }  
             
       printf("\n\n");
       puts(temp);
       imprimir_TS_packet_field(x);


      //Calculo de CRC-8
      init_crc8();
      *crc=0;
      crc8(crc, x.sync_byte);
      crc8(crc, x.PID[0]);
      crc8(crc, x.PID[1]);
      crc8(crc, x.counter);
      for(i=0;i<184;i++)
          crc8(crc, x.data_byte[i]);
      printf("\nCRC8 : %x\n\n", crc_8);



      fclose (pFile);
  }


  
  system("PAUSE");
  return 0;
}
