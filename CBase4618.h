/********************************************************************
* @file  CBase4618.h
* @author Ryan McKay 2024
* @brief A short program to simulate an ETCH - A - SKETCH toy.
* ********************************************************************/

#pragma once

#include "PiControl.h"

/**
 * @class CBase4618
 * @brief This class is set to be inherited moving forward
 */
class CBase4618
{
protected:



	cv::Mat _canvas;	///< Matrix object that holds the canvas size


public:
	PiControl _Control;	///< Makes a connection with the CControl operations for communication with the uC
	/**
	 * @brief Purely virtual update function to be ran in inheriting classes
	 */
	virtual void update() = 0;
	
	/**
	 * @brief Purely virtual draw function to be ran in inheriting classes
	 */
	virtual void draw() = 0;
	
	/**
	 * @brief Runs the update and draw functions continuously to create an video
	 */
	virtual void run() = 0;

};
