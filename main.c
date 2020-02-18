#include <stdio.h>
#include <string.h>
#include "st7580/include/st7580.h"


#define TRIG_BUF_SIZE   21
#define ACK_BUF_SIZE   	17

char MsgOut[100];

static int sleep_ms(unsigned int milliseconds)
{
    const time_t seconds = milliseconds / 1000U;
    const long   nanoseconds = (milliseconds % 1000UL) * 1000UL * 1000UL;
    const struct timespec request =
        { seconds,
          nanoseconds };
    return nanosleep(&request, NULL);
}


void AppliMasterBoard(void)
{
	uint8_t ret;
	uint8_t cRxLen;
	ST7580Frame* RxFrame;
	uint8_t lastIDRcv = 0;
	int it = 0;

	uint8_t aTrsBuffer[TRIG_BUF_SIZE] = {'T','R','I','G','G','E','R',' ',\
																						'M','E','S','S','A','G','E',' ',\
																						'I','D',':',' ','@'};
	uint8_t aRcvBuffer[ACK_BUF_SIZE];

	printf("P2P Communication Test - Master Board Side\n\r\n\r");

	while(1)
	{
		/* Initialize Trigger Msg */
		aTrsBuffer[TRIG_BUF_SIZE-1]++;
		if (aTrsBuffer[TRIG_BUF_SIZE-1] > 'Z')
		{
			aTrsBuffer[TRIG_BUF_SIZE-1] = 'A';
		}

		printf("Iteration %d\n\r", ++it);

		/* Send Trigger Msg send */
		ret = ST7580DlData(DATA_OPT, aTrsBuffer, TRIG_BUF_SIZE, NULL);

		/* Check TRIGGER Msg send result */
		if(ret)
		{
			/* Transmission Error */
			printf("Trigger Transmission Err: %d\n\r", ret);
			continue;
		}

		printf("Trigger Msg Sent, ID: %c\n\r", aTrsBuffer[TRIG_BUF_SIZE-1]);

		/* Wait ACK Msg sent back from slave */
		RxFrame = NULL;
		for (int j=0;((j<10) && (RxFrame==NULL));j++)
		{
			RxFrame = ST7580NextIndicationFrame();
			if (RxFrame != NULL)
			{
				/* Check if a duplicated indication frame with STX = 03 is received */
				if ((RxFrame->stx == ST7580_STX_03)&&(lastIDRcv == RxFrame->data[3+ACK_BUF_SIZE]))
				{
					RxFrame = NULL;
				}
				else
				{
					lastIDRcv = RxFrame->data[3+ACK_BUF_SIZE];
					break;
				}
			}
			sleep_ms(200);
		}
		/* Check received ACK Msg */
		if (RxFrame == NULL)
		{
			/* No ACK Msg received until timeout */
			printf("ACK Timeout - No ACK Received\n\r");
			continue;
		}

		cRxLen = (RxFrame->length - 4);

		printf("ACK Msg Received\n\r");

		if (cRxLen != ACK_BUF_SIZE){
			/* ACK len mismatch */
			printf("Wrong ACK Length\n\r");
			continue;
		}

		/* Copy payload from RX frame */
		memcpy(aRcvBuffer,&(RxFrame->data[4]),cRxLen);

		/* Check ID to verify if the right ACK has been received */
		if (aRcvBuffer[ACK_BUF_SIZE-1] == aTrsBuffer[TRIG_BUF_SIZE-1])
		{
			printf("ACK Msg Received, ID: %c \n", aRcvBuffer[ACK_BUF_SIZE-1]);
		}
		else
		{
			printf("WRONG ACK Msg Received, ID: %c \n", aRcvBuffer[ACK_BUF_SIZE-1]);
		}
		printf("PAYLOAD: %s\n", aRcvBuffer);

		sleep_ms(1000);
	}
	return;
}

void AppliSlaveBoard(void)
{
	ST7580Frame* RxFrame;
	uint8_t cRxLen;
	int ret;
	uint8_t lastIDRcv = 0;
	int it = 0;

	uint8_t aTrsBuffer[ACK_BUF_SIZE] = {'A','C','K',' ','M','E','S','S',\
																						'A','G','E',' ','I','D',':',' ',\
																						'@'};
	uint8_t aRcvBuffer[TRIG_BUF_SIZE];

	printf("P2P Communication Test - Slave Board Side\n\r\n\r");


	while(1)
	{
		printf("Iteration %d\n\r", ++it);

		/* Receive Trigger Msg from M board */
		RxFrame=NULL;

		do
		{
			RxFrame = ST7580NextIndicationFrame();

			if (RxFrame != NULL)
			{
				/* Check if a duplicated indication frame with STX = 03 is received */
				if ((RxFrame->stx == ST7580_STX_03)&&(lastIDRcv == RxFrame->data[3+TRIG_BUF_SIZE]))
				{
					RxFrame = NULL;
				}
				else
				{
					lastIDRcv = RxFrame->data[3+TRIG_BUF_SIZE];
					break;
				}
			}
			sleep_ms(200);
		} while(RxFrame==NULL);

		cRxLen = (RxFrame->length - 4);
		memcpy(aRcvBuffer,&(RxFrame->data[4]),cRxLen);

		printf("Trigger Msg Received, ID: %c\n\r", aRcvBuffer[TRIG_BUF_SIZE-1]);

		printf("PAYLOAD: ");
		sprintf(MsgOut, "%s", aRcvBuffer);
		printf("%s\n\r", MsgOut);

		/* Send back ACK Msg to Master Board */
		aTrsBuffer[ACK_BUF_SIZE-1] = aRcvBuffer[TRIG_BUF_SIZE-1];
		do
		{
			ret = ST7580DlData(DATA_OPT, aTrsBuffer, ACK_BUF_SIZE, NULL);
		} while (ret!=0);

		printf("ACK Msg Sent, ID: %c\n\r", aTrsBuffer[ACK_BUF_SIZE-1]);
		printf("PAYLOAD: ");
		sprintf(MsgOut, "%s", aTrsBuffer);
		printf("%s\n\r\n\r", MsgOut);
	}
}

int main(int argc, char *argv[])
{
	printf("ST7580 \n");

	if (argc < 2)
	{
		printf("usage: %s master/slave\n", argv[0]);
		return -1;
	}

	ST7580InterfaceInit();

	printf("ST7580 INIT OK\n");

	printf("Writing MIB_MODEM_CONF\n");
	ST7580MibWrite(MIB_MODEM_CONF, modem_config, sizeof(modem_config));
	sleep_ms(500);
	printf("Writing MIB_PHY_CONF\n");
	ST7580MibWrite(MIB_PHY_CONF, phy_config, sizeof(phy_config));

	if(!strcmp("master", argv[1]))
	{
		AppliMasterBoard();
	}
	else
	{
		AppliSlaveBoard();
	}

	return 0;
}
