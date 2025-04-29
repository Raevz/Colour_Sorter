// PiControl.cpp : Defines the entry point for the application.
//

#include "PiControl.h"

PiControl::PiControl(){}
PiControl::~PiControl(){}

void PiControl::print_menu()
{
	std::cout << "\n**************************************************";
	std::cout << "\n***** ELEX4618 Lab8: Recycling by Ryan McKay *****";
	std::cout << "\n**************************************************";
	std::cout << "\nTHIS------> IS THE MENU";
	std::cout << "\nEnter Command";
	std::cout << "\n(A)nalog \n(B)utton LED Control \n(D)Counter \n(S)ervo Test \n(R)ecycler \n(C)alibrate Hues \n(Q)uit";
	std::cout << "\nCMD> ";
}

void PiControl::gpio_setup()
{
	gpioSetMode(BOOSTER_BUTTON_1, PI_INPUT);
	gpioSetMode(BOOSTER_BUTTON_2, PI_INPUT);
	gpioSetMode(GBUTTON, PI_INPUT);
	gpioSetMode(PBUTTON, PI_INPUT);
	gpioSetMode(YBUTTON, PI_INPUT);
	gpioSetMode(BBUTTON, PI_INPUT);
	
	gpioSetPullUpDown(GBUTTON, PI_PUD_UP);
	gpioSetPullUpDown(PBUTTON, PI_PUD_UP);
	gpioSetPullUpDown(YBUTTON, PI_PUD_UP);
	gpioSetPullUpDown(BBUTTON, PI_PUD_UP);
	
	gpioSetMode(SERVO_GATE, PI_OUTPUT);
	gpioSetMode(SERVO_1, PI_OUTPUT);
	gpioSetMode(SERVO_2, PI_OUTPUT);
	gpioSetMode(SERVO_3, PI_OUTPUT);
	gpioSetMode(SERVO_TOP1, PI_OUTPUT);
	gpioSetMode(SERVO_TOP2, PI_OUTPUT);
	
	gpioSetMode(ACTIVE_LED, PI_OUTPUT);
	gpioSetMode(RED_RGB, PI_OUTPUT);
	gpioSetMode(GREEN_RGB, PI_OUTPUT);
	gpioSetMode(BLUE_RGB, PI_OUTPUT);

	gpioWrite(ACTIVE_LED, 0);
	gpioWrite(RED_RGB, 0);
	gpioWrite(GREEN_RGB, 0);
	gpioWrite(BLUE_RGB, 0);
	
	gpioWrite(SERVO_GATE, 0);
	gpioWrite(SERVO_1, 0);
	gpioWrite(SERVO_2, 0);
	gpioWrite(SERVO_3, 0);
	gpioWrite(SERVO_TOP1, 0);
	gpioWrite(SERVO_TOP2, 0);
}

bool PiControl::get_button(int Button)
{
	if (gpioRead(Button) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
///////////////////////////CHANGED TO MEASURE TRIPWIRE/////////////////////////////////////////////
void PiControl::button_count()
{
	//auto timer_start; 
	auto timer_start = std::chrono::system_clock::now();
	int count = 0;
	while (std::chrono::system_clock::now() - timer_start < std::chrono::seconds(10))
	{
		std::cout << "\nButton Presses: " << count;

		if (get_button(BOOSTER_BUTTON_1))
		{
			++count;
			auto punishment_timer = std::chrono::system_clock::now();
			do
			{
				std::cout << "\nButton Presses: " << count;
				std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(10));

			} while (std::chrono::system_clock::now() - punishment_timer < std::chrono::milliseconds(500));
		}
		std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(10));
	}
}

void PiControl::button_LED()
{
	bool state;
	gpioSetMode(ACTIVE_LED, PI_OUTPUT);

	auto timer_start = std::chrono::system_clock::now();
	while (std::chrono::system_clock::now() - timer_start < std::chrono::seconds(10))
	{
		state = get_button(BOOSTER_BUTTON_1);

		std::cout << "\nButton State: " << state;

		gpioWrite(ACTIVE_LED, state);
	}
	gpioWrite(ACTIVE_LED, 0);
}

void PiControl::enable_LED(int light)
{
	gpioWrite(light, 1);
}

void PiControl::disable_LED(int light)
{
	gpioWrite(light, 0);
}

void PiControl::servo_TEST()
{
	gpioServo(SERVO_GATE, SERVO_MIDDLE + SERVO_QUARTER);
	std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));

	gpioServo(SERVO_GATE, SERVO_MIDDLE - SERVO_QUARTER);
	std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));

	gpioServo(SERVO_GATE, SERVO_LEFT);
	std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));

	gpioServo(SERVO_GATE, SERVO_RIGHT);
	std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));

	gpioServo(SERVO_GATE, SERVO_MIDDLE);
}

cv::Point2f PiControl::get_analog()
{
	int xResult = 0;
	float xPercent = 0;
	int yResult = 0;
	float yPercent = 0;
	char channel_0 = 0;
	char channel_2 = 2;
	
	xResult = read_mcp3008_channel(channel_0);
	xPercent = xResult * 100 / ADC_VALUE;
	
	yResult = read_mcp3008_channel(channel_2);
	yPercent = yResult * 100 / ADC_VALUE;
	//std::cout << xResult << " " << yResult;
	cv::Point2f JoystickPosition = cv::Point2f(xPercent, yPercent);
	
	return JoystickPosition;
}
int PiControl::read_mcp3008_channel(const char& channel) 
{
	int read_val;
	unsigned char inBuf[3];
	char cmd[] = { 1, (8 + channel) << 4, 0 }; // 0b1XXX0000 where XXX=channel 0
	int handle = spiOpen(0, 200000, 3); // Mode 0, 200kHz
	spiXfer(handle, cmd, (char*) inBuf, 3); // Transfer 3 bytes
	read_val = ((inBuf[1] & 3) << 8) | inBuf[2]; // Format 10 bits
	//std::cout << read_val;
	spiClose(handle); // Close SPI system
	return read_val;
}


void PiControl::display_analog()
{
	int xRaw = 0;
	int yRaw = 0;
	
	auto timer_start = std::chrono::system_clock::now();
	while (std::chrono::system_clock::now() - timer_start < std::chrono::seconds(10))
	{
		cv::Point2f JoystickPosition = get_analog();

		xRaw = (JoystickPosition.x * ADC_VALUE) / 100;
		yRaw = (JoystickPosition.y * ADC_VALUE) / 100;

		std::cout << "\nX: " << xRaw << " (" << JoystickPosition.x << ")%";
		std::cout << "\t\tY: " << yRaw << " (" << JoystickPosition.y << ")%";
	}
}

//cv::Point3f CControl::get_accelerometer()
//{
//	int xResult, yResult, zResult;
//
//	get_data(Analog, ACCELEROMETER_X_CHAN, xResult);
//	get_data(Analog, ACCELEROMETER_Y_CHAN, yResult);
//	get_data(Analog, ACCELEROMETER_Z_CHAN, zResult);
//
//
//	cv::Point3f accelerometer = cv::Point3f(xResult, yResult, zResult);
//
//	return accelerometer;
//}

