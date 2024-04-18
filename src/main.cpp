
#include "ctrl.h"
#include <thread>

int main(int argc, char* argv[])
{
	Ctrl::GetInstance().start();
	std::this_thread::sleep_for(std::chrono::seconds(999999999999));
	return 0;
}