#include "main.h"
#include "button.h"
#include "i2c_lcd.h"
#include "stm32f4xx_hal.h"

#include <string.h>
#include <stdio.h>
//함수
void washing(void);
void make_trigger();
extern void delay_us(unsigned long us);
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart3;
extern void buzzer_main();
void set_time_button_w(void);
void lcd_display_mode_select_w(void);

//변수

extern uint8_t lcd_display_mode_flag_w;
extern char lcd_buff[40];
extern TIM_HandleTypeDef htim2;
extern int get_button(GPIO_TypeDef *GPIO, uint16_t GPIO_PIN, uint8_t button_number);
void ultrasonic_processing_w(void);
extern volatile int TIM10_10ms_ultrasonic;

extern void lcd_string(uint8_t *str);
extern void move_cursor(uint8_t row, uint8_t column);

int dis_w;// 거리를 측정한 펄스 개수를 저장하는 변수
volatile int distance_w;  // 거리를 측정한 펄스 개수를 저장하는 변수
volatile int ic_cpt_finish_flag_w = 0; // 초음파 거리 측정 완료 indicator 변수
// 1. Drivers/STM32F4xx_HALL_Driver/Src/stm32f4xx_hal_tim.c에 가서
// HAL_TIM_IC_CaptureCallback을 잘라내기를 해서 이곳으로 가져 온다.
// 2. 초음파 센서의 ECHO핀의 상승 에지와 하강 에지 발생 시 이곳으로 들어온다.
volatile uint8_t is_first_capture_w = 0; // 0: 상승 에지 1: 하강 에지




////////////////////////////////////////////////////////////////////////////////
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

	if (htim->Instance == TIM3)
	{
		if (is_first_capture_w == 0)   // 상승에지
		{
			__HAL_TIM_SET_COUNTER(htim, 0); // clear H/W counter
			is_first_capture_w = 1;	// 상승 에지를 만났다는 flag변수 indicator를 set
		}
		else if (is_first_capture_w == 1)	// 하강 에지를 만나면
		{
			is_first_capture_w = 0;  // 다음 echo 펄스를 count 하기 위해 변수 초기화
			distance_w = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);  // 현재까지 count한 펄스 수를 읽어 온다.
			ic_cpt_finish_flag_w = 1; // 초음파 측정 완료
		}
	}
}
void ultrasonic_processing_w(void)
{
	//char lcd_buff[20];  // lcd 문자를 저장할 버퍼 추가

	if (TIM10_10ms_ultrasonic >= 100)    // 1초
	{
		TIM10_10ms_ultrasonic = 0;
		make_trigger();
		if (ic_cpt_finish_flag_w)   // 초음파 측정이 완료되었으면 - if (ic_cpt_finish_flag_w) >= 1
		{
			ic_cpt_finish_flag_w = 0;
			dis_w = distance_w;
			dis_w = dis_w * 0.034 / 2; // 1us가 0.034cm를 이동
			// 왕복 거리를 리턴해 주기 때문에 /2를 해주는 것이다.
			//	printf("dis: %dcm\n", dis_w);

			// sprintf(lcd_buff, "dis: %03dcm", dis_w);  // sprintf - atoi 안 해도 된다?
			// move_cursor(0,0);    // 0번째 라인에 0번째 커서를 가리킨다.
			// lcd_string(lcd_buff);

		}
	}
}
void make_trigger()
{
	HAL_GPIO_WritePin(ULTRASONIC_TRIGGER_GPIO_Port, ULTRASONIC_TRIGGER_Pin, 0);  // 일단 0부터 시작
	delay_us(2);
	HAL_GPIO_WritePin(ULTRASONIC_TRIGGER_GPIO_Port, ULTRASONIC_TRIGGER_Pin, 1);
	delay_us(10);
	HAL_GPIO_WritePin(ULTRASONIC_TRIGGER_GPIO_Port, ULTRASONIC_TRIGGER_Pin, 0);
}
/////////////////////////////////////////////////////////////////////////////////////

//void washing(void)
//{
	//	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8 - 빨래
	//	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

//	if (dis_w <= 8) // 거리가 4cm 이하면 열리고 닫힌다.
//	{
//		// 180도 회전
//		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 100);
//		HAL_Delay(2000);
//		// 90도
//		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 60);
//		HAL_Delay(2000);
//		// 0도
//		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 5);
//		HAL_Delay(2000);

		//			if (get_button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
		//			{
		//				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 0); // 역회전  E8 - 탈수
		//				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1); // E9
		//			}
//	}
//}


//void set_time_button_w(void)
//{
//
//
//	if (get_button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
//	{
//		move_cursor(0,0);
//		strcpy(lcd_buff,"You want to use");
//		lcd_string(lcd_buff);
//
//		move_cursor(1,0);
//		strcpy(lcd_buff,"        me, ha?");
//		lcd_string(lcd_buff);
//		HAL_Delay(3000);
//
//		move_cursor(0,0);
//		strcpy(lcd_buff,"then, go to the");
//		lcd_string(lcd_buff);
//		move_cursor(1,0);
//		strcpy(lcd_buff,"next Button Go!");
//		lcd_string(lcd_buff);
//	}
//	if (get_button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
//	{
//		move_cursor(0,0);  // 헹굼
//		strcpy(lcd_buff, "      rinse    ");
//		lcd_string(lcd_buff);
//		move_cursor(1,0);
//		strcpy(lcd_buff, "               ");
//		lcd_string(lcd_buff);
//
//		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
//		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9
//
//
//	}
//	if (get_button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
//	{
//		move_cursor(0,0);  // 탈수
//		lcd_string("    spin-dry  ");
//		move_cursor(1,0);
//		lcd_string("               ");
//
//		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 0); // 역회전  E8
//		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1); // E9
//	}
//	if (get_button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
//	{
//		move_cursor(0,0);  // 건조 - 노래
//		lcd_string("    all-dry  ");
//		move_cursor(1,0);
//		lcd_string("               ");
//
//		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
//		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
//
//		buzzer_main();
//
//
//	}
//}


int run_plag = 0; // 0번 버튼을 눌렀을 떄 동작.
int run_plag_end = 0;

int wash_time = 0; // control times
int rinse_time = 0;
int dewa_time = 0;
int dry_time = 0;

int flag_wash = 0;
int flag_wash_end = 0;

int flag_auto = 0;  // 표준의 자동화 플래그
int flag_auto2 = 0;

int flag_4 = 0;
int flag_end = 0;
int flag_end2 = 0;
int flag_end3 = 0;  // control 끝나는 end

int min_w = 0;

int init = 0; // 첫 화면 출력 플래그.





////////// 물 업 다운 함수

//void water_up()
//{
//		if (t1ms_counter_w >= 500)     //초기화, 계속 증가로 인한 반복 기능. 200ms가 될 때까지 아무것도 안 하는데, 그게 딜레이를 대신한다.
//		{
//			t1ms_counter_w = 0;
//
//			HAL_GPIO_WritePin(GPIOD, 0x01 << water_i, 1);
//			water_i++;
//		}
//}
//void water_down()
//{
//		if (t1ms_counter_w >= 100)     //초기화, 계속 증가로 인한 반복 기능. 200ms가 될 때까지 아무것도 안 하는데, 그게 딜레이를 대신한다.
//		{
//			t1ms_counter_w = 0;
//
//			HAL_GPIO_WritePin(GPIOD, 0x80 >> 8 - water_i, 0);
//			water_i--;
//		}
//}
///////////


void lcd_display_mode_select_w(void) // 메인
{

	// 동등한 if문을 넣는다. 같이 실행된다. 그리고 하나가 빨리 끝나면 다른 것도 끝난다;;;

	if (flag_end == 0)
	{
		if(!lcd_display_mode_flag_w && init == 0)
		{
			if (dis_w <= 8) // 거리가 8cm 이하면 열리고 닫힌다.
			{
				// 90도
				__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 60);
				//HAL_Delay(2000);

			}
			else
			{ // 180도 회전
				__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 100);
				//HAL_Delay(2000);
			}

			move_cursor(0,0);
			lcd_string(" mode select?   ");
			move_cursor(1,0);
			lcd_string(" press Button4! ");
		}
		else
		{//버튼 4를 누른다.
			if(lcd_display_mode_flag_w == 1) // 모드 1일 때 standard
			{
				move_cursor(0,0);
				lcd_string(" +standard");
				move_cursor(1,0);
				lcd_string("  control");
				// 여기서 버튼 3을 눌러서 선택한다.
				if(flag_auto == 1)
				{
					HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
					HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

					for (min_w = 5; min_w >= 0; min_w--)
					{
						sprintf(lcd_buff, "TIME: %02d      ", min_w);
						move_cursor(0,0);
						lcd_string(lcd_buff);
						move_cursor(1,0);
						lcd_string("           ");
						HAL_Delay(1000);

						if (min_w == 0)
						{
							HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
							HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
							flag_end = 1;

						}
					}
				}

			}

			if(lcd_display_mode_flag_w == 0) // 모드 0일 때 통과 플래그 1개
			{
				if(flag_end2 == 0)  // 자동 while 문이니까 되돌아오지 못하게 해야 한다. 플래그 또 1개
				{
					move_cursor(0,0);
					lcd_string("  standard");
					move_cursor(1,0);
					lcd_string(" +control");
				}
				// 4번 버튼으로 고른다.

				// 여기서 버튼 3을 눌러서 선택한다.

				if(flag_auto2 == 1)
				{
					if (flag_4 == 0)  // 밑에 3개도 추가했음.
					{// 세탁, 버튼2로 고른다. 여기서 버튼1을 누르면 wash의 시간이 증가한다.
						if (run_plag_end == 0)                               // 여기 집합이랑
						{
						move_cursor(0,0);
						lcd_string(" +wash  rinse");
						move_cursor(1,0);
						lcd_string("  dewa  dry  ");
						}

						//여기서부터 복사문들.

						if(run_plag == 1) // 시간 누적은 1번 버튼. 0번 버튼을 눌렀을 떄 가동
						{
							if(run_plag_end == 1) // zero_plag_end == 0일 때는 위로 안 간다. 쌍방 플래그.    //이거 집합이랑 다른 집에 있어야 한다. 다 바꾼다.
							{

								// wash_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								//for(wash_time; wash_time >= 0 ; wash_time--)
								while(wash_time >= 0)
								{
									sprintf(lcd_buff, "wash: %02d      ", wash_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);
									wash_time--;

									if (wash_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// rinse_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(rinse_time; rinse_time >= 0 ; rinse_time--)
								{
									sprintf(lcd_buff, "rinse: %02d      ", rinse_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (rinse_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// dewa_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(dewa_time; dewa_time >= 0 ; dewa_time--)
								{
									sprintf(lcd_buff, "dewa: %02d      ", dewa_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (dewa_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// dry_time만큼 실행하라. 그리고 끝내라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(dry_time; dry_time >= 0 ; dry_time--)
								{
									sprintf(lcd_buff, "dry: %02d      ", dry_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (dry_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								flag_end3 = 1;
								if (flag_end3 == 1)
								{
									move_cursor(0,0);
									lcd_string("  THE END  ");
									move_cursor(1,0);
									lcd_string("           ");
									buzzer_main(); // 노래 나옴
								}

							}
						}//
					}
					else if (flag_4 == 1)
					{// 헹굼, 버튼2로 고른다. 여기서 버튼1을 누르면 rinse의 시간이 증가한다.
						if(run_plag_end == 0)
						{
							move_cursor(0,0);
							lcd_string("  wash +rinse");
							move_cursor(1,0);
							lcd_string("  dewa  dry  ");
						}

						if(run_plag == 1) // 0번 버튼을 눌렀을 떄 가동
						{
							if(run_plag_end == 1) // zero_plag_end == 0일 때는 위로 안 간다. 쌍방 플래그.
							{
								// wash_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(wash_time; wash_time >= 0 ; wash_time--)
								{
									sprintf(lcd_buff, "wash: %02d      ", wash_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (wash_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// rinse_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(rinse_time; rinse_time >= 0 ; rinse_time--)
								{
									sprintf(lcd_buff, "rinse: %02d      ", rinse_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (rinse_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// dewa_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(dewa_time; dewa_time >= 0 ; dewa_time--)
								{
									sprintf(lcd_buff, "dewa: %02d      ", dewa_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (dewa_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// dry_time만큼 실행하라. 그리고 끝내라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(dry_time; dry_time >= 0 ; dry_time--)
								{
									sprintf(lcd_buff, "dry: %02d      ", dry_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (dry_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								flag_end3 = 1;
								if (flag_end3 == 1)
								{
									move_cursor(0,0);
									lcd_string("  THE END  ");
									move_cursor(1,0);
									lcd_string("           ");
									buzzer_main(); // 노래 나옴
								}

							}
						}//
					}
					else if (flag_4 == 2)
					{// 탈수, 버튼2로 고른다. 여기서 버튼1을 누르면 dewa의 시간이 증가한다.
						if(run_plag_end == 0)
						{
							move_cursor(0,0);
							lcd_string("  wash  rinse");
							move_cursor(1,0);
							lcd_string(" +dewa  dry  ");
						}

						if(run_plag == 1) // 0번 버튼을 눌렀을 떄 가동
						{
							if(run_plag_end == 1) // zero_plag_end == 0일 때는 위로 안 간다. 쌍방 플래그.
							{
								// wash_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(wash_time; wash_time >= 0 ; wash_time--)
								{
									sprintf(lcd_buff, "wash: %02d      ", wash_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (wash_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// rinse_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(rinse_time; rinse_time >= 0 ; rinse_time--)
								{
									sprintf(lcd_buff, "rinse: %02d      ", rinse_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (rinse_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// dewa_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(dewa_time; dewa_time >= 0 ; dewa_time--)
								{
									sprintf(lcd_buff, "dewa: %02d      ", dewa_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (dewa_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// dry_time만큼 실행하라. 그리고 끝내라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(dry_time; dry_time >= 0 ; dry_time--)
								{
									sprintf(lcd_buff, "dry: %02d      ", dry_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (dry_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								flag_end3 = 1;
								if (flag_end3 == 1)
								{
									move_cursor(0,0);
									lcd_string("  THE END  ");
									move_cursor(1,0);
									lcd_string("           ");
									buzzer_main(); // 노래 나옴
								}

							}
						}//
					}
					else if (flag_4 == 3)
					{// 건조, 버튼2로 고른다. 여기서 버튼1을 누르면 dry의 시간이 증가한다.
						if(run_plag_end == 0)
						{
							move_cursor(0,0);
							lcd_string("  wash  rinse");
							move_cursor(1,0);
							lcd_string("  dewa +dry  ");
						}

						if(run_plag == 1) // 0번 버튼을 눌렀을 떄 가동
						{
							if(run_plag_end == 1) // zero_plag_end == 0일 때는 위로 안 간다. 쌍방 플래그.
							{
								// wash_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(wash_time; wash_time >= 0 ; wash_time--)
								{
									sprintf(lcd_buff, "wash: %02d      ", wash_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (wash_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// rinse_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(rinse_time; rinse_time >= 0 ; rinse_time--)
								{
									sprintf(lcd_buff, "rinse: %02d      ", rinse_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (rinse_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// dewa_time만큼 실행하라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(dewa_time; dewa_time >= 0 ; dewa_time--)
								{
									sprintf(lcd_buff, "dewa: %02d      ", dewa_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (dewa_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								// dry_time만큼 실행하라. 그리고 끝내라.
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 정회전  E8
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0); // E9

								for(dry_time; dry_time >= 0 ; dry_time--)
								{
									sprintf(lcd_buff, "dry: %02d      ", dry_time);
									move_cursor(0,0);
									lcd_string(lcd_buff);
									move_cursor(1,0);
									lcd_string("           ");
									HAL_Delay(1000);

									if (dry_time == 0)
									{
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1); // 멈춤
										HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
									}
								}
								HAL_Delay(1000);

								flag_end3 = 1;
								if (flag_end3 == 1)
								{
									move_cursor(0,0);
									lcd_string("  THE END  ");
									move_cursor(1,0);
									lcd_string("           ");
									buzzer_main(); // 노래 나옴
								}

							}
						}
					}
				}
			}
		}
	}

	if (flag_end == 1)
	{

		move_cursor(0,0);
		lcd_string("  THE END  ");
		move_cursor(1,0);
		lcd_string("           ");
		buzzer_main(); // 노래 나옴
	}

	if(get_button(GPIOC, GPIO_PIN_13, 4) == BUTTON_PRESS)  // 버튼 4를 누르면 바뀐다.
	{
		init=1;
		lcd_display_mode_flag_w++;
		lcd_display_mode_flag_w %= 2;   // 모드는 총 2개 (표준, 조종)
		lcd_command(CLEAR_DISPLAY);
	}
	if (get_button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS) // 3번 버튼을 누르면 선택한다.
	{
		flag_auto++;
		flag_auto %= 2;
		flag_auto2++;
		flag_auto %= 2;

		flag_end2++;
		flag_end2 %= 2;

		lcd_command(CLEAR_DISPLAY);
	}
	if (get_button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS) // 2번 버튼을 누르면 4개 중 하나를 보여준다.
	{
		flag_4++;
		flag_4 %= 4;
		lcd_command(CLEAR_DISPLAY);

	}
	if (get_button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS) // 1번 버튼을 누르면 4개 중 하나를 선택한다. 시간만 증가.
	{
		if (flag_4 == 0)
		{
			wash_time++;
		}
		else if (flag_4 == 1)
		{
			rinse_time++;
		}
		else if (flag_4 == 2)
		{
			dewa_time++;
		}
		else if (flag_4 == 3)
		{
			dry_time++;
		}
	}
	if (get_button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS) // 0번 버튼을 누르면 누적 시간 전부 소비하고 종료.
	{
		run_plag++;
		run_plag_end++;
	}

}


