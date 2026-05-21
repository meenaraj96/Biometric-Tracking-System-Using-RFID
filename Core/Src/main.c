/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include "MFRC522_STM32.h"
#include "fonts.h"
#include "SH1106.h"
#include "LoRa.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    //unsigned char
	uint8_t uid_bytes[5]; // Array to store the 4 bytes of the UID
    char owner[50]; //const char* name; // Pointer to the user's name
    bool isTappedIn;
    RTC_TimeTypeDef InTime;
    RTC_TimeTypeDef OutTime;
    RTC_DateTypeDef InDate;
    RTC_DateTypeDef OutDate;
} RFID_Card;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//RC522 Pins
#define RC522_CS_GPIO_Port		GPIOA
#define RC522_CS_Pin 			GPIO_PIN_4
#define RC522_RESET_GPIO_Port	GPIOB
#define RC522_RESET_Pin			GPIO_PIN_0

//LoRa Pins
#define LoRa_NSS_GPIO_Port 	GPIOA
#define LoRa_NSS_Pin 		GPIO_PIN_8
#define LoRa_RST_GPIO_Port 	GPIOA
#define LoRa_RST_Pin 		GPIO_PIN_12
#define LoRa_DIO0_GPIO_Port	GPIOA
#define LoRa_DIO0_Pin 		GPIO_PIN_11

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
int LoRa_DeviceID_3 = 3;
char LoRa_Message[1000];

RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;
char *daysz[] = {"Error", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

uint32_t duration_buffer;
char id_buffer[30], id_buffer_date[30], id_buffer_time[30], id_buffer_duration[30];

LoRa myLoRa;
const char *send_req = "REQUEST_DATA3";
char rx_buffer[20];
int rssi;

volatile bool lora_irq_flag = false;
static uint8_t request_sent = 0;

static bool ReceiveFirstTimeFlag = false; // Declare a global flag variable
bool ReceiveSixHourTimeReqFlag = true; // Declare a global flag variable

volatile uint32_t total_seconds = 0;
static uint32_t last_trigger_time = 0;
volatile uint32_t tot_sec = 0;
volatile uint8_t total_min = 0;
volatile uint8_t total_hour = 0;
char display_buffer[150];
const uint32_t INTERVAL_6H = 21600; // 6 hours = 6 * 3600 = 21600 seconds // 21,600,000 ms
const uint32_t INTERVAL_4M = 240; // 4 minutes = 4 * 60 = 240 seconds

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
extern void initialise_monitor_handles(void); // Add this prototype
bool isSameUID(uint8_t *uid1, uint8_t *uid2);
int is_valid_epoch(time_t timestamp);
bool Parse_Epoch(char *data, uint32_t *epoch_out);
void updateRTC_fromEpoch(uint32_t epochTime_val);
void convertTime(uint32_t total_sec, uint8_t *h, uint8_t *m, uint8_t *s);
void displayDatabase();
void addNewCardLogic();

#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE {
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t uid[5];

MFRC522_t rfID = {&hspi1, RC522_CS_GPIO_Port, RC522_CS_Pin, RC522_RESET_GPIO_Port, RC522_RESET_Pin};

RFID_Card cardDatabase[50] = {
    //{{0x96, 0xA7, 0x33, 0x06}, "Meena", false},
    //{{0x72, 0x69, 0xE8, 0x5C}, "Akhila", false}
};

//to compare 2 UIDs
bool isSameUID(uint8_t *uid1, uint8_t *uid2) {
    return (memcmp(uid1, uid2, 4) == 0);
}


bool Parse_Epoch(char *data, uint32_t *epoch_out)
{
    // Expected: TIME:1709370205
    if (strncmp(data, "TimeDate,", 9) != 0)
        return false;

    uint32_t epoch = atoi(&data[9]); //ASCII to Integer

    // Basic validation (year 2020–2100 range)
    if (epoch < 1577836800 || epoch > 4102444800)
        return false;

    *epoch_out = epoch;
    return true;
}


int is_valid_epoch(time_t timestamp) {
    // Attempt to convert the timestamp to a UTC time structure
    struct tm *timeinfo = gmtime(&timestamp);

    if (timeinfo == NULL) {
        // gmtime returns NULL if the timestamp is out of the supported range for the system's time_t
        return 0; // Invalid
    }

    // Further validation can be done on the fields of struct tm.
    // For example, check if the year is within a reasonable range.
    // tm_year is years since 1900, so 1970 is tm_year >= 70
    if (timeinfo->tm_year < 70) {
        return 0; // Before the Unix epoch
    }

    // You could also check against a maximum reasonable date, e.g., the year 2038 problem limit if applicable
    if (timeinfo->tm_year > 138 && sizeof(time_t) == 4) {
         // This is a simplistic check for dates beyond the potential 2038 overflow on 32-bit systems
        return 0;
    }

    return 1; // Considered a valid/plausible epoch time
}

void convertTime(uint32_t total_sec, uint8_t *h, uint8_t *m, uint8_t *s) {
    *h = total_sec / 3600;          // Calculate hours
    *m = (total_sec % 3600) / 60;   // Calculate minutes from remainder
    *s = total_sec % 60;            // Calculate remaining seconds
}

void SPI_Deselect_All() {
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LoRa_NSS_GPIO_Port, LoRa_NSS_Pin, GPIO_PIN_SET);
}

void SPI_Select_RC522() {
    HAL_GPIO_WritePin(LoRa_NSS_GPIO_Port, LoRa_NSS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_RESET);
}

void SPI_Select_LORA() {
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LoRa_NSS_GPIO_Port, LoRa_NSS_Pin, GPIO_PIN_RESET);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(1);
  SH1106_Init();
  SH1106_Clear();

  SPI_Deselect_All();
  HAL_Delay(10);
  printf("\r\n");

  SPI_Select_RC522();
  USER_LOG("RFID Initiated");
  MFRC522_Init(&rfID);
  uint8_t version = MFRC522_ReadReg(&rfID, PCD_VersionReg);
  USER_LOG("RC522 Version: 0x%02X", version);
  SPI_Deselect_All();
  HAL_Delay(10);

  SPI_Select_LORA();
  myLoRa.hSPIx = &hspi1;
  myLoRa.CS_port = LoRa_NSS_GPIO_Port;
  myLoRa.CS_pin = LoRa_NSS_Pin;
  myLoRa.reset_port = LoRa_RST_GPIO_Port;
  myLoRa.reset_pin = LoRa_RST_Pin;
  myLoRa.DIO0_port = LoRa_DIO0_GPIO_Port;
  myLoRa.DIO0_pin = LoRa_DIO0_Pin;

  myLoRa.frequency = 433;			// default = 433 MHz
  myLoRa.spredingFactor = SF_10;	// default = SF_7
  myLoRa.bandWidth = BW_125KHz;	// default = BW_125KHz
  myLoRa.crcRate = CR_4_5;		// default = CR_4_5
  myLoRa.power = POWER_20db;		// default = 20db
  myLoRa.overCurrentProtection = 130;	// default = 100 mA
  myLoRa.preamble = 8;				// default = 8;

  LoRa_setSyncWord(&myLoRa, 0x12);

  LoRa_reset(&myLoRa);
  HAL_Delay(50);

  uint16_t LoRaStatus = LoRa_init(&myLoRa);
  initialise_monitor_handles(); // Call this before any printf
  printf("\nAfter Init: 0x%X\r\n", LoRa_read(&myLoRa, RegOpMode));
  if(LoRaStatus != LORA_OK) {
	  //Initialization failed, blink LED or error trap
	  printf("LoRa Initialization failed\r\n");
	  printf("LoRa Error Status: %d\r\n", LoRaStatus);
	  HAL_Delay(100);
	  while (1);
  }
 //Initialization failed, blink LED or error trap
  printf("LoRa Initialization Success\r\n");
  printf("LoRa Status Code: %d\r\n", LoRaStatus);

  for (int i = 0; i < 2; i++) {
      uint8_t v = LoRa_read(&myLoRa, RegVersion);
      printf("Version[%d]: 0x%X\r\n", i, v);
      HAL_Delay(500);
  }
  SPI_Deselect_All();
  HAL_Delay(10);

  HAL_TIM_Base_Start_IT(&htim2); // Start TIM2 in interrupt mode

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	SH1106_Clear();

	while (!ReceiveFirstTimeFlag) // First Time send Request
	{
		printf("First LoRa request...\r\n");
		goto SEND_REQUEST;
	}

	/*if (total_seconds % INTERVAL_6H == 0) // Every 6 hrs send Request
	{
		ReceiveSixHourTimeReqFlag = false;
	}
	else if (total_seconds % INTERVAL_6H == 1) // Every 6 hrs send Request
	{
		ReceiveSixHourTimeReqFlag = false;
	}*/

	if (total_seconds >= INTERVAL_6H &&
	    (total_seconds - last_trigger_time) >= INTERVAL_6H)
	{
	    last_trigger_time = total_seconds;
	    ReceiveSixHourTimeReqFlag = false;
	}

	while (!ReceiveSixHourTimeReqFlag)
	{
		printf("6-hour LoRa request...\r\n");
		sprintf(display_buffer, "Time Passed: %lu secs, %lu mins, %lu hrs\r\n", total_seconds, (total_seconds/60), (total_seconds/3600));
		HAL_UART_Transmit(&huart1, (uint8_t*)display_buffer, strlen(display_buffer), 100);
		request_sent = 0;
		goto SEND_REQUEST;
	}

	SH1106_GotoXY(5, 15);
	SH1106_Puts("^  Tap Your", &Font_7x10, 1);
	SH1106_GotoXY(5, 25);
	SH1106_Puts("| Access card", &Font_7x10, 1);
	SH1106_GotoXY(5, 35);
	SH1106_Puts("    Above", &Font_7x10, 1);
	SH1106_UpdateScreen();
	HAL_Delay(1000);

	// Card not stored in Database - Write new card Logic here // Check if this UID is already "Tapped In"
	addNewCardLogic();

	if (waitcardDetect(&rfID) == STATUS_OK){
		HAL_Delay(20);   // give clone chip time
		if (MFRC522_ReadUid(&rfID, uid) == STATUS_OK){
			USER_LOG("CARD ID:%02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3]);
			int index = -1;
			for(int i=0; i<25; i++)
			{
			  if (isSameUID(cardDatabase[i].uid_bytes, uid)) { // Check index of "Tapped In" UID
				  index = i;
				  break;
			  }
			}
			HAL_Delay(20);
			if (index != -1) {
				if (memcmp(uid,cardDatabase[index].uid_bytes,4) == 0) {
					if (!cardDatabase[index].isTappedIn) {
						SH1106_Clear();
						SH1106_GotoXY(5, 5);
						SH1106_Puts(cardDatabase[index].owner, &Font_7x10, 1);

						snprintf(id_buffer,sizeof(id_buffer),"** ** ** %02X",uid[3]);
						SH1106_GotoXY(10, 15);
						SH1106_Puts(id_buffer, &Font_7x10, 1);

						HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
						cardDatabase[index].InTime = sTime;
						snprintf(id_buffer_time, sizeof(id_buffer_time),"IN Time:%02d:%02d:%02d", cardDatabase[index].InTime.Hours, cardDatabase[index].InTime.Minutes, cardDatabase[index].InTime.Seconds);
						SH1106_GotoXY(1, 25);
						SH1106_Puts(id_buffer_time, &Font_7x10, 1);

						HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
						cardDatabase[index].InDate = sDate;
						snprintf(id_buffer_date, sizeof(id_buffer_date),"%02d/%02d/%04d %s", cardDatabase[index].InDate.Date, cardDatabase[index].InDate.Month, (cardDatabase[index].InDate.Year+2000), daysz[cardDatabase[index].InDate.WeekDay]);
						SH1106_GotoXY(1, 35);
						SH1106_Puts(id_buffer_date, &Font_7x10, 1);

						SH1106_GotoXY(5, 45);
						SH1106_Puts("Access Granted", &Font_7x10, 1);
						SH1106_UpdateScreen();
						HAL_Delay(2000);

						SPI_Deselect_All();
						HAL_Delay(10);

						char IN_Time_buffer[100], IN_Date_buffer[100];
						snprintf(IN_Time_buffer, sizeof(IN_Time_buffer), "%02d:%02d:%02d", cardDatabase[index].InTime.Hours, cardDatabase[index].InTime.Minutes, cardDatabase[index].InTime.Seconds);
						snprintf(IN_Date_buffer, sizeof(IN_Date_buffer), "%02d/%02d/%04d", cardDatabase[index].InDate.Date, cardDatabase[index].InDate.Month, (cardDatabase[index].InDate.Year+2000));

						snprintf(LoRa_Message, sizeof(LoRa_Message), "%d,%s,%s,%s,%02X %02X %02X %02X,IN,", LoRa_DeviceID_3, IN_Date_buffer, IN_Time_buffer, cardDatabase[index].owner, cardDatabase[index].uid_bytes[0], cardDatabase[index].uid_bytes[1], cardDatabase[index].uid_bytes[2], cardDatabase[index].uid_bytes[3]);
						if (LoRa_transmit(&myLoRa, (uint8_t*)LoRa_Message, strlen(LoRa_Message), 5000))
						{
							printf("Message sent to Gateway\r\n");
						}
						else
						{
							printf("LoRa TX failed\r\n");
						}

						HAL_Delay(6000);

						snprintf(LoRa_Message, sizeof(LoRa_Message), "%d,%s,%s,%s,%02X %02X %02X %02X,IN,", LoRa_DeviceID_3, IN_Date_buffer, IN_Time_buffer, cardDatabase[index].owner, cardDatabase[index].uid_bytes[0], cardDatabase[index].uid_bytes[1], cardDatabase[index].uid_bytes[2], cardDatabase[index].uid_bytes[3]);
						if (LoRa_transmit(&myLoRa, (uint8_t*)LoRa_Message, strlen(LoRa_Message), 5000))
						{
							printf("Message sent to Gateway\r\n");
						}
						else
						{
							printf("LoRa TX failed\r\n");
						}

						SPI_Deselect_All();
						HAL_Delay(10);

						cardDatabase[index].isTappedIn = true;
					}
					else
					{
						SH1106_Clear();
						SH1106_GotoXY(5, 5);
						SH1106_Puts(cardDatabase[index].owner, &Font_7x10, 1);

						snprintf(id_buffer,sizeof(id_buffer),"** ** ** %02X", uid[3]);
						SH1106_GotoXY(10, 15);
						SH1106_Puts(id_buffer, &Font_7x10, 1);

						HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
						cardDatabase[index].OutTime = sTime;
						snprintf(id_buffer_time, sizeof(id_buffer_time),"Out Time:%02d:%02d:%02d", cardDatabase[index].OutTime.Hours, cardDatabase[index].OutTime.Minutes, cardDatabase[index].OutTime.Seconds);
						SH1106_GotoXY(1, 25);
						SH1106_Puts(id_buffer_time, &Font_7x10, 1);

						HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
						cardDatabase[index].OutDate = sDate;
						snprintf(id_buffer_date,sizeof(id_buffer_date),"%02d/%02d/%04d %s", cardDatabase[index].OutDate.Date, cardDatabase[index].OutDate.Month, (cardDatabase[index].OutDate.Year+2000), daysz[cardDatabase[index].OutDate.WeekDay]);
						SH1106_GotoXY(1, 35);
						SH1106_Puts(id_buffer_date, &Font_7x10, 1);

						uint32_t inSeconds=0, outSeconds=0, diffSeconds=0;
						inSeconds = (cardDatabase[index].InTime.Hours * 3600) + (cardDatabase[index].InTime.Minutes * 60) + cardDatabase[index].InTime.Seconds;
						outSeconds = (cardDatabase[index].OutTime.Hours * 3600) + (cardDatabase[index].OutTime.Minutes * 60) + cardDatabase[index].OutTime.Seconds;

						if (outSeconds >= inSeconds) {
							diffSeconds = outSeconds - inSeconds;
						}
						else
						{
							diffSeconds = (86400 - inSeconds) + outSeconds; // Handle case where OutTime is on the next day
						}

						uint8_t hours=0, minutes=0, seconds=0;
						convertTime(diffSeconds, &hours, &minutes, &seconds);
						snprintf(id_buffer_duration, sizeof(id_buffer_duration),"%d hr,%d min,%d sec", hours, minutes, seconds);
						SH1106_GotoXY(2, 45);
						SH1106_Puts(id_buffer_duration, &Font_7x10, 1);

						SH1106_UpdateScreen();
						HAL_Delay(2000);

						cardDatabase[index].isTappedIn = false;

						SPI_Deselect_All();
						HAL_Delay(10);

						char OUT_Time_buffer[100], OUT_Date_buffer[100];
						snprintf(OUT_Time_buffer, sizeof(OUT_Time_buffer), "%02d:%02d:%02d", cardDatabase[index].OutTime.Hours, cardDatabase[index].OutTime.Minutes, cardDatabase[index].OutTime.Seconds);
						snprintf(OUT_Date_buffer, sizeof(OUT_Date_buffer), "%02d/%02d/%04d", cardDatabase[index].OutDate.Date, cardDatabase[index].OutDate.Month, (cardDatabase[index].OutDate.Year+2000));

						snprintf(LoRa_Message, sizeof(LoRa_Message), "%d,%s,%s,%s,%02X %02X %02X %02X,OUT,", LoRa_DeviceID_3, OUT_Date_buffer, OUT_Time_buffer, cardDatabase[index].owner, cardDatabase[index].uid_bytes[0], cardDatabase[index].uid_bytes[1], cardDatabase[index].uid_bytes[2], cardDatabase[index].uid_bytes[3]);
						if (LoRa_transmit(&myLoRa, (uint8_t*)LoRa_Message, strlen(LoRa_Message), 5000))
						{
							printf("Message sent to Gateway\r\n");
						}
						else
						{
							printf("LoRa TX failed\r\n");
						}

						HAL_Delay(5000);

						snprintf(LoRa_Message, sizeof(LoRa_Message), "%d,%s,%s,%s,%02X %02X %02X %02X,OUT,", LoRa_DeviceID_3, OUT_Date_buffer, OUT_Time_buffer, cardDatabase[index].owner, cardDatabase[index].uid_bytes[0], cardDatabase[index].uid_bytes[1], cardDatabase[index].uid_bytes[2], cardDatabase[index].uid_bytes[3]);
						if (LoRa_transmit(&myLoRa, (uint8_t*)LoRa_Message, strlen(LoRa_Message), 5000))
						{
							printf("Message sent to Gateway\r\n");
						}
						else
						{
							printf("LoRa TX failed\r\n");
						}

						SPI_Deselect_All();
						HAL_Delay(10);
					}
				}
				else {
				  SH1106_Clear();
				  SH1106_GotoXY(5, 35);
				  SH1106_Puts("Access Denied", &Font_7x10, 1);
				  SH1106_UpdateScreen();
				  HAL_Delay(2000);
				}
			}
			else
			{
				// Card not stored in Database
				SH1106_Clear();
				SH1106_GotoXY(5, 15);
				SH1106_Puts("Access Denied", &Font_7x10, 1);
				SH1106_UpdateScreen();
				HAL_Delay(2000);
			}
		}
		else
		{
			USER_LOG("UID read failed\r\n");
		}
		waitcardRemoval(&rfID);
	}

	/*if (HAL_GPIO_ReadPin(GPIOA, Push_Button_Pin) == GPIO_PIN_RESET) { // Button is pressed
		//HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_RESET); // Turn on LED
		//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		printf("\nButton Pressed\r\n");
		HAL_Delay(1000);
	}*/

	// ================= TX BLOCK =================
	SEND_REQUEST:
	{
		if (!request_sent)
		{
			  printf("\nSending time request...\r\n");

			  if (LoRa_transmit(&myLoRa, (uint8_t*)send_req, strlen(send_req), 5000))
			  {
				  request_sent = 1;
				  printf("Time request sent\r\n");
				  lora_irq_flag = true;
			  }
			  else
			  {
				  printf("LoRa TX failed\r\n");
				  request_sent = 0;
			  }
		}
		else
		{
			if(lora_irq_flag)
			{
			  lora_irq_flag = false;
			  printf("\nRX started\r\n");
			  int len = LoRa_receive(&myLoRa, (uint8_t*)rx_buffer, sizeof(rx_buffer), 4000);
			  if (len <= 0)
			  {
				  request_sent = 0;
				  goto SEND_REQUEST;
			  }

			  rx_buffer[len] = '\0';
			  printf("RX Data: %s\r\n", rx_buffer);

			  uint32_t epoch;
			  if (Parse_Epoch(rx_buffer, &epoch))
			  {
				  printf("Valid epoch: %lu\r\n", epoch);

				  uint32_t rx_epochdate = (uint32_t)epoch;
				  if ( is_valid_epoch((time_t)rx_epochdate) ) {
					  printf("Valid epoch time\r\n");
					  rssi = LoRa_getRSSI(&myLoRa);
					  printf("RSSI: %d\r\n", rssi);
					  updateRTC_fromEpoch((uint32_t)rx_epochdate);
					  ReceiveFirstTimeFlag = true;
					  ReceiveSixHourTimeReqFlag = true;
				  }
				  else {
					  printf("Invalid epoch time\r\n");
					  request_sent = 0;
				  }
			  }
			  else
			  {
				  printf("Invalid time packet\r\n");
				  request_sent = 0;
			  }
			}
			HAL_Delay(20);
		}
	}

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 1;
  DateToUpdate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RC522_NSS_Pin|LoRa_CS_Pin|LoRa_Reset_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RC522_Reset_GPIO_Port, RC522_Reset_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Push_Button_Pin */
  GPIO_InitStruct.Pin = Push_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Push_Button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RC522_NSS_Pin LoRa_CS_Pin LoRa_Reset_Pin */
  GPIO_InitStruct.Pin = RC522_NSS_Pin|LoRa_CS_Pin|LoRa_Reset_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : RC522_Reset_Pin */
  GPIO_InitStruct.Pin = RC522_Reset_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RC522_Reset_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LoRa_DIO0_Pin */
  GPIO_InitStruct.Pin = LoRa_DIO0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LoRa_DIO0_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
    	total_seconds++; // Signal the main loop that 1 second has passed
    	tot_sec++;
    	if(tot_sec >= 60) {
    		tot_sec = 0;
    		total_min++;
    		if(total_min >= 60) {
    			total_min = 0;
    			total_hour++;
    			if(total_hour >= 24) {
    				total_hour = 0;
    			}
    		}
    	}
    }
}

void updateRTC_fromEpoch(uint32_t epochTime_val) {
	time_t TimeNow = (time_t)epochTime_val;
	char buf[100];

    struct tm time_tm;
    time_tm = *(localtime(&TimeNow));

	strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S", &time_tm);
	printf("Received time: %s\n", buf);

    // Populate HAL Time structure
    sTime.Hours = (uint8_t)time_tm.tm_hour;
    sTime.Minutes = (uint8_t)time_tm.tm_min;
    sTime.Seconds = (uint8_t)time_tm.tm_sec;


    // Populate HAL Date structure
    sDate.Month = (uint8_t)time_tm.tm_mon + 1; // tm_mon is 0-11, HAL is 1-12
    sDate.Date = (uint8_t)time_tm.tm_mday;

    // tm_year is years since 1900, HAL is years since 2000
    //year + 1900 - 2000 = year - 100
    sDate.Year = (uint8_t)(time_tm.tm_year - 100);

    //sDate.WeekDay = (uint8_t)time_tm.tm_wday + 1;
    // Handle WeekDay if necessary (tm_wday is 0-6, HAL might differ)
    // tm_wday is 0-6 (Sun-Sat), RTC needs 1-7 (Mon-Sun or specific definitions)
    // Need to adjust 0 (Sunday) to 7 if using HAL_WEEKDAY_MONDAY=1
    sDate.WeekDay = (time_tm.tm_wday == 0) ? 7 : (uint8_t)time_tm.tm_wday;

    // Set the RTC
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void addNewCardLogic()
{
	if (HAL_GPIO_ReadPin(GPIOA, Push_Button_Pin) == GPIO_PIN_RESET) { // Button is pressed
		HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_RESET); // Turn on LED
		SH1106_Clear();
		SH1106_GotoXY(5, 5);
		SH1106_Puts("Tap New Card ", &Font_7x10, 1);
		SH1106_GotoXY(5, 15);
		SH1106_Puts(" to Add ", &Font_7x10, 1);
		SH1106_UpdateScreen();
		HAL_Delay(5000);

		while(1)
		{
			if (waitcardDetect(&rfID) == STATUS_OK){
				HAL_Delay(20);   // give clone chip time
				if (MFRC522_ReadUid(&rfID, uid) == STATUS_OK){
					USER_LOG("CARD ID:%02X %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3], uid[4]);

					int index1 = -1;
					for(int i=0; i<25; i++) {
					  if(isSameUID(cardDatabase[i].uid_bytes, uid)) {
						  index1 = i;
						  break;
					  }
					  else
					  {
						  index1 = -1;
					  }
					}

					if(index1 == -1) {
					  USER_LOG("1) Card does not Exist in Database");
					  displayDatabase();
					  for(int i=0; i<25; i++) {
						  if ((cardDatabase[i].uid_bytes[0] == 0)) { //(cardDatabase[i].owner[0] == '\0') &&
							  memcpy(cardDatabase[i].uid_bytes, uid, 5);
							  cardDatabase[i].isTappedIn = false;
							  USER_LOG("Enter Name for New added Card");

							  #define MAX_NAME_LEN 50
							  uint8_t Name[MAX_NAME_LEN];
							  uint8_t ch;
							  uint16_t j = 0;
							  while(1)
							  {
								  SH1106_Clear();
								  SH1106_GotoXY(5, 5);
								  SH1106_Puts("Enter Name for", &Font_7x10, 1);
								  SH1106_GotoXY(5, 15);
								  SH1106_Puts("  New Card", &Font_7x10, 1);
								  SH1106_UpdateScreen();
								  HAL_Delay(1000);

								  //SH1106_Clear();
								  HAL_StatusTypeDef UARTstatus = HAL_UART_Receive(&huart1, &ch, 1, HAL_MAX_DELAY);
								  if(UARTstatus == HAL_OK)
								  {
									  if (ch == '\b' && j > 0)
									  {
									      j--;
									  }
									  else if (ch != '\r' && ch != '\n')
									  {
										  if (j < MAX_NAME_LEN - 1) // Store only if space available
										  {
											  Name[j++] = ch;
										  }
									  }
									  if (ch == '\n' || ch == '\r') // Stop when Enter is pressed
										  break;
								  }
								  else if (UARTstatus == HAL_TIMEOUT)
								  {
									  SH1106_Clear();
									  SH1106_GotoXY(5, 15);
									  SH1106_Puts("Timeout! Try Again", &Font_7x10, 1);
									  SH1106_UpdateScreen();
									  HAL_Delay(1000);
								  }
							  }
							  // Null terminate string
							  Name[j] = '\0';
							  USER_LOG("Received Name: %s",Name);
							  memcpy(cardDatabase[i].owner, Name, j);

							  SH1106_Clear();
							  SH1106_GotoXY(5, 5);
							  SH1106_Puts("New Card Added", &Font_7x10, 1);
							  snprintf(id_buffer,sizeof(id_buffer),"** ** ** %02X",uid[3]);
							  SH1106_GotoXY(5, 25);
							  SH1106_Puts(id_buffer, &Font_7x10, 1);
							  SH1106_UpdateScreen();
							  HAL_Delay(3000);
							  SH1106_Clear();

							  USER_LOG("1) New Card Added in Database");
							  displayDatabase();

							  //HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_SET); // Turn on LED
							  break;
						  }
					  }
					  break;
					}
					else
					{
					  USER_LOG("1) Card Exists in Database");
					  USER_LOG("Index Number:%d", index1);
					  SH1106_Clear();
					  SH1106_GotoXY(5, 5);
					  SH1106_Puts("Old Card Exists", &Font_7x10, 1);
					  SH1106_GotoXY(5, 25);
					  SH1106_Puts("Unable to Add", &Font_7x10, 1);
					  SH1106_UpdateScreen();
					  HAL_Delay(3000);
					  SH1106_Clear();
					  break;
					}
				}
				else
				{
					USER_LOG("New Card Read Failed!!!");
					SH1106_Clear();
					SH1106_GotoXY(5, 5);
					SH1106_Puts(" New Card", &Font_7x10, 1);
					SH1106_GotoXY(5, 15);
					SH1106_Puts("Read Failed!!!", &Font_7x10, 1);
					SH1106_GotoXY(5, 25);
					SH1106_Puts("Try Again", &Font_7x10, 1);
					SH1106_UpdateScreen();
					HAL_Delay(2000);
					SH1106_Clear();
					break;
				}
			}
		}
	}
	else {
	    // Button is not pressed
	    HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_SET); // Turn off LED
	}
}

void displayDatabase() {
    printf("Idx | UID (Hex)            | Owner                | Status\n");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < 50; i++) {
        // Accessing the current card
        RFID_Card *c = &cardDatabase[i];

        // Print Index
        printf("%02d  | ", i);

        // Print UID bytes in Hex (5 bytes)
        for (int j = 0; j < 5; j++) {
            printf("%02X ", c->uid_bytes[j]);
        }
        printf("| ");

        // Print Owner Name (padded to 20 chars) and Tapped Status
        printf("%-20s | %s\n",
               c->owner,
               c->isTappedIn ? "Tapped In" : "Tapped Out");
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if (GPIO_Pin == myLoRa.DIO0_pin) {
		//lora_irq_flag = true;
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
