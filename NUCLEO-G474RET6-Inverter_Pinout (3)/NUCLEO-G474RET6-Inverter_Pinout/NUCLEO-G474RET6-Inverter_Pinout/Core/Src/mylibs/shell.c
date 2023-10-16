/*
 * shell.c
 *
 *  Created on: Oct 1, 2023
 *      Author: nicolas
 */
#include "usart.h"
#include "mylibs/shell.h"
#include <stdio.h>
#include <string.h>
#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;
extern float courant_mes;
extern float offset_courant;

uint8_t prompt[]="user@Nucleo-STM32G474RET6>>";
uint8_t started[]=
		"\r\n*-----------------------------*"
		"\r\n| Welcome on Nucleo-STM32G474 |"
		"\r\n*-----------------------------*"
		"\r\n";
uint8_t newline[]="\r\n";
uint8_t backspace[]="\b \b";
uint8_t cmdNotFound[]="Command not found\r\n";
uint8_t brian[]="Brian is in the kitchen\r\n";
uint8_t uartRxReceived;
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];

char	 	cmdBuffer[CMD_BUFFER_SIZE];
int 		idx_cmd;
char* 		argv[MAX_ARGS];
int		 	argc = 0;
char*		token;
int 		newCmdReady = 0;
int 		maxSpeed = 100;
int 		minSpeed = 0;
int			speed[50];
uint32_t    speedValue;
uint32_t 	courant[50];



void Shell_Init(void){
	memset(argv, NULL, MAX_ARGS*sizeof(char*));
	memset(cmdBuffer, NULL, CMD_BUFFER_SIZE*sizeof(char));
	memset(uartRxBuffer, NULL, UART_RX_BUFFER_SIZE*sizeof(char));
	memset(uartTxBuffer, NULL, UART_TX_BUFFER_SIZE*sizeof(char));

	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_UART_Transmit(&huart2, started, strlen((char *)started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
}

void Shell_Loop(void){
	if(uartRxReceived){
		switch(uartRxBuffer[0]){
		case ASCII_CR: // Nouvelle ligne, instruction à traiter
			HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
			cmdBuffer[idx_cmd] = '\0';
			argc = 0;
			token = strtok(cmdBuffer, " ");
			while(token!=NULL){
				argv[argc++] = token;
				token = strtok(NULL, " ");
			}
			idx_cmd = 0;
			newCmdReady = 1;
			break;
		case ASCII_BACK: // Suppression du dernier caractère
			cmdBuffer[idx_cmd--] = '\0';
			HAL_UART_Transmit(&huart2, backspace, sizeof(backspace), HAL_MAX_DELAY);
			break;

		default: // Nouveau caractère
			cmdBuffer[idx_cmd++] = uartRxBuffer[0];
			HAL_UART_Transmit(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
		}
		uartRxReceived = 0;
	}

	if(newCmdReady){

		if (strncmp(cmdBuffer,"speed=",strlen("speed="))==0)
		{

			speedValue= atoi(cmdBuffer+strlen("speed="));

			if (speedValue> maxSpeed)
			{
				speedValue= maxSpeed;

			}
			else if (speedValue< minSpeed)
			{
				speedValue = minSpeed;

			}
			//ChangeSpeed(speedValue);
			sprintf(speed,"Speed is changed to %d\r\n",speedValue);
			HAL_UART_Transmit(&huart2, speed, sizeof(speed), HAL_MAX_DELAY);
		}

		else if (strncmp(cmdBuffer,"start",strlen("stop"))==0)
				{
					HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1); //D11
					HAL_TIMEx_PWMN_Start(&htim1,TIM_CHANNEL_1);

					TIM1->CCR1 = (TIM1->ARR)*50/100;

					HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2); //D5
					HAL_TIMEx_PWMN_Start(&htim1,TIM_CHANNEL_2);

					TIM1->CCR2 = (TIM1->ARR)*50/100;




				}

		else if (strncmp(cmdBuffer,"courant",strlen("courant"))==0)
				{
					//courant_mes = read_current();
					courant_mes = 0;
					courant_mes = HAL_ADC_GetValue(&hadc1);
					sprintf(courant,"Current value is %f\r\n",courant_mes);
					HAL_UART_Transmit(&huart2, courant, sizeof(courant), HAL_MAX_DELAY);
				}

		else if (strncmp(cmdBuffer,"stop",strlen("stop"))==0)
		{
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1); //D11
			HAL_TIMEx_PWMN_Stop(&htim1,TIM_CHANNEL_1);
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_2); //D5
			HAL_TIMEx_PWMN_Stop(&htim1,TIM_CHANNEL_2);

		}

		else if(strcmp(argv[0],"help")==0){
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Print all available functions here\r\n");
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
		}
		else{
			HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
		}
		HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
		newCmdReady = 0;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}
