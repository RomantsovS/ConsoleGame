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

	char *p_next_write = &buffer[0];

	ConsoleColor curCol;
	bool firstSymbol = true;

	for (pos y = 0; y < height; y++)
	{
		for (pos x = 0; x < width; x++)
		{
			if (firstSymbol)
			{
				curCol = contents[y * width + x].color;
				firstSymbol = false;
			}
			else
			{
				if (curCol != contents[y * width + x].color)
				{
					*p_next_write = '\0';

					SetColor(curCol, backgroundPixel.color);

					outputStream.write(&buffer[0], p_next_write - &buffer[0]);

					p_next_write = &buffer[0];

					curCol = contents[y * width + x].color;
				}
			}

			*p_next_write++ = contents[y * width + x].value;
		}
		*p_next_write++ = '\n';
	}

	for (auto iter = str.cbegin(); iter != str.cend(); ++iter)
		*p_next_write++ = *iter;

	*p_next_write = '\0';

	SetColor(curCol, backgroundPixel.color);
	outputStream.write(&buffer[0], p_next_write - &buffer[0]);

	return *this;
}