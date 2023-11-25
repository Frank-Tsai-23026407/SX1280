#include "SX128x_OBJ.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;
extern int irq_timeout_times;

void SX128x_OBJ::CommonTransceiverSetting()
{
    SetStandby((RadioStandbyModes_t)MODE_STDBY_XOSC);
    SetPacketType(PACKET_TYPE_LORA);
    SetRfFrequency(RfFrequency);
    SetBufferBaseAddresses(0x80, 0x00);
    SetModulationParams(mparams);
    SetPacketParams(pparams);
}

void SX128x_OBJ::TxSetting()
{
	SetTxParams(13, RADIO_RAMP_02_US);
	InterruptSetting();
	SetDioIrqParams(0xFFFF, 0xFFFF, 0x0000, 0x0000);
}

uint8_t SX128x_OBJ::TxBlocking(uint8_t* data_out, uint8_t len)
{
	while(tx_activated); // wait not busy

	WriteBuffer(0x80, data_out, len);

	// set tx mode
	TickTime_t timeout;
	timeout.PeriodBase = RADIO_TICK_SIZE_0015_US;
	timeout.PeriodBaseCount = 0;
	SetTx(timeout);
//	RadioStatus_t status;

	// wait complete
	uint16_t irq;
	uint16_t t = __HAL_TIM_GET_COUNTER(&htim1);
	do{
		irq = GetIrqStatus();
		if (__HAL_TIM_GET_COUNTER(&htim1) - t >= 35535)
		{
			irq_timeout_times ++ ;
			return 1;
		}
	}while(!(irq & 0x0001));

	// clear irq
	ClearIrqStatus(0xFFFF);
	return 0;
}

inline void SX128x_OBJ::InterruptSetting()
{
	SetDioIrqParams(0x0003, 0x0003, 0x0000, 0x0000);
}

inline void SX128x_OBJ::InterruptClearing()
{
	ClearIrqStatus(0xFFFF);
}

uint8_t SX128x_OBJ::Init()
{
	Reset();
	// Wakeup();
	// SetRegistersDefault();

	uint8_t rx[2], tx[2];
	*(uint32_t*)tx = 0x80 | 0x01 << 8;
	HalSpiTransferDelay(rx, tx, 2);
	RadioStatus_t status;
	status = GetStatus();
	if(status.Fields.CmdStatus != 0x01)
		return 1;
	if(status.Fields.ChipMode != 0x03)
		return 1;

	CommonTransceiverSetting();
	return 0;
}

SX128x_OBJ::SX128x_OBJ()
{
	mparams.PacketType = PACKET_TYPE_LORA;
    mparams.Params.LoRa.SpreadingFactor = LORA_SF7;
	mparams.Params.LoRa.Bandwidth = LORA_BW_0800;
	mparams.Params.LoRa.CodingRate = LORA_CR_4_5;

	pparams.PacketType = PACKET_TYPE_LORA;
	pparams.Params.LoRa.HeaderType = LORA_PACKET_FIXED_LENGTH;
	pparams.Params.LoRa.InvertIQ = LORA_IQ_NORMAL;
	pparams.Params.LoRa.Crc = LORA_CRC_ON;
	pparams.Params.LoRa.PayloadLength = PACKET_SIZE;
	pparams.Params.LoRa.PreambleLength = 12;

//	callbacks.txDone = &tx_recursion();
	
}

uint8_t SX128x_OBJ::PutPacket(uint8_t* in)
{

	if(tx_length == FIFO_SIZE) {
		return 1; // FIFO_OVERFLOW
	} else {
		for(int i = 0; i < PACKET_SIZE; i++)
		{
			*(tx_eprt+i) 	= *(in+i);
		}
//		*(uint32_t*) tx_eprt 	= *(uint32_t*)in;
//		*(uint32_t*)(tx_eprt+4) = *(uint32_t*)(in+4);
		if(tx_eprt == tx_fifo[FIFO_SIZE - 1])
		{
			tx_eprt = tx_fifo[0];
		}
		else
		{
			tx_eprt += PACKET_SIZE;
		}
		tx_length ++;
		if(!tx_activated)
		{
			TickTime_t t;
			t.PeriodBase = RADIO_TICK_SIZE_0015_US;
			t.PeriodBaseCount = 0;
			tx_activated = 1;
			SendPayload(tx_sprt, PACKET_SIZE, t, 0x80);
		}
	}
	return 0;
}

inline uint8_t SX128x_OBJ::GetLength()
{
	return tx_length;
}

void SX128x_OBJ::tx_recursion()
{

	uint16_t irqRegs = GetIrqStatus();
	if(!(irqRegs|IRQ_TX_DONE))
		return;
	ClearIrqStatus(IRQ_TX_DONE);
	if(tx_sprt == tx_fifo[FIFO_SIZE - 1])
	{
		tx_sprt = tx_fifo[0];
	}
	else
	{
		tx_sprt += PACKET_SIZE;
	}
	tx_length--;
	if(tx_length >= 0)
	{
		tx_activated = 0;
		return;
	}
	TickTime_t t;
	t.PeriodBase = RADIO_TICK_SIZE_0015_US;
	t.PeriodBaseCount = 0;
	SendPayload(tx_sprt, PACKET_SIZE, t, 0x80);

}

void SX128x_OBJ::RxSetting()
{
//	SetTxParams(13, RADIO_RAMP_02_US);
//	InterruptSetting();
	SetDioIrqParams(0x4023, 0x0001, 0x0002, 0x4020);
//	SetDioIrqParams(0xFFFF, 0xFFFF, 0x0000, 0x0000);
}

uint8_t SX128x_OBJ::RxBlocking(uint8_t* data_in, uint8_t max_len)
{
	// set tx mode
	TickTime_t timeout;
	timeout.PeriodBase = RADIO_TICK_SIZE_0015_US;
	timeout.PeriodBaseCount = 0;
	SetRx(timeout);

	// wait message
	// while(!HAL_GPIO_ReadPin(GPIOA, GPIO_Pin_8));
	uint8_t tx[7] = {};
	uint8_t rx[7] = {};
	while(1)
	{
		*(uint32_t*)tx = 0x15 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
		HalSpiTransfer(rx, tx, 4);
		if(rx[3] & 0x02) break;
	}

	// GetPacketStatus
	*(uint32_t*)tx = 0x1D | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	*(uint32_t*)(tx+4) = 0x00 | 0x00 << 8 | 0x00 << 16;
	HalSpiTransfer(rx, tx, 7);

	// ClrIrqStatus
	ClearIrqStatus(IRQ_RX_DONE);

	// GetRxBufferStatus
	uint8_t payload_length, rx_pointer;
	GetRxBufferStatus(&payload_length, &rx_pointer);

	// ReadBuffer
	if(payload_length > max_len) return 0;
	ReadBuffer(rx_pointer, data_in, payload_length);

	// FrequencyError

	return payload_length;
}
