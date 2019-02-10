#ifndef RENDER_ENTITY_H
#define RENDER_ENTITY_H

#include <algorithm>

#include "RenderWorld.h"
#include "RenderSystem.h"

class RenderEntity
{
public:
	virtual ~RenderEntity();

	/*virtual void FreeRenderEntity() = 0;
	virtual void UpdateRenderEntity(const renderEntity_s *re, bool forceUpdate = false) = 0;
	virtual void GetRenderEntity(renderEntity_s *re) = 0;
	virtual void ForceUpdate() = 0;
	virtual int	GetIndex() = 0;*/
};

class RenderEntityLocal : public RenderEntity
{
public:
	RenderEntityLocal();

	~RenderEntityLocal();

	/*virtual void FreeRenderEntity();
	virtual void UpdateRenderEntity(const renderEntity_t *re, bool forceUpdate = false);
	virtual void GetRenderEntity(renderEntity_t *re);
	virtual void ForceUpdate();
	virtual int	GetIndex();*/

	renderEntity_s parms;

	int index;
};

class RenderSystemLocal : public RenderSystem
{
public:
	RenderSystemLocal();

	~RenderSystemLocal();

	virtual void Init();

	void SetHeight(size_t h) { height = h; }
	void SetWidth(size_t w) { width = w; }

	virtual void Draw(const renderEntity_s &ent);

	void Display();

	virtual void Clear();

	void FillBorder();
private:
	RenderWorld *renderWorld;

	Screen screen;

	size_t height, width, borderWidth, borderHeight;
	Screen::Pixel borderPixel;
};

extern RenderSystemLocal tr;

#endif