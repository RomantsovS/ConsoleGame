#include "Screen.h"

Screen& Screen::display()
{
	static clock_t lastClock = clock();

	auto curClock = clock();

	static std::string str = "FPS: ";

	if (curClock - lastClock >= CLOCKS_PER_SEC)
	{
		str = "FPS: " + std::to_string(FPS);

		FPS = 0;
		lastClock = curClock;
	}
	else
		FPS++;

	/*for (pos h = 0; h != height; ++h)
	{
		for (pos w = 0; w != width; ++w)
		{
			outputStream << contents[h * width + w];
		}
		outputStream << std::endl;
	}
	outputStream << std::endl;

	outputStream << str;*/

	char *p_next_write = &buffer[0];

	for (pos y = 0; y < height; y++)
	{
		for (pos x = 0; x < width; x++)
		{
			*p_next_write++ = contents[y * width + x];
		}
		*p_next_write++ = '\n';
	}

	for (auto iter = str.cbegin(); iter != str.cend(); ++iter)
		*p_next_write++ = *iter;

	*p_next_write = '\0';


	outputStream.write(&buffer[0], p_next_write - &buffer[0]);

	return *this;
}