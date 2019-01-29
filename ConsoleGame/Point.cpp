#include "Point.h"
#include "RenderStatic.h"
#include "PhysicsStatic.h"

/*SimpleObject::SimpleObject(Screen::Pixel drawPix) : drawPixel(drawPix), active(true)
{
}

SimpleObject::~SimpleObject()
{
}*/

Point::Point(pos_type position, Screen::Pixel drawPix) : pos(position), drawPixel(drawPix)
{
	int a = 0;
}

Point::~Point()
{
	int a = 0;
}

void Point::init()
{
	auto renderEntity = std::make_shared<RenderStatic>();

	setRenderEntity(renderEntity);

	auto physics = std::make_shared<PhysicsStatic>();

	setPhysics(physics);
}

void Point::update()
{
}

/*bool Point::checkCollide(const pos_type pos) const
{
	return getPos() != pos;
}

void Point::drawToScreen(Screen & screen)
{
	screen.set(pos.h, pos.w, drawPixel);
}*/
