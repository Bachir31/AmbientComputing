#include <Arduino.h>
#include <string.h>

/*
  Turns on an LED on for one second, then off for one second, repeatedly.
*/

/* constants that define the size for each part of trame */
const int LED_PIN = 13;
const int SIZE_HEADER = 2;
const int SIZE_ADR = 4;
const int SIZE_CHEKSUM = 2;
const int SIZE_TAIL = 2;
const int SIZE_INFO = 2;
const int SIZE_CD_FUNC = 1;
const int SIZE_CD_SS_FUNC = 1;

/* construction of tram */
typedef struct{
    byte head[SIZE_HEADER];
    byte adr_dst[SIZE_ADR];
    short size_info;
    byte cd_func;
    byte cd_ss_func;
    byte* data;
    byte chksum[SIZE_CHEKSUM];
    byte tail[SIZE_TAIL];
}struct_message;

typedef struct_message* Message;
typedef byte* Trame;
short checksum(Message* trame);

/* function that create a tram */
Message create_message(byte adr_dst[], byte *data, short size_data){
    int i = 0;
    byte* b = NULL;
    short size_info;

    if(adr_dst==NULL){
        return NULL;
    }

    if(sizeof(adr_dst)!= SIZE_ADR){
        return NULL;
    }

    if(data == NULL){
        return NULL;
    }

    Message message = (Message)malloc(sizeof(struct_message));

    if(message == NULL) {
        return NULL;
    }

    /*filling header*/
    message->head[0] = 0x01;
    message->head[1] = 0xFF;

    /*filling tail*/
    message->tail[0] = 0x02;
    message->tail[1] = 0xFF;

    memcpy(message->adr_dst, adr_dst, SIZE_ADR);

    message->size_info = (SIZE_INFO + SIZE_CD_FUNC + SIZE_CD_SS_FUNC  + size_data)*sizeof(byte);

    message->data = (byte*)malloc(size_data * sizeof(byte));
    memcpy(message->data, data, size_data * sizeof(byte));

    return message;
}

void free_Message(Message m) {
    int i = 0;
    int size_message = 0;

    if(m == NULL) {
        return;
    }
    free(m->data);
    free(m);
}

int sizeOfMessage(Message m){
    int nb_bytes;
    if(m == NULL) {
        return -1;
    }

    nb_bytes = SIZE_HEADER + SIZE_ADR + m->size_info + SIZE_CHEKSUM + SIZE_TAIL;

    return nb_bytes;
}

Trame convertMessageToTrame(Message m) {
    int i = 0;
    int j = 0;
    int index = 0;
    int nb_bytes;
    int size_data;
    Trame trame = NULL;

    if(m == NULL) {
        return NULL;
    }

    nb_bytes = sizeOfMessage(m);
    size_data = m->size_info - SIZE_INFO - SIZE_CD_FUNC - SIZE_CD_SS_FUNC;
    trame = (Trame) malloc(nb_bytes * sizeof(byte));

    if(trame == NULL) {
        return NULL;
    }

    for( i; i < SIZE_HEADER ; i++ ) {
        trame[i] = m->head[i];
    }

    index = i+SIZE_ADR;
    for( i; i < index; i++) {
        trame[i] = m->adr_dst[j++];
    }

    /*converting short into 2 bytes */
    trame[i++] = (m->size_info >> 8);
    trame[i++] = (m->size_info & 0xFF);

    /*adding code function and code sub-function*/
    trame[i++] = m->cd_func;
    trame[i++] = m->cd_ss_func;

    j=0;
    index = i+size_data;
    for( i; i < index; i++ ) {
        trame[i] = m->data[j];
    }

    return trame;
}

short checksum(Message* trame) {
    short checksumValue = 0;
    short sizeOfTrame = sizeof(trame);

    sizeOfTrame = (sizeOfTrame - SIZE_HEADER - SIZE_TAIL)*sizeof(byte);

    checksumValue = sizeOfTrame % 65536;

    return checksumValue;
}

void print_Message(Message m) {
    int i = 0;

    //error if null
    if(m == NULL) {
        return;
    }

    printf("\n\t=================================== \n");
    /*printing header*/
    for(i; i < SIZE_HEADER; i++) {
        printf("header[%d] : %x \n", i, m->head[i]);
    }

    /*printing address destination*/
    i = 0;
    for(i; i < SIZE_ADR; i++) {
        printf("address destination[%d] : %x \n", i, m->adr_dst[i]);
    }

    /*printing size information*/
    printf("size information : %d \n", m->size_info );

    /*printing data */
    printf("data : %s \n",m->data);

    /*printing chucksum */
    printf("Chuksum : %d \n",m->chksum);

    /*printing queue*/
    i = 0;
    for(i; i < SIZE_TAIL; i++) {
        printf("tail[%d] : %x \n", i, m->tail[i]);
    }
    printf("\n\t=================================== \n");
}



void setup()
{
	Serial.begin(9600);
	pinMode(LED_PIN, OUTPUT);
}

char* byteToString(byte b, char *str)
{
    sprintf(str,"%.2X ", b);
    return str;
}

void debugFrame(byte* frame, int size)
{
    char tmp[4];
    for(int i=0 ; i < size ; i++){
        Serial.print(byteToString(frame[i], tmp));
    }
    Serial.println("");
}

void loop()
{
    int i = 0;
    Serial.println("");
    Serial.println("");
	Serial.println("Hello world!");
	byte adr[4] = {0x01, 0x02, 0x03, 0x4};

	char data[6] = "hello";
    data[5] = '\0';

	byte byte_data[6];

	for(i; i<6;i++) {
        byte_data[i] = data[i];
	}

	Message message = create_message(adr, byte_data, 6);

    print_Message(message);

    Trame trame =  convertMessageToTrame(message);

    debugFrame(trame, sizeOfMessage(message));
	delay(1000);              // wait for a second
	digitalWrite(LED_PIN, HIGH);   // set the LED on
	delay(1000);              // wait for a second
	digitalWrite(13, LOW);    // set the LED off

	byte test[5] = {0x00, 0x41, 0x43, 0xFF};
	Serial.println("Using Serial.Write;");
	Serial.write(test,5);
	Serial.println("Using debugFrame ;");
	debugFrame(test, 5);
}
