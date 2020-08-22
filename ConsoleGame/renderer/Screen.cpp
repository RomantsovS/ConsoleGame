#include <random>
#include <cwchar>

#include "Screen.h"
#include "../sys/sys_public.h"

Screen::Screen(pos_type ht, pos_type wd, Pixel back) :
	height(ht), width(wd), backgroundPixel(back), buffer(nullptr), h_console_std_out(0), 
	h_console_draw(0), cur_write_coord({0, 0})
{
}

Screen::~Screen()
{
	if(buffer)
		delete[] buffer;
}

void Screen::init()
{
	contents.resize(height * width);
	clearContents();

	cur_write_coord = { 0, 0 };

	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions

	MoveWindow(console, r.left, r.top, 800, 600, true);

	buffer = new char[height * (width)];

	h_console_std_out = GetStdHandle(STD_OUTPUT_HANDLE);
	h_console_draw = CreateConsoleScreenBuffer(GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

	if (h_console_draw == INVALID_HANDLE_VALUE || h_console_std_out == INVALID_HANDLE_VALUE)
		common->FatalError("CreateConsoleScreenBuffer  failed - (%d)\n", GetLastError());

	if(!SetConsoleActiveScreenBuffer(h_console_draw))
		common->FatalError("SetConsoleActiveScreenBuffer  failed - (%d)\n", GetLastError());

	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(h_console_draw, &cursorInfo);
	cursorInfo.bVisible = false; // set the cursor visibility
	SetConsoleCursorInfo(h_console_draw, &cursorInfo);

	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;                   // Width of each character in the font
	cfi.dwFontSize.Y = 32;                  // Height
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, L"Consolas"); // Choose your font
	SetCurrentConsoleFontEx(h_console_draw, FALSE, &cfi);
}

void Screen::clear()
{
	clearContents();
}

void Screen::clearTextInfo()
{
	DWORD written;
	FillConsoleOutputCharacter(h_console_draw, ' ', width * 20, COORD({ 0, height + 1 }), &written);
}

void Screen::display(const std::string &str)
{
	cur_write_coord = { 0, 0 };

	char* p_next_write = &buffer[0];
	ConsoleColor curCol = Screen::ConsoleColor::None;

	for (pos_type y = 0; y < height; y++)
	{
		for (pos_type x = 0; x < width; x++)
		{
			if (curCol == Screen::ConsoleColor::None)
			{
				curCol = contents[y * width + x].color;
			}
			else
			{
				if (curCol != contents[y * width + x].color)
				{
					writeInColor(cur_write_coord, buffer, p_next_write - &buffer[0], curCol);

					curCol = contents[y * width + x].color;

					cur_write_coord = { x, y };
					p_next_write = &buffer[0];
				}
			}

			*p_next_write++ = contents[y * width + x].value;
		}
	}

	std::string text_full_string = str;

	if (str.size() % width != 0)
	{
		text_full_string.append(width - (str.size() % width), ' ');
	}

	for (auto iter = text_full_string.cbegin(); iter != text_full_string.cend(); ++iter)
		*p_next_write++ = *iter;

	auto lenght = p_next_write - &buffer[0];
	writeInColor(cur_write_coord, buffer, lenght, curCol);

	cur_write_coord.Y += static_cast<SHORT>(ceil(lenght / width)) + 1;
	cur_write_coord.X = 0;
}

void Screen::clearContents()
{
	for (size_t i = 0; i != height * width; ++i)
	{
		contents[i] = Pixel(backgroundPixel.value, backgroundPixel.color);
	}
}

void Screen::writeInColor(COORD coord, const char* symbol, size_t lenght, Screen::ConsoleColor color_text, Screen::ConsoleColor color_background)
{
	if (color_background == ConsoleColor::None)
		color_background = backgroundPixel.color;

	std::vector<WORD> attribute(lenght, (WORD)((color_background << 4) | color_text | FOREGROUND_INTENSITY));
	DWORD written;
	
	WriteConsoleOutputAttribute(h_console_draw, &attribute[0], lenght, coord, &written);
	WriteConsoleOutputCharacter(h_console_draw, symbol, lenght, coord, &written);
}

void Screen::writeInColor(const std::string& text, ConsoleColor color_text, ConsoleColor color_background)
{
	std::string text_full_string = text;

	if (text.size() % width != 0)
	{
		text_full_string.append(width - (text.size() % width), ' ');
	}

	writeInColor(cur_write_coord, text_full_string.c_str(), text_full_string.size(), color_text, color_background);

	cur_write_coord.Y += static_cast<SHORT>(ceil(static_cast<pos_type>(text.size()) / width)) + 1;
	cur_write_coord.X = 0;
}

bool Screen::readInput(unsigned& key)
{
	/*DWORD events;
	INPUT_RECORD input_record;

	// update the game time
	static auto real_time_last = Sys_Milliseconds();
	auto real_time = Sys_Milliseconds();

	if (real_time - real_time_last < 100) {
		return false;
	}

	real_time_last = real_time;

	key = 0;

	GetNumberOfConsoleInputEvents(h_console_std_in, &events);

	if (!events)
		return false;

	ReadConsoleInput(h_console_std_in, &input_record, 1, &events);
	
	switch (input_record.EventType)
	{
	case KEY_EVENT:
	{
		if (input_record.Event.KeyEvent.bKeyDown)
		{
			key = input_record.Event.KeyEvent.wVirtualKeyCode;
			return true;
		}
	}
	}*/

	return false;
}

std::string Screen::waitConsoleInput()
{
	/*std::string text_input;
	DWORD events;
	INPUT_RECORD input_record;
	COORD coord = { 0, 0 };
	DWORD written;

	while (true)
	{
		ReadConsoleInput(h_console_std_in, &input_record, 1, &events);

		if (input_record.EventType == KEY_EVENT && input_record.Event.KeyEvent.bKeyDown)
		{
			if (input_record.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
				return std::string();
			else if (input_record.Event.KeyEvent.wVirtualKeyCode == VK_RETURN)
				return text_input;
			else if (input_record.Event.KeyEvent.wVirtualKeyCode == VK_SHIFT ||
				input_record.Event.KeyEvent.wVirtualKeyCode == VK_CONTROL) {
				continue;
			}

			text_input += input_record.Event.KeyEvent.uChar.AsciiChar;
			WriteConsole(h_console_std_out, &input_record.Event.KeyEvent.uChar.AsciiChar, 1, &written, NULL);
		}
	}*/

	return "";
}

void Screen::writeConsoleOutput(const std::string& text)
{
	DWORD written;
	WriteConsole(h_console_std_out, text.c_str(), text.size(), &written, NULL);
}

void Screen::clearConsoleOutut()
{
	DWORD written;
	COORD coord({ 0, 0 });

	FillConsoleOutputCharacter(h_console_std_out, ' ', 255, coord, &written);
	SetConsoleCursorPosition(h_console_std_out, coord);
}

void Screen::setDrawOutputBuffer()
{
	if (!SetConsoleActiveScreenBuffer(h_console_draw))
		common->FatalError("SetConsoleActiveScreenBuffer  failed - (%d)\n", GetLastError());
}

void Screen::setStdOutputBuffer()
{
	if (!SetConsoleActiveScreenBuffer(h_console_std_out))
		common->FatalError("SetConsoleActiveScreenBuffer  failed - (%d)\n", GetLastError());
}
