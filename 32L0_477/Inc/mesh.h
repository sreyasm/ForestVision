#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32l0xx_hal.h"
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

//definition for type of packet
#define REQ_ACT_T           0x01
#define RESP_ACT_T          0x02


//definition for interrupt type
#define OTHERS				   0
#define TIMEOUT                1
#define LORA_PACKET            2


//definition for packet type
#define REQ_ID                 1
#define RESP_ID                2
#define REQ_ACK				   3
#define RESP_ACK		 	   4
#define UPDATE_PACKET		   5

//definition for REQ_ID and RESP_ID
#define TYPE                   0
#define TRANS_ID               1 //aka self_ID
#define UUID 				   2

//definition for REQ_ACK and RESP_ACK
#define ACK_REQ_ID			   1
#define ACK_RESP_ID            2

//definition for Update Packet
#define SENDER_ID			   1


//function definition----------------------------------------------------
uint8_t get_ID(SPI_HandleTypeDef, GPIO_TypeDef*);
bool req_ID(SPI_HandleTypeDef, GPIO_TypeDef*, uint8_t, uint8_t);
void resp_ID(SPI_HandleTypeDef, GPIO_TypeDef*, uint8_t*);
bool req_ACK(SPI_HandleTypeDef, GPIO_TypeDef*,uint8_t);
void resp_ACK(SPI_HandleTypeDef, GPIO_TypeDef*,uint8_t *);

//Functions for Timeout Table
void Init_timeout();
void Update_timeout(uint8_t);
void Check_timeout();
void Delete_timeout(uint8_t);
void print_timeout();

//Functions for Routing Table
void Init_RT();
void Convert_Table_to_Pkt(uint8_t*);
void Convert_Pkt_to_Table(uint8_t*);
void Print_RT();
void Update_Packet();
void Update_Self();
void Delete_Router(uint8_t);
//END of function definition---------------------------------------------
