#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc8.c"

#define MAX 1           //Numero de Paquetes TS a procesar
//#define MAXNUM 400


#define TS      0   	//TRANSPORT STREAM
#define GSE     1   	//GENERIC ENCAPSULATED STREAM
#define GCS     2   	//GENERIC CONTINUOUS STREAM
#define GFPS    3   	//GENERIC FIXED-LENGTH PACKETIZED STREAM

#define ON      1
#define OFF     0

#define KBCH    200 	//Numero de bits protegidos por los codigos BHC y LDPC
#define FRAG    ON  	//Fragmentacion
#define IBS     OFF 	//In-band Signalling
#define NPD     ON  	//Null packet deletion

#define FEF                     OFF     //Future Extension Frame
#define NUM_PLP                 1       //Numero de PLPs a la entrada - Physical Layer Pipes
#define DFL                     KBCH-80 //Data Field Length
#define PLP_NUM_BLOCKS          0       //
#define PLP_NUM_BLOCKS_MAX      0       //

char dnp_counter=0;     //contador de paquetes nulos





typedef struct
//------------------------------------------------------------
//Estructura/union para TS_packet_field con cada item separado
{
    unsigned char   sync_byte;
    unsigned char   transport_error_indicator:      1;
    unsigned char   payload_unit_start_indicator:   1;
    unsigned char   transport_priority:             1;
    unsigned short  pid:                            13;

    unsigned char   transport_scrambling_control:   2;
    unsigned char   adaption_field_control:         2;
    unsigned char   continuity_counter:             4;

    unsigned char   data_byte[184];
}TS_packet_field_indep;


typedef struct
//------------------------------------------------------------
//Estructura TS_packet_field
{
    unsigned char   sync_byte;
    unsigned char   pid[2];
    unsigned char   counter;
    unsigned char   data_byte[184];
}TS_packet_field;


//Estructura UP
typedef struct
//------------------------------------------------------------
//Estructura TS_packet_field
{
	TS_packet_field	data;
    unsigned char   issy;
    unsigned char   dnp;
    unsigned char   crc;
}UP;



typedef struct
//------------------------------------------------------------
//Estructura BB_header
{
    unsigned short  matype;
    unsigned short  upl;
    unsigned short  dfl;
    unsigned char   sync;
    unsigned short  syncd;
    unsigned char   crc8;

}BB_header;



typedef struct
//------------------------------------------------------------
//Estructura BB_frame
{
    BB_header       header;
    unsigned char   dfl[KBCH-80];

}BB_frame;



void imprimir_TS_packet_field(TS_packet_field x)
//------------------------------------------------------------
//Función para imprimier un TS_packet_field
{
    int i;

       printf("sync_byte: \t\t\t\t%x\n", x.sync_byte);
       printf("transport_error_indicator: \t\t%x\n", (x.pid[0]&0x80)>>7);
       printf("payload_unit_start_indicator: \t\t%x\n", (x.pid[0]&0x40)>>6);
       printf("transport_priority: \t\t\t%x\n", (x.pid[0]&0x20)>>5);
       printf("PID: \t\t\t\t\t%x\n", ((unsigned short)(x.pid[0]&0x1F)<<8 )+ x.pid[1]);
       printf("transport_scrambling_control: \t\t%x\t\n", (x.counter&0xC0)>>6);
       printf("adaption_field_control: \t\t%x\t\n", (x.counter&0x30)>>4);
       printf("continuity_counter: \t\t\t%x\t\n", (x.counter&0x0F));
       printf("data_byte:\n");
       for(i=0;i<184;i++)
            printf("%x", x.data_byte[i]);

        printf("\n\n");

}



int null_packet_deletion(TS_packet_field * packet)
//------------------------------------------------------------
//Funcion para eliminar paquetes nulos
//---------------si el paquete es nulo, aumenta dnp_counter y retorna 0
//---------------si el paquete es util, pone en 0 dnp_counter y retorna 1
{
    if( (packet->pid[0] == 0x1f) && (packet->pid[1] == 0xff))
    {
        dnp_counter++;
        return 0;
    }

    dnp_counter=0;
    return 1;
}




int get_useful_packet(TS_packet_field packetin, int dnp_counter, UP *packetout)
//------------------------------------------------------------
//Funcion para eliminar paquetes nulos
//---------------si el paquete es nulo, aumenta dnp_counter y retorna 0
//---------------si el paquete es util, pone en 0 dnp_counter y retorna 1
{

	unsigned char * crc, i;
    packetout->data = packetin;
    packetout->issy	= 0;  ///PILAS falta determinar ISSY
    packetout->dnp 	= dnp_counter;


    //Calculo de CRC-8
    init_crc8();
    crc=&(packetout->crc);
    *crc=0;
    crc8(crc, packetin.sync_byte);
    crc8(crc, packetin.pid[0]);
    crc8(crc, packetin.pid[1]);
    crc8(crc, packetin.counter);
    for(i=0;i<184;i++)
        crc8(crc, packetin.data_byte[i]);

}


void imprimir_UP(UP x)
//------------------------------------------------------------
//Función para imprimier un TS_packet_field
{
    int i;

       printf("sync_byte: \t\t\t\t%x\n", x.data.sync_byte);
       printf("transport_error_indicator: \t\t%x\n", (x.data.pid[0]&0x80)>>7);
       printf("payload_unit_start_indicator: \t\t%x\n", (x.data.pid[0]&0x40)>>6);
       printf("transport_priority: \t\t\t%x\n", (x.data.pid[0]&0x20)>>5);
       printf("PID: \t\t\t\t\t%x\n", ((unsigned short)(x.data.pid[0]&0x1F)<<8 )+ x.data.pid[1]);
       printf("transport_scrambling_control: \t\t%x\t\n", (x.data.counter&0xC0)>>6);
       printf("adaption_field_control: \t\t%x\t\n", (x.data.counter&0x30)>>4);
       printf("continuity_counter: \t\t\t%x\t\n", (x.data.counter&0x0F));
       printf("data_byte:\n");
       for(i=0;i<184;i++)
            printf("%x", x.data.data_byte[i]);
       printf("\n\nISSY: \t\t\t\t%x\n", x.issy);
       printf("DNP: \t\t\t\t%x\n", x.dnp);
       printf("CRC8: \t\t\t\t%x\n", x.crc);     
            

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
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':   r = a-'a'+10;
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


//------------------------------------------------------------
//Convierte dos char a hexadecimal
unsigned char convHex2(char a, char b)
{
    char r=0;

    r = (convHex(a)<<4) | convHex(b);

    return r;
}



//------------------------------------------------------------
//Programa principal
int main ()
{
  FILE * pFile;
  //char x[MAX][MAXNUM];
  TS_packet_field x;
  UP	up;
  char y[188*2];
  unsigned char temp[100],a ,b;
  short c;

  int i,j;


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
            x.pid[0]    = convHex2(y[2], y[3]);
            x.pid[1]    = convHex2(y[4], y[5]);
            x.counter   = convHex2(y[6], y[7]);
            for(i=0, j=8;i<184;i++, j+=2)
                x.data_byte[i] = convHex2(y[j], y[j+1]);

        }

       printf("\n\n");
       puts(temp);


       get_useful_packet(x, dnp_counter, &up);
       imprimir_UP(up);


      fclose (pFile);
  }



  system("PAUSE");
  return 0;
}
