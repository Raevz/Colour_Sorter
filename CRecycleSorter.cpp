#include "CRecycleSorter.h"

#define CVUI_IMPLEMENTATION
#include "cvui.h"

using namespace cv;

CRecycleSorter::CRecycleSorter()
{
	_LowHue = 35;
	_HighHue = 75;
	
	_LowSat = 50;
	_HighSat = 255;
	
	_LowVal = 50;
	_HighVal = 255;	
	
	_BlueCount = 0;
	_GreenCount = 0;
	_PinkCount = 0;
	_YellowCount = 0;
	_SearchCount = 0;
	
	_drawCount = 0;
	
	_currentColour = "NO BALLZ";
	
	_control = cv::Mat::zeros(cv::Size(250, 600), CV_8UC1);
	_canvas = cv::Mat::zeros(cv::Size(400, 150), CV_8UC3);
		
	lowPink = cv::Scalar(130, 230, 200);
	upPink = cv::Scalar(160, 255, 255);
    lowGreen = cv::Scalar(35, 130, 213);
    upGreen = cv::Scalar(80, 255, 255);
    lowBlue = cv::Scalar(85, 200, 240);
    upBlue = cv::Scalar(97, 255, 255);
    lowYellow = cv::Scalar(0, 25, 240);
    upYellow = cv::Scalar(40, 180, 255);
		
	_active = false;
	_do_exit = false;	
	
	cam.open(0, cv::CAP_V4L2);
	if (cam.isOpened() == false) 
    {
        std::cout << "Could not open camera" << std::endl;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, 160); // Example width
	cam.set(cv::CAP_PROP_FRAME_HEIGHT, 120); // Example height
	
	RGB_OFF();
	
	CServer server;
}

CRecycleSorter::~CRecycleSorter()
{	
	cam.release();
	cv::destroyAllWindows();
	server.stop();
}

void CRecycleSorter::run()
{
	
	cv::imshow("Control", _control);
	cvui::init("Control");	
	
	std::thread t1(&CRecycleSorter::Control_thread, this); 
	std::thread t2(&CRecycleSorter::update_thread, this);
	std::thread t3(&CServer::serverthread, &server);
	t3.detach();
	reloadBall();
 
	//Wait for exit prompt
	do
	{
		std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500)); 
		
		if(_active)
		{
			autoSort();
			gpioWrite(ACTIVE_LED, 1);
		}
		else
		{
			gpioWrite(ACTIVE_LED, 0);
		}

	} while (cv::waitKey(1) != 'q' && _do_exit == false);

	t1.join();
	t2.join();
	
	cv::destroyAllWindows();
}

void CRecycleSorter::update()
{
	cv::Mat cap, hsv, pMask, yMask, bMask, gMask;	
			
	cv::Scalar lower(_LowHue, _LowSat, _LowVal);
	cv::Scalar upper(_HighHue, _HighSat, _HighVal);
	
	cam.read(cap);
	cv::cvtColor(cap, hsv, cv::COLOR_BGR2HSV);
	
	cv::inRange(hsv, lowPink, upPink, pMask);
    cv::inRange(hsv, lowGreen, upGreen, gMask);
    cv::inRange(hsv, lowBlue, upBlue, bMask);
    cv::inRange(hsv, lowYellow, upYellow, yMask);
    
    DandE(gMask);
    DandE(yMask);
    DandE(pMask);
    DandE(bMask);
    
    yellowContour.clear();
    pinkContour.clear();
    greenContour.clear();
    blueContour.clear();
    
    cv::findContours(yMask, yellowContour, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(pMask, pinkContour, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(gMask, greenContour, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(bMask, blueContour, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);	
    
    draw();
}

void CRecycleSorter::draw()
{
	double pink, yellow, green, blue;
	cv::Mat irl, show;
	cam.read(irl);
	
	pink = drawBox(irl, pinkContour, "Pink", cv::Scalar(255, 0, 255));
    green = drawBox(irl, greenContour, "Green", cv::Scalar(0, 255, 0));
    blue = drawBox(irl, blueContour, "Blue", cv::Scalar(255, 0, 0));
    yellow = drawBox(irl, yellowContour, "Yellow", cv::Scalar(0, 255, 255));
    
    sortByArea(pink, green, blue, yellow);
    
    cv::imshow("LiveCam", irl);
    results();
    
    if(_drawCount >= 5)
    {
    show = irl;
    server.set_txim(show);
    _drawCount = 0;
	}
    _drawCount++;     
}

int CRecycleSorter::autoSort()
{
	RGB_OFF();
	_prevColour = _currentColour;
	
	if(_prevColour == "Green")
	{
		sort1();
		_GreenCount++;
	}
	else if(_prevColour == "Pink")
	{
		sort2();
		_PinkCount++;
	}
	else if(_prevColour == "Yellow")
	{
		sort3();
		_YellowCount++;
	}
	else if(_prevColour == "Blue")
	{
		sort4();
		_BlueCount++;
	}
	else 
	{
		_SearchCount++;
		reloadBall();
		
		if(_SearchCount >= 3)
		{
			_active = false;
		}
		return 0;
	}
	
	_SearchCount = 0;
	releaseBall();
	return 0;	
}

//void CRecycleSorter::tripwire()
//{
	//if(gpioRead(TRIPWIRE) == false)
	//{
		//_readyToChange = true;
	//}
//}

void CRecycleSorter::releaseBall()
{
	gpioServo(SERVO_GATE, SERVO_LEFT - 100);
	std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
	gpioServo(SERVO_GATE, SERVO_RIGHT);
	agitate();
	gpioServo(SERVO_GATE, 1150);
}

void CRecycleSorter::reloadBall()
{
	gpioServo(SERVO_GATE, SERVO_RIGHT);
	agitate();
	gpioServo(SERVO_GATE, 1150);
}

void CRecycleSorter::gate()
{
	int pos;
	int input = 0;
	gpioServo(SERVO_GATE, SERVO_RIGHT);
	pos = SERVO_RIGHT;
	std::cout << "\n" << pos; 
	
	do
	{
		std::cin >> input;
		pos += input;
		gpioServo(SERVO_GATE, pos);
		std::cout << pos << std::endl;
	}while(input != 27);
	
}

void CRecycleSorter::sortByArea(const double& pink, const double& green, const double& blue, const double& yellow)
{
	std::vector<std::tuple<double, std::string>> areas 
	{		
        {pink, "Pink"},
        {green, "Green"},
        {blue, "Blue"},
        {yellow, "Yellow"}
	};
	
	//I got some help from GPT with this as well 
	//but I will be genuinely shocked if this matches with someone else lol	
	auto largest = std::max_element(areas.begin(), areas.end(),
        [](const std::tuple<double, std::string>& a, const std::tuple<double, std::string>& b) {
            return std::get<0>(a) < std::get<0>(b);
        }); 
	if (std::get<0>(*largest) == 0)
	{
		_currentColour = "NO BALLZ";
	}	
    else if (largest != areas.end()) {
        _currentColour = std::get<1>(*largest);
    } else {
        _currentColour = "NO BALLZ";
    }
	
}

void CRecycleSorter::DandE(cv::Mat& mask)
{
	cv::erode(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	cv::dilate(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
}

//Lots of help from GPT on this function
double CRecycleSorter::drawBox(cv::Mat& frame, const std::vector<std::vector<cv::Point>>& box, const std::string& label, const cv::Scalar& colour) 
{

   double max = 0;
   double limit =30;
   std::vector<cv::Point> largestArea;
   
   for (const auto& contour : box) 
   {
      double area = cv::contourArea(contour);
      if (area > max && area >= limit) 
      {
         max = area;
         largestArea = contour;
      }
   }

   if (!largestArea.empty()) 
   {
      cv::Rect boundRect = cv::boundingRect(largestArea);
      cv::rectangle(frame, boundRect, colour, 2);
      cv::putText(frame, label, cv::Point(boundRect.x, boundRect.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.75, colour, 2);
   }
   
   if(max > 0)
   return max;
   else 
   return 0;
}

void CRecycleSorter::calibrate()
{
	cv::imshow("Control", _control);
	cvui::init("Control");		
	
	do
	{
		cv::Scalar lower(_LowHue, _LowSat, _LowVal);
		cv::Scalar upper(_HighHue, _HighSat, _HighVal);
		ControlPanelCal();
		cv::Mat hsv, cap, gMask;
		
		
		cam.read(cap);
		cv::cvtColor(cap, hsv, cv::COLOR_BGR2HSV);
		cv::inRange(hsv, lower, upper, gMask);
	    cv::erode(gMask, gMask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		cv::dilate(gMask, gMask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		
		greenContour.clear();
		cv::findContours(gMask, greenContour, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		
		drawBox(cap, greenContour, "Green", cv::Scalar(0, 255, 0));
		
		cv::imshow("Range", cap);
		cv::imshow("Mask", gMask);		
		
		
	} while (cv::waitKey(1) != 'q' && _do_exit == false);
		
	cv::destroyAllWindows();
	
}

void CRecycleSorter::results()
{
	_canvas = cv::Mat::zeros(cv::Size(400, 150), CV_8UC3);
	cv::Point gui_position = cv::Point(_canvas.size().width / 4, _canvas.size().height / 2);
	cv::putText(_canvas, _currentColour, gui_position, 0, 1, cv::Scalar(255,255,255), 2);
	cv::imshow("Results", _canvas);		
}

void CRecycleSorter::serverCommands()
{
	std::vector<std::string> cmds;
	server.get_cmd(cmds);
	
	for (const std::string& cmd : cmds)
	{
		if(!cmd.empty())
		{
			switch(cmd[0])
			{
				case 'S':
				{
					switch(cmd[2])
					{
						case '0':
						{
							switch(cmd[4])
							{
								case '0': _active = false; break;
								case '1': _active = true; break;
							}
							break;
						}
						case '1':
						{
							switch(cmd[4])
							{
								case '0': _sortGRN = true; break;
								case '1': _sortPNK = true; break;
								case '2': _sortYLW = true; break;
								case '3': _sortBLU = true; break;
							}
							break;
						}
						default: break;
					}
				break;
				}
				case 'G':
				{
					switch(cmd[2])
					{
						case '0':
						{
							if(_active)
							{
								server.send_string("ON\n");
								break;
							}
							else
							{
								server.send_string("OFF\n");
								break;
							}
							
						}
						case '1':
						{
							std::string ballCount;
							switch(cmd[4])
							{
								case '0':
								{ 
								 ballCount = std::to_string(_GreenCount) + "\n";
								 server.send_string(ballCount);	
								 break;
								}
								case '1':
								{ 
								 ballCount = std::to_string(_PinkCount)  + "\n";
								 server.send_string(ballCount);	
								 break;
								}
								case '2':
								{ 
								 ballCount = std::to_string(_YellowCount)  + "\n";
								 server.send_string(ballCount);	
								 break;
								}
								case '3':
								{  
								 ballCount = std::to_string(_BlueCount)  + "\n";
								 server.send_string(ballCount);	
								 break;
								}
							}
							break;
						}
					}
				}
			}
		}
	}
	
}

void CRecycleSorter::ControlPanel()
{
	cv::Point gui_position;	
	std::string BCNT = "Blue Balls Sorted: " + std::to_string(_BlueCount);
	std::string GCNT = "Green Balls Sorted: " + std::to_string(_GreenCount);
	std::string PCNT = "Pink Balls Sorted: " + std::to_string(_PinkCount);
	std::string YCNT = "Yellow Balls Sorted: " + std::to_string(_YellowCount);
	std::string ACTV = "ACTIVE STATUS: " + std::to_string(_active);
	
	//Control Pane 
	gui_position = cv::Point(0, 0);
	cvui::window(_control, gui_position.x, gui_position.y, 250, 600, "RECYCLE CONTROL");
	
	/////////////Active Status Indicator/////////////////
	gui_position += cv::Point(15, 40);
	cvui::text(_control, gui_position.x, gui_position.y, ACTV);
		
	////////////Ball Counts//////////////
	gui_position += cv::Point(0, 30);
	cvui::text(_control, gui_position.x, gui_position.y, "BALL COUNTS");
	gui_position += cv::Point(0, 20);
	cvui::text(_control, gui_position.x, gui_position.y, GCNT);
	gui_position += cv::Point(0, 20);
	cvui::text(_control, gui_position.x, gui_position.y, PCNT);
	gui_position += cv::Point(0, 20);
	cvui::text(_control, gui_position.x, gui_position.y, YCNT);
	gui_position += cv::Point(0, 20);
	cvui::text(_control, gui_position.x, gui_position.y, BCNT);
	
	/////////////////READ SERVER/////////////////
	serverCommands();	
	/////////////////////////////////////////////
	
	////////////Buttons////////////////
	//On button
	gui_position += cv::Point(5,30);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "ON"))
	{
		_active = true;
		_SearchCount = 0;
	}
	
	//Off button
	gui_position += cv::Point(105,0);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "OFF"))
	{
		_active = false;
	}
	
	//Exit button
	gui_position += cv::Point(-53,40);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "EXIT"))
	{
		_do_exit = true;
	}
	
	//Reset Ball Counts
	gui_position += cv::Point(0,40);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "RESET COUNT"))
	{
		_BlueCount = 0;
		_GreenCount = 0;
		_PinkCount = 0;
		_YellowCount = 0;
	}
	
	if(!_active)
	{
		RGB_OFF();
		//Green Manual Sort
		gui_position += cv::Point(-52,40);
		if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "GREEN") || gpioRead(GBUTTON) == false || _sortGRN)
		{
			sort1();
			std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
			releaseBall();
			_GreenCount++;
			_sortGRN = false;		
		}
	
		//Pink Manual Sort
		gui_position += cv::Point(105, 0);
		if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "PINK") || gpioRead(PBUTTON) == false || _sortPNK)
		{
			sort2();
			std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
			releaseBall();
			_PinkCount++;
			_sortPNK = false;
		}
	
		//Yellow Manual Sort
		gui_position += cv::Point(-105,40);
		if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "YELLOW") || gpioRead(YBUTTON) == false || _sortYLW)
		{
			sort3();
			std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
			releaseBall();
			_YellowCount++;
			_sortYLW = false;
		}
	
		//Blue Manual Sort
		gui_position += cv::Point(105, 0);
		if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "BLUE") || gpioRead(BBUTTON) == false || _sortBLU)
		{
			sort4();
			std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
			releaseBall();
			_BlueCount++;
			_sortBLU = false;
		}	
	}
	cvui::update();
	cv::imshow("Control", _control);	
}
void CRecycleSorter::ControlPanelCal()
{
	cv::Point gui_position;		
	
	//Control Pane 
	gui_position = cv::Point(0, 0);
	cvui::window(_control, gui_position.x, gui_position.y, 250, 600, "RECYCLE CONTROL");
		
	//Low Hue Trackbar
	gui_position += cv::Point(15, 40);
	cvui::text(_control, gui_position.x, gui_position.y, "Low Hue");
	gui_position += cv::Point(0, 15);
	cvui::trackbar(_control, gui_position.x, gui_position.y, 200, &_LowHue, 0, 179);
	
	//High Hue Trackbar
	gui_position += cv::Point(0, 45);
	cvui::text(_control, gui_position.x, gui_position.y, "High Hue");
	gui_position += cv::Point(0, 15);
	cvui::trackbar(_control, gui_position.x, gui_position.y, 200, &_HighHue, 0, 179);
	
	//Low Saturation Trackbar
	gui_position += cv::Point(0, 45);
	cvui::text(_control, gui_position.x, gui_position.y, "Low Saturation");
	gui_position += cv::Point(0, 15);
	cvui::trackbar(_control, gui_position.x, gui_position.y, 200, &_LowSat, 0, 255);
	
	//High Saturation Trackbar
	gui_position += cv::Point(0, 45);
	cvui::text(_control, gui_position.x, gui_position.y, "High Saturation");
	gui_position += cv::Point(0, 15);
	cvui::trackbar(_control, gui_position.x, gui_position.y, 200, &_HighSat, 0, 255);
	
	//Low Value Trackbar
	gui_position += cv::Point(0, 45);
	cvui::text(_control, gui_position.x, gui_position.y, "Low Value");
	gui_position += cv::Point(0, 15);
	cvui::trackbar(_control, gui_position.x, gui_position.y, 200, &_LowVal, 0, 255);
	
	//High Value Trackbar
	gui_position += cv::Point(0, 45);
	cvui::text(_control, gui_position.x, gui_position.y, "High Value");
	gui_position += cv::Point(0, 15);
	cvui::trackbar(_control, gui_position.x, gui_position.y, 200, &_HighVal, 0, 255);
	
	//On button
	gui_position += cv::Point(5,75);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "ON"))
	{
		_active = true;
	}
	
	//Off button
	gui_position += cv::Point(105,0);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "OFF"))
	{
		_active = false;
	}
	
	//Exit button
	gui_position += cv::Point(-53,40);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "EXIT"))
	{
		_do_exit = true;
	}
	
	if(!_active)
	{
	//Green Manual Sort
	gui_position += cv::Point(-52,40);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "GREEN") || gpioRead(GBUTTON) == false)
	{
		sort1();
		std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
		releaseBall();		
	}
	
	//Pink Manual Sort
	gui_position += cv::Point(105, 0);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "PINK") || gpioRead(PBUTTON) == false)
	{
		sort2();
		std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
		releaseBall();
	}
	
	//Yellow Manual Sort
	gui_position += cv::Point(-105,40);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "YELLOW") || gpioRead(YBUTTON) == false)
	{
		sort3();
		std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
		releaseBall();
	}
	
	//Blue Manual Sort
	gui_position += cv::Point(105, 0);
	if (cvui::button(_control, gui_position.x, gui_position.y, 100, 30, "BLUE") || gpioRead(BBUTTON) == false)
	{
		sort4();
		std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
		releaseBall();
	}	
}
	cvui::update();
	cv::imshow("Control", _control);	
}

void CRecycleSorter::Control_thread(CRecycleSorter* ptr)
{
    while (ptr->_do_exit == false)
    {
        ptr->ControlPanel();
    }
}

//void CRecycleSorter::tripwire_thread(CRecycleSorter* ptr)
//{
    //while (ptr->_do_exit == false)
    //{
        //ptr->tripwire();
    //}
//}
void CRecycleSorter::update_thread(CRecycleSorter* ptr)
{
    while (ptr->_do_exit == false)
    {
        ptr->update();
    }
}

void CRecycleSorter::sort1()
{
	gpioWrite(GREEN_RGB, 1);
	gpioServo(SERVO_1, SERVO_MIDDLE - (SERVO_QUARTER + 200));
	gpioServo(SERVO_2, SERVO_MIDDLE - SERVO_QUARTER);
}

void CRecycleSorter::sort2()
{
	gpioWrite(RED_RGB, 1);
	gpioServo(SERVO_1, SERVO_MIDDLE - (SERVO_QUARTER + 200));
	gpioServo(SERVO_2, SERVO_MIDDLE + SERVO_QUARTER);	
}

void CRecycleSorter::sort3()
{
	gpioWrite(GREEN_RGB, 1);
	gpioWrite(BLUE_RGB, 1);
	gpioServo(SERVO_1, SERVO_MIDDLE + 250 );
	gpioServo(SERVO_3, SERVO_MIDDLE - SERVO_QUARTER);
}

void CRecycleSorter::sort4()
{
	gpioWrite(BLUE_RGB, 1);
	gpioServo(SERVO_1, SERVO_MIDDLE + 250);
	gpioServo(SERVO_3, SERVO_MIDDLE + SERVO_QUARTER);
}

void CRecycleSorter::RGB_OFF()
{
	gpioWrite(RED_RGB, 0);
	gpioWrite(GREEN_RGB, 0);
	gpioWrite(BLUE_RGB, 0);
}

void CRecycleSorter::agitate()
{	
	gpioServo(SERVO_TOP1, SERVO_MIDDLE + (SERVO_QUARTER + 250));
	gpioServo(SERVO_TOP2, SERVO_MIDDLE + (SERVO_QUARTER + 250));
	std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(300));
	
	gpioServo(SERVO_TOP1, SERVO_MIDDLE - (SERVO_QUARTER + 250));
	gpioServo(SERVO_TOP2, SERVO_MIDDLE - (SERVO_QUARTER + 250));
	std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(400));
	gpioServo(SERVO_TOP2, SERVO_MIDDLE + (SERVO_QUARTER + 250));	
}
