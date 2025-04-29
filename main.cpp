#include "PiControl.h"
#include "CRecycleSorter.h"

int main()
{
	if (gpioInitialise() < 0)
	{
		return 1;
	}
		
	PiControl Control;
		
	Control.gpio_setup();
	
	char cmd1 = -1;
	
	do
	{
		cmd1 = -1;
		
		Control.print_menu();

		std::cin >> cmd1;
		switch (cmd1)
		{
		case 'a':
		case 'A': Control.display_analog();
			break;

		case 'b':
		case 'B': Control.button_LED();
			break;

		case 'D':
		case 'd': Control.button_count();
			break;

		case 'p':
		case 'P': 
		{
			//std::cout << "R";
			CRecycleSorter Recycler;
			//std::cout <<"make";
			Recycler.run();
			break;
		}			

		case 's':
		case 'S': Control.servo_TEST();
			break;
		
		case 'r':
		case 'R': 
		{
			XInitThreads();
			CRecycleSorter Recycler;
			Recycler.run();
			break;
		}
		
		case 'c':
		case 'C':
		{
			CRecycleSorter Rec;
			Rec.calibrate();
			break;
		}
		
		case 'J':
		{
			CRecycleSorter Rec;
			Rec.releaseBall();
			break;
		}
		
		case 'G':
		{
			CRecycleSorter Rec;
			Rec.gate();
			break;
		}
		
		case 'L':
		{
			CRecycleSorter Rec;
			Rec.agitate();
			break;
		}
		
		case 'q':
		case 'Q': cmd1 = 'q';
			break;

		default:
			std::cout << "\nunsupported character \n\n";
			break;
		}
	} while (cmd1 != 'q' && 'Q');


	gpioTerminate();

	return 0;

}
