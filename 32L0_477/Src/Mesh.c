#include "mesh.h"

//get ID for the given node
//return either the ID that is usable or -1 for error
uint8_t get_ID(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs){

	uint8_t my_ID = 1;
	uint8_t packet_ID = 1;
	while(req_ID(hspi2,cs,my_ID,packet_ID))
	{
		//ID is already_taken
		my_ID++;
		packet_ID++;
		HAL_Delay(5000);
	}

	return my_ID;
}

//request ID
bool req_ID(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs, uint8_t dest_ID, uint8_t packet_ID)
{
	uint8_t data[256];

	//Set the Interrupt
//	interrupt = OTHERS;

	//set the packet to sent out accordingly
	//(REQ_IQ, destination_ID, packet_ID)
	uint8_t req_ACK_packet[4];
	req_ACK_packet[0] = REQ_ID;
	req_ACK_packet[1] = dest_ID;
	req_ACK_packet[2] = packet_ID;
	req_ACK_packet[3] = 0x0;

	//Send the packet out
	send(hspi2, cs, req_ACK_packet);

	//Set timer to timeout (10 seconds)
	set_tim2(10);

//	while(true){
//		interrupt = OTHERS;
//		setModeRx(hspi2,cs);
//		do{asm("wfi");}while(interrupt == OTHERS);
//		if(interrupt == TIMEOUT) //timeouts
//		{
//			return false;
//		}
//		else if(interrupt == LORA_PACKET)//got a response
//		{
//			//check type and destination
//			spiReadbuff(hspi2,cs,data);
//			if(data[TYPE] == RESP_ID && data[TRANS_ID] == dest_ID)
//			{
//				stop_tim2();
//				return true;
//			}
//		}
//	}
}

//respond ID (receives req_ID and resp_ID)
// REQ_ID : check if it is for self, then respond, else broadcast
// RESP_ID: broadcast away
void resp_ID(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs, uint8_t* packet_data)
{
	if(packet_data[TYPE] == REQ_ID){
		if(packet_data[TRANS_ID] == self_ID && packet_data[UUID] != req_ACK_UUID){ //needs resp
			//Make packet
			//(RESP_ID, dest_ID, packet_ID)
			uint8_t resp_ACK_packet[4];
			resp_ACK_packet[0] = RESP_ID;
			resp_ACK_packet[1] = packet_data[TRANS_ID];
			resp_ACK_packet[2] = packet_data[UUID];
			resp_ACK_packet[3] = 0x0;

			//Send the packet out
			send(hspi2, cs, resp_ACK_packet);

			UART_send("Respond\x0D\x0A");
		}
		else if(packet_data[UUID] != req_ACK_UUID){
			send(hspi2, cs, packet_data);
			UART_send("Broadcast\x0D\x0A");
		}
		req_ACK_UUID = packet_data[UUID];
	}
	else if(packet_data[TYPE] == RESP_ID && packet_data[UUID] != resp_ACK_UUID){
		send(hspi2,cs, packet_data);
		resp_ACK_UUID = packet_data[UUID];

		UART_send("BackProp\x0D\x0A");
	}
}

//request an ACK from dest_ID
//return a bool: true if ACK is received, else false if timeout
bool req_ACK(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs, uint8_t dest_ID)
{
	uint8_t data[256];

	//set the packet to sent out accordingly
	//(REQ_ACK, sender_ID, dest_ID)
	uint8_t req_ACK_packet[4];
	req_ACK_packet[0] = REQ_ACK;
	req_ACK_packet[1] = self_ID;
	req_ACK_packet[2] = dest_ID;
	req_ACK_packet[3] = 0x0;

	//Send the packet out
	send(hspi2, cs, req_ACK_packet);

	//Set timer to timeout (10 seconds)
	set_tim2(5);

//	while(true){
//		interrupt = OTHERS;
//		setModeRx(hspi2,cs);
//		do{asm("wfi");}while(interrupt == OTHERS);
//		if(interrupt == TIMEOUT) //timeouts
//		{
//			return false;
//		}
//		else if(interrupt == LORA_PACKET)//got a response
//		{
//			//check type and destination
//			spiReadbuff(hspi2,cs,data);
//			if(data[TYPE] == RESP_ACK && data[ACK_REQ_ID] == self_ID && data[ACK_RESP_ID] == dest_ID)
//			{
//				stop_tim2();
//				return true;
//			}
//		}
//	}
}

//take in all "ACK type" packet and then output accordingly
void resp_ACK(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs, uint8_t* packet_data){

	if(packet_data[TYPE] == REQ_ACK) //Is corerct packet type
	{
		if(packet_data[ACK_RESP_ID] == self_ID){ //Needs respond
			//Make packet
			//(RESP_ACK, dest_ID, self_ID)
			uint8_t resp_ACK_packet[4];
			resp_ACK_packet[0] = RESP_ACK;
			resp_ACK_packet[1] = packet_data[ACK_REQ_ID];
			resp_ACK_packet[2] = packet_data[ACK_RESP_ID];
			resp_ACK_packet[3] = 0x0;

			//Send the packet out
			send(hspi2, cs, resp_ACK_packet);

			//Update your own timeout_table
			Update_timeout(packet_data[ACK_REQ_ID]);
		}
	}
}

//Functions for Timeout Table
void Init_timeout(){
	for(int i = 0; i < MAX_NODE; i++)
	{
		timeout[i].router_ID = 0;
		timeout[i].timeout = 0;
		timeout[i].active = false;
	}
}

void Update_timeout(uint8_t updated_node){
	for(int i = 0; i < MAX_NODE; i++)
	{
		if(timeout[i].router_ID == updated_node && timeout[i].active == true)
		{
			timeout[i].timeout = HAL_GetTick();
			return;
		}
	}

	//If not found in router, it means it died and was restarted
	for(int i = 0; i < MAX_NODE; i++)
	{
		if(timeout[i].active == false)
		{
			timeout[i].router_ID = updated_node;
			timeout[i].timeout = HAL_GetTick();
			timeout[i].active = true;
			return;
		}
	}

}

void Check_timeout(){
	uint32_t crnt_time = HAL_GetTick();
	uint32_t diff;
	for(int i = 0; i < MAX_NODE; i++)
	{
		if(timeout[i].active == true)
		{
			diff = crnt_time - timeout[i].timeout;
			if(diff > TIMEOUT_PERIOD * 1000)
			{
				UART_send("Killed");
				Delete_Router(timeout[i].router_ID);
				Delete_timeout(timeout[i].router_ID);
				Print_RT();
				print_timeout();
			}
		}
	}
}
void Delete_timeout(uint8_t node){ //kill the node
	for(int i = 0; i < MAX_NODE; i++)
	{
		if(timeout[i].active == true && timeout[i].router_ID == node)
		{
			timeout[i].router_ID = 0;
			timeout[i].timeout = 0;
			timeout[i].active = false;
			break;
		}
	}
}

void print_timeout(){
	for(int i = 0; i < MAX_NODE; i++)
	{
		if(timeout[i].active == true)
		{
			uint8_t msg[30];
			sprintf(msg,"ID: %d, Timeout: %d, Active: %d", timeout[i].router_ID, timeout[i].timeout, timeout[i].active);
			UART_send(msg);

		}
	}
}

//Functions for Routing Table
void Init_RT(){ //Init everything to all 1's
	for(int i = 0; i < MAX_NODE; i++){
		rt[i].dest_ID 	= 0;
		rt[i].active	= 0;
		rt[i].next_hop	= 0;
		rt[i].num_hop	= 0;
		rt[i].signal	= 0;
		rt[i].battery	= 0;
		rt[i].fire		= 0;
		for(int j = 0; j < MAX_NODE; j++)
		{
			rt[i].route[j]	= 0;
		}
	}

	rt[0].dest_ID = self_ID;
	rt[0].active = 1;
	rt[0].next_hop = self_ID;
	rt[0].battery = self_battery;
}
void Convert_Table_to_Pkt(uint8_t* packet){ //packet should be min size 152 for max_node 10
	packet[TYPE] 		= UPDATE_PACKET;
	packet[SENDER_ID]	= self_ID;
	for(int i = 0; i < MAX_NODE; i++)
	{
		packet[(i*17)+2] = rt[i].dest_ID != 0 ? rt[i].dest_ID : 0xFF;
		packet[(i*17)+3] = rt[i].active != 0 ? rt[i].active : 0xFF;
		packet[(i*17)+4] = rt[i].next_hop != 0 ? rt[i].next_hop : 0xFF;
		packet[(i*17)+5] = rt[i].num_hop != 0 ? rt[i].num_hop : 0xFF;
		packet[(i*17)+6] = rt[i].signal != 0 ? rt[i].signal : 0xFF;
		packet[(i*17)+7] = rt[i].battery != 0 ? rt[i].battery : 0xFF;
		packet[(i*17)+8] = rt[i].fire != 0 ? rt[i].fire : 0xFF;
		for(int j = 0; j < MAX_NODE; j++)
		{
			packet[(i*17)+j+9] = rt[i].route[j] != 0 ? rt[i].route[j] : 0xFF;
		}
	}
}

void Convert_Pkt_to_Table(uint8_t* packet){
	sender_ID = packet[SENDER_ID];
	for(int i = 0; i < MAX_NODE; i++)
	{
		recv_rt[i].dest_ID	= packet[(i*17)+2] != 0xFF ? packet[(i*17)+2] : 0;
		recv_rt[i].active	= packet[(i*17)+3] != 0xFF ? packet[(i*17)+3] : 0;
		recv_rt[i].next_hop	= packet[(i*17)+4] != 0xFF ? packet[(i*17)+4] : 0;
		recv_rt[i].num_hop	= packet[(i*17)+5] != 0xFF ? packet[(i*17)+5] : 0;
		recv_rt[i].signal	= packet[(i*17)+6] != 0xFF ? packet[(i*17)+6] : 0;
		recv_rt[i].battery  = packet[(i*17)+7] != 0xFF ? packet[(i*17)+7] : 0;
		recv_rt[i].fire		= packet[(i*17)+8] != 0xFF ? packet[(i*17)+8] : 0;
		for(int j = 0; j < MAX_NODE; j++)
		{
			recv_rt[i].route[j]	= packet[(i*17)+j+9] != 0xFF ? packet[(i*17)+j+9] : 0;
		}
	}
}

void Print_RT(){
	for(int i = 0; i < MAX_NODE; i++)
	{
		if(rt[i].active != 0)
		{
			uint8_t msg[256];
			sprintf(msg,"ID: %d, Active: %d, Next_hop: %d, num_hop: %d, signal: %d, battery: %d, fire: %d", rt[i].dest_ID,
					rt[i].active, rt[i].next_hop, rt[i].num_hop, rt[i].signal, rt[i].battery, rt[i].fire);
			UART_send(msg);
			sprintf(msg,"%d -> %d -> %d -> %d -> %d -> %d -> %d -> %d -> %d -> %d",
					rt[i].route[0],rt[i].route[1],rt[i].route[2],rt[i].route[3],rt[i].route[4],
					rt[i].route[5],rt[i].route[6],rt[i].route[7],rt[i].route[8],rt[i].route[9]);
			UART_send(msg);
		}

		//For recv_rt
//		if(recv_rt[i].active == 0xFF)
//		{
//			uint8_t msg[256];
//			sprintf(msg,"ID: %d, Active: %d, Next_hop: %d, num_hop: %d, signal: %d", recv_rt[i].dest_ID,
//					recv_rt[i].active, recv_rt[i].next_hop, recv_rt[i].num_hop, recv_rt[i].signal);
//			UART_send(msg);
//			sprintf(msg,"%d -> %d -> %d -> %d -> %d -> %d -> %d -> %d -> %d -> %d",
//					recv_rt[i].route[0],recv_rt[i].route[1],recv_rt[i].route[2],recv_rt[i].route[3],recv_rt[i].route[4],
//					recv_rt[i].route[5],recv_rt[i].route[6],recv_rt[i].route[7],recv_rt[i].route[8],recv_rt[i].route[9]);
//			UART_send(msg);
//		}
	}
}

void Update_Packet(){ //Ensure that recv_packet is updated using Convert_Pkt_to_Table (index i for recv_rt and j for rt)
	//update myself
	for(int j = 0; j < MAX_NODE; j++)
	{
		if(rt[j].active == 1 && rt[j].dest_ID == self_ID)
		{
			rt[j].signal = self_signal;
			rt[j].battery = self_battery;
			rt[j].fire = self_fire;
			break;
		}
	}
	//update the sender to the routing table first
	bool sender_in_rt = false;
	for(int i = 0; i < MAX_NODE; i++)
	{
		if(recv_rt[i].active == 1 && recv_rt[i].dest_ID == sender_ID) // found in recv_rt (always true tbh) (always index 0)
		{
			for(int j = 0; j < MAX_NODE; j++) //try to find sender_ID in rt
			{
				if(rt[j].active == 1 && rt[j].dest_ID == sender_ID) // found in rt
				{
					sender_in_rt = true;
					rt[j].next_hop = sender_ID;
					rt[j].num_hop = 1;
					rt[j].signal = recv_rt[i].signal;
					rt[j].battery = recv_rt[i].battery;
					rt[j].fire 	= recv_rt[i].fire;
					rt[j].route[0] = self_ID; rt[j].route[1] = recv_rt[i].dest_ID; rt[j].route[2] = 0;
				}
			}

			if(sender_in_rt == false) //can't find sender_ID in rt
			{
				for(int j = 0; j < MAX_NODE; j++) //find an empty table route
				{
					if(rt[j].active != 1) // found empty table route
					{
						rt[j].dest_ID = recv_rt[i].dest_ID;
						rt[j].active = 1;
						rt[j].next_hop = sender_ID;
						rt[j].num_hop = 1;
						rt[j].signal = recv_rt[i].signal;
						rt[j].battery = recv_rt[i].battery;
						rt[j].fire = recv_rt[i].fire;
						rt[j].route[0] = self_ID; rt[j].route[1] = recv_rt[i].dest_ID; rt[j].route[2] = 0;
						break;
					}
				}
			}
		}
	}

	//update the rest of the table
	for(int i = 0; i < MAX_NODE; i++)
	{
		if(recv_rt[i].active == 1 && recv_rt[i].dest_ID != sender_ID){ //the crnt route is valid

			bool in_rt = false; //check if ID is alr in routing table
			for(int j = 0; j < MAX_NODE; j++) //go through the routing table for the spot
			{
				if(rt[j].active == 1 && rt[j].dest_ID == recv_rt[i].dest_ID) //found in the own routing table
				{
					in_rt = true;

					//check if self is in path
					bool in_path = false;
					for(int k = 0; k < MAX_NODE; k++){
						if(recv_rt[i].route[k] == self_ID) {in_path=true;break;}
					}

					//in_path == false && (check for force update || lower cost)
					if(in_path == false && (rt[j].next_hop == sender_ID || rt[j].num_hop > (recv_rt[i].num_hop + 1)) ){
						rt[j].next_hop = sender_ID;
						rt[j].num_hop = recv_rt[i].num_hop + 1;
						rt[j].signal = recv_rt[i].signal;
						rt[j].battery = recv_rt[i].battery;
						rt[j].fire = recv_rt[i].fire;
						rt[j].route[0] = self_ID;
						for(int k = 0; k < (MAX_NODE - 1); k++){
							rt[j].route[k+1] = recv_rt[i].route[k];
						}
					}

					break;
				}
			}

			if(in_rt == false) // fail to find ID in rt
			{
				//check if self is in path
				bool in_path = false;
				for(int k = 0; k < MAX_NODE; k++){
					if(recv_rt[i].route[k] == self_ID) {in_path=true;break;}
				}

				if(in_path == false) //if not in path, add to rt
				{
					for(int j = 0; j < MAX_NODE; j++)
					{
						if(rt[j].active != 1){
							rt[j].dest_ID = recv_rt[i].dest_ID;
							rt[j].active = 1;
							rt[j].next_hop = sender_ID;
							rt[j].num_hop = recv_rt[i].num_hop + 1;
							rt[j].signal = recv_rt[i].signal;
							rt[j].battery = recv_rt[i].battery;
							rt[j].fire = recv_rt[i].fire;
							rt[j].route[0] = self_ID;
							for(int k = 0; k < (MAX_NODE-1); k++){
								rt[j].route[k+1] = recv_rt[i].route[k];
							}
							break;
						}
					}
				}
			}

		}
	}

	//After updating table, check that all route using this node is still active, else kill it
	//E.g
	// 1 -> 3 && 1 -> 3 -> 2
	// If 3 doesnt have 3 -> 2, kill 1 -> 3 -> 2
	for(int j = 0 ; j < MAX_NODE; j++)
	{
		//route is active && sender_ID is next_hop
		if(rt[j].active == 1 && rt[j].next_hop == sender_ID){

			bool found_in_recv_rt = false;
			for(int i = 0; i <MAX_NODE; i++){
				//found in recv_rt and is well and active
				if(recv_rt[i].dest_ID == rt[j].dest_ID && recv_rt[i].active)
				{
					found_in_recv_rt = true;
					break;
				}
			}

			//delete route if can't find in recv_rt
			if(found_in_recv_rt == false)
			{
				Delete_Router(rt[j].dest_ID);
			}
		}
	}
}

void Update_Self()
{
	//update myself
	for(int j = 0; j < MAX_NODE; j++)
	{
		if(rt[j].active == 1 && rt[j].dest_ID == self_ID)
		{
			rt[j].signal = self_signal;
			rt[j].battery = self_battery;
			rt[j].fire = self_fire;
			break;
		}
	}
}
void Delete_Router(uint8_t router)
{
	for(int i = 0; i < MAX_NODE; i++){
		if(rt[i].active == 1)
		{
			if(rt[i].dest_ID == router) //route of dead router
			{
				//delete everything
				rt[i].dest_ID = 0;
				rt[i].active = 0;
				rt[i].next_hop = 0;
				rt[i].num_hop = 0;
				rt[i].signal = 0;
				rt[i].battery = 0;
				rt[i].fire = 0;
				for(int j = 0; j < MAX_NODE; j++){
					rt[i].route[j] = 0;
				}
			}
			else //other router
			{
				bool dead_in_route = false;
				for(int j = 0; j < MAX_NODE; j++){
					if(rt[i].route[j] == router){ // current route uses the dead router
						dead_in_route = true;
						break;
					}
				}
				if(dead_in_route == true){ //delete route
					//delete everything
					rt[i].dest_ID = 0;
					rt[i].active = 0;
					rt[i].next_hop = 0;
					rt[i].num_hop = 0;
					rt[i].signal = 0;
					rt[i].battery = 0;
					rt[i].fire = 0;
					for(int j = 0; j < MAX_NODE; j++){
						rt[i].route[j] = 0;
					}
				}
			} //end_if_else
		}//end_active
	}//end_for
}
