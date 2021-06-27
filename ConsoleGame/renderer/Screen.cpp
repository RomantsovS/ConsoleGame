#pragma hdrstop
#include "../idlib/precompiled.h"

#include "tr_local.h"

idCVar window_font_width("window_font_width", "8", CVAR_SYSTEM | CVAR_INIT, "");
idCVar window_font_height("window_font_height", "8", CVAR_SYSTEM | CVAR_INIT, "");
idCVar text_info_max_height("text_info_max_height", "20", CVAR_SYSTEM | CVAR_INIT, "");

Screen::Screen(pos_type ht, pos_type wd, Pixel back) :
	height(ht), width(wd), backgroundPixel(back), buffer(nullptr), cur_write_coord({0, 0}),
	window_rect({ 0, 0, 1, 1 }) {
	h_console_std_out = GetStdHandle(STD_OUTPUT_HANDLE);
	h_console_std_in = GetStdHandle(STD_INPUT_HANDLE);
}

Screen::~Screen() {
	if(buffer)
		delete[] buffer;
}

void Screen::init() {
	if (h_console_std_out == INVALID_HANDLE_VALUE)
		common->FatalError("Bad h_console_std_out");

	// Change console visual size to a minimum so ScreenBuffer can shrink
		// below the actual visual size
	SetConsoleWindowInfo(h_console_std_out, TRUE, &window_rect);

	// Set the size of the screen buffer
	COORD coord = { (short)width, (short)height + static_cast<short>(text_info_max_height.GetInteger()) };
	if (!SetConsoleScreenBufferSize(h_console_std_out, coord)) {
		common->FatalError("SetConsoleScreenBufferSize failed - (%s)\n", getLastErrorMsg());
	}

	if(!SetConsoleActiveScreenBuffer(h_console_std_out))
		common->FatalError("SetConsoleActiveScreenBuffer  failed - (%s)\n", getLastErrorMsg());

	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(h_console_std_out, &cursorInfo);
	cursorInfo.bVisible = false; // set the cursor visibility
	SetConsoleCursorInfo(h_console_std_out, &cursorInfo);

	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = window_font_width.GetInteger(); // Width of each character in the font
	cfi.dwFontSize.Y = window_font_height.GetInteger(); // Height
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, L"Consolas"); // Choose your font
	SetCurrentConsoleFontEx(h_console_std_out, FALSE, &cfi);

	// Get screen buffer info and check the maximum allowed window size. Return
		// error if exceeded, so user knows their dimensions/fontsize are too large
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(h_console_std_out, &csbi))
		common->FatalError("GetConsoleScreenBufferInfo");
	if (height > csbi.dwMaximumWindowSize.Y)
		common->FatalError("Screen Height / Font Height Too Big");
	if (width > csbi.dwMaximumWindowSize.X)
		common->FatalError("Screen Width / Font Width Too Big");

	// Set Physical Console Window Size
	window_rect = { 0, 0, static_cast<short>(width) - 1, (short)height + static_cast<short>(text_info_max_height.GetInteger()) - 1 };
	if (!SetConsoleWindowInfo(h_console_std_out, TRUE, &window_rect))
		common->FatalError("SetConsoleWindowInfo wrong width or height");

	// Set flags to allow mouse input		
	if (!SetConsoleMode(h_console_std_in, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT))
		common->FatalError("SetConsoleMode std in");

	// Allocate memory for screen buffer
	buffer = new CHAR_INFO[width * height];
	memset(buffer, 0, sizeof(CHAR_INFO) * width * height);

	contents.resize(height * width);
	clearContents();

	cur_write_coord = { 0, 0 };
}

Screen& Screen::set(pos_type r, pos_type col, Screen::Pixel ch) {
	if (r >= height || r < 0) {
		return *this;
		common->Error("Screen height: %d out of range: %d", r, height);
	}

	if (col >= width || col < 0) {
		return *this;
		common->Error("Screen width: %d out of range: %d", col, width);
	}

	contents[r * width + col] = ch;  // set specified location to given value

	return *this;                  // return this object as an lvalue
}

void Screen::clear() {
	clearContents();
}

void Screen::clearTextInfo() {
	DWORD written;
	FillConsoleOutputCharacter(h_console_std_out, ' ', width * 20, COORD({ 0, cur_write_coord.Y }), &written);
}

void Screen::display() {
	cur_write_coord = { 0, 0 };

	for (pos_type y = 0; y < height; ++y) {
		for (pos_type x = 0; x < width; ++x) {
			buffer[y * width + x].Char.AsciiChar = contents[y * width + x].value;
			buffer[y * width + x].Attributes = static_cast<int>(contents[y * width + x].color);
		}
	}

	WriteConsoleOutput(h_console_std_out, buffer, { (short)width, (short)height }, { 0,0 }, &window_rect);
	cur_write_coord.Y += height;
}

void Screen::clearContents() {
	for (size_t i = 0; i != height * width; ++i)
	{
		contents[i] = Pixel(backgroundPixel.value, backgroundPixel.color);
	}
}

void Screen::writeInColor(COORD coord, const char* symbol, size_t lenght, Screen::ConsoleColor color_text, Screen::ConsoleColor color_background) {
	if (color_background == ConsoleColor::None)
		color_background = backgroundPixel.color;

	std::vector<WORD> attribute(lenght, (WORD)((static_cast<int>(color_background) << 4) | static_cast<int>(color_text) | FOREGROUND_INTENSITY));
	DWORD written;
	
	WriteConsoleOutputAttribute(h_console_std_out, &attribute[0], lenght, coord, &written);
	WriteConsoleOutputCharacter(h_console_std_out, symbol, lenght, coord, &written);
}

void Screen::writeInColor(const std::string& text, ConsoleColor color_text, ConsoleColor color_background) {
	std::string text_full_string = text;

	if (text.size() % width != 0) {
		text_full_string.append(width - (text.size() % width), ' ');
	}

	writeInColor(cur_write_coord, text_full_string.c_str(), text_full_string.size(), color_text, color_background);

	cur_write_coord.Y += static_cast<SHORT>(ceil(static_cast<pos_type>(text.size()) / width)) + 1;
	cur_write_coord.X = 0;
}

bool Screen::readInput(unsigned& key) {
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

std::string Screen::waitConsoleInput() {
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

void Screen::writeConsoleOutput(const std::string& text) {
	DWORD written;
	WriteConsole(h_console_std_out, text.c_str(), text.size(), &written, NULL);
}

void Screen::clearConsoleOutut() {
	DWORD written;
	COORD coord({ 0, 0 });

	FillConsoleOutputCharacter(h_console_std_out, ' ', 255, coord, &written);
	SetConsoleCursorPosition(h_console_std_out, coord);
}

void Screen::setDrawOutputBuffer() {
	if (!SetConsoleActiveScreenBuffer(h_console_std_out))
		common->FatalError("SetConsoleActiveScreenBuffer  failed - (%s)\n", getLastErrorMsg());
}

void Screen::setStdOutputBuffer() {
	if (!SetConsoleActiveScreenBuffer(h_console_std_out))
		common->FatalError("SetConsoleActiveScreenBuffer  failed - (%s)\n", getLastErrorMsg());
}
