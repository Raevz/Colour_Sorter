// PiControl.h : Include file for standard system include files,
// or project specific include files.

//#pragma once
//#include <pigpio.h>
#ifndef PICONTROL_H
#define PICONTROL_H

#include "pigpio.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#define BOOSTER_BUTTON_1 2
#define BOOSTER_BUTTON_2 3
#define ACTIVE_LED 4
#define RED_RGB 25
#define GREEN_RGB 24
#define BLUE_RGB 23

#define GBUTTON 21
#define PBUTTON 20
#define YBUTTON 16
#define BBUTTON 12

#define SERVO_GATE 17
#define SERVO_1 18
#define SERVO_2 27
#define SERVO_3 22
#define SERVO_TOP1 26
#define SERVO_TOP2 19

#define SERVO_LEFT 2500
#define SERVO_MIDDLE 1385
#define SERVO_RIGHT 500
#define SERVO_QUARTER 500

#define ADC_VALUE 1024


class PiControl {
public:
	/**
	 * @brief Constructor for CControl class.
	 */
	PiControl();
	/**
	 * @brief Destructor for CControl class.
	 */
	~PiControl();

	cv::Point2f get_analog();
	//cv::Point3f get_accelerometer();

	
	void display_analog();	///< Show the raw and percent postions of the analog x and y
	//void display_accelerometer();
	bool get_button(int Button);		///< After debouncing the button for 1 second, display the number of button pushes
	void button_LED();
	void button_count();
	void servo_TEST();		///< Moves the servo through it's full range of motion
	//void servo_PLAY();		///< Allows the user to control the servo with the Analog Stick
	//void servo_SLOW();
	void disable_LED(int light);
	void enable_LED(int light);
	void print_menu();
	void gpio_setup();
	int spi_init();
	int read_mcp3008_channel(const char& channel);
	/** @} */ // end of Control Functions

};

#endif /* PICONTROL_H */
