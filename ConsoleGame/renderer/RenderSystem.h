#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

bool R_IsInitialized();

class idRenderWorld;

class idRenderSystem
{
public:	
	idRenderSystem();

	virtual ~idRenderSystem() = 0;

	virtual void Init() = 0;

	// only called before quitting
	virtual void Shutdown() = 0;

	virtual void SetHeight(int h) = 0;
	virtual void SetWidth(int w) = 0;
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;

	// allocate a renderWorld to be used for drawing
	virtual std::shared_ptr<idRenderWorld> AllocRenderWorld() = 0;
	virtual	void FreeRenderWorld(std::shared_ptr<idRenderWorld> rw) = 0;

	// All data that will be used in a level should be
	// registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time
	// if necessary.
	virtual void BeginLevelLoad() = 0;
	virtual void EndLevelLoad() = 0;

	virtual void DrawStretchPic(int x, int y, int w, int h, int s1, int t1, const idMaterial* material) = 0;

	virtual void DrawBigChar(int x, int y, int ch) = 0;
	virtual void DrawBigStringExt(int x, int y, const std::string& string, const int setColor, bool forceColor) = 0;

	virtual void DrawPositionedString(Vector2 pos, const std::string& str, int color) = 0;
	virtual void DrawString(const std::string& text, const int color) = 0;

	virtual void RenderCommandBuffers() = 0;

	virtual void UpdateTimers() = 0;
};

extern idRenderSystem *renderSystem;
extern bool r_initialized;

#endif