#include "idlib/precompiled.h"
#pragma hdrstop

constexpr int MAX_EDIT_LINE = 50;

/*
===============
idEditField::idEditField
===============
*/
idEditField::idEditField() {
	buffer.reserve(MAX_EDIT_LINE);
	Clear();
}

/*
===============
idEditField::~idEditField
===============
*/
idEditField::~idEditField() {
}

/*
===============
idEditField::Clear
===============
*/
void idEditField::Clear() noexcept {
	buffer.clear();
}

/*
===============
idEditField::CharEvent
===============
*/
void idEditField::CharEvent(int ch) {
	//
	// ignore any other non printable chars
	//
	if (ch < 32) {
		return;
	}

	if (buffer.size() == MAX_EDIT_LINE - 1) {
		return; // all full
	}

	buffer.append(1, ch);
}

/*
===============
idEditField::KeyDownEvent
===============
*/
void idEditField::KeyDownEvent(int key) {
	if (key == static_cast<int>(keyNum_t::K_BACKSPACE)) {
		if (!buffer.empty()) {
			buffer.pop_back();
		}
		return;
	}
	else if (key == static_cast<int>(keyNum_t::K_SPACE)) {
		buffer.append(1, ' ');
	}
}

/*
===============
idEditField::GetBuffer
===============
*/
const char* idEditField::GetBuffer() noexcept {
	return buffer.c_str();
}