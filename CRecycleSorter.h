#ifndef RECYCLESORTER_H
#define RECYCLESORTER_H

#include "CBase4618.h"
#include "server.h"
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

#define SERVO_GATE 17
#define SERVO_1 18
#define SERVO_2 27
#define SERVO_3 22
#define SERVO_TOP1 26
#define SERVO_TOP2 19

#define GBUTTON 21
#define PBUTTON 20
#define YBUTTON 16
#define BBUTTON 12

#define SERVO_LEFT 2500
#define SERVO_MIDDLE 1385
#define SERVO_RIGHT 500
#define SERVO_QUARTER 500

#define ADC_VALUE 1024

class CRecycleSorter: public CBase4618
{
	private:
	int _HighHue, _LowHue;
	int _HighSat, _LowSat;
	int _HighVal, _LowVal;
	int _drawCount;
	
	int _BlueCount, _GreenCount, _PinkCount, _YellowCount, _SearchCount;
	
	cv::Scalar lowPink, upPink, lowYellow, upYellow;
	cv::Scalar lowGreen, lowBlue, upBlue;
	cv::Scalar upGreen;
	std::string _currentColour;
	std::string _prevColour;
	
	std::vector<std::string> ColourName;
	cv::Mat _control;
	cv::VideoCapture cam;
	
	std::vector<std::vector<cv::Point>> yellowContour, pinkContour, blueContour, greenContour;
	
	bool _readyToChange;
	bool _active;
	bool _do_exit;		///< Exit flag for the program
	bool _sortGRN, _sortPNK, _sortYLW, _sortBLU;
	
	public:
	CServer server;
	
	CRecycleSorter();
	~CRecycleSorter();
	
	void update() override;
	void draw() override;
	void run() override;
	void ControlPanel();
	void ControlPanelCal();
	void results();
	int autoSort();
	void sort1();
	void sort2();
	void sort3();
	void sort4();
	void RGB_OFF();
	void releaseBall();
	void reloadBall();
	void tripwire();
	void agitate();
	void gate();
	
	void serverCommands();
	
	
	double drawBox(cv::Mat& frame, const std::vector<std::vector<cv::Point>>& box, const std::string& label, const cv::Scalar& colour); 
	//Dilate and Erode image
	void DandE(cv::Mat& mask);
	void sortByArea(const double& pink, const double& green, const double& blue, const double& yellow);

	void calibrate();
	
	static void Control_thread(CRecycleSorter* ptr);
	static void tripwire_thread(CRecycleSorter* ptr);
	static void update_thread(CRecycleSorter* ptr);
};

#endif /* RECYCLESORTER_H */
