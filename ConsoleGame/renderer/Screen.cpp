#include <random>

#include "Screen.h"

Screen::Screen(pos ht, pos wd, Pixel back) : height(ht), width(wd), backgroundPixel(back)
{
	contents.resize(ht * wd);
	buffer.resize(ht * (wd + 1) + 1);

	clearContents();
}

Screen::~Screen()
{
}

void Screen::init()
{
	//std::cout.setf()
}

Screen& Screen::display(const std::string &str)
{
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

					std::cout.write(&buffer[0], p_next_write - &buffer[0]);

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

	*p_next_write++ = '\n';

	SetColor(curCol, backgroundPixel.color);
	std::cout.write(&buffer[0], p_next_write - &buffer[0]);
	SetColor(Screen::ConsoleColor::White, backgroundPixel.color);

	return *this;
}

void Screen::clearContents()
{
	for (size_t i = 0; i != height * width; ++i)
	{
		contents[i] = Pixel(backgroundPixel.value, backgroundPixel.color);
	}
}

void SetColor(Screen::ConsoleColor text, Screen::ConsoleColor background)
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}