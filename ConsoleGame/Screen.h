#ifndef SCREEN
#define SCREEN

#include <string>
#include <iostream>
#include <ctime>

class Screen {
public:
    typedef std::string::size_type pos;

	Screen(std::ostream &os, pos ht, pos wd, char back = ' '):
		height(ht), width(wd), backgroundSymbol(back), contents(ht * wd, backgroundSymbol), outputStream(os),
		buffer(new char[ht * (wd + 1) + 1]), FPS(0) {}

	~Screen()
	{
		delete [] buffer;
	}

    char get() const              // get the character at the cursor
	    { return contents[cursor]; }       // implicitly inline
    inline char get(pos ht, pos wd) const; // explicitly inline

    Screen &move(pos r, pos c);      // can be made inline later
    Screen &set(char);
    Screen &set(pos, pos, char);
	
	pos getHeight() const { return height; }
	pos getWidth() const { return width; }

	const char &getBackgroundSymbol() const { return backgroundSymbol; }

	void clear() { std::fill(contents.begin(), contents.end(), backgroundSymbol); }

	Screen &display();
private:
	pos cursor;
	pos height, width;
	char backgroundSymbol;
	char *buffer;
	size_t FPS;

	std::string contents;
	std::ostream &outputStream;
};

inline                   // we can specify inline on the definition
Screen &Screen::move(pos r, pos c)
{
    pos row = r * width; // compute the row location
    cursor = row + c;    // move cursor to the column within that row
    return *this;        // return this object as an lvalue
}

char Screen::get(pos r, pos c) const // declared as inline in the class
{
    pos row = r * width;      // compute row location
    return contents[row + c]; // return character at the given column
}

inline Screen &Screen::set(char c) 
{ 
    contents[cursor] = c; // set the new value at the current cursor location
    return *this;         // return this object as an lvalue
}
inline Screen &Screen::set(pos r, pos col, char ch)
{
	if (r >= height)
		throw std::out_of_range("height out of range");

	if (col >= width)
		throw std::out_of_range("width out of range");

	contents[r*width + col] = ch;  // set specified location to given value

	return *this;                  // return this object as an lvalue
}

#endif