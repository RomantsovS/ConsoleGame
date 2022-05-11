#ifndef TR_LOCAL_H
#define TR_LOCAL_H

#include "idlib/precompiled.h"

class idRenderWorldLocal;
class idRenderEntityLocal;
struct viewEntity_t;
struct portalArea_t;

// areas have references to hold all the lights and entities in them
struct areaReference_t {
	areaReference_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		idLib::Printf("%s ctor\n", "areaReference_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	~areaReference_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		idLib::Printf("%s dtor\n", "areaReference_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	areaReference_t(const areaReference_t&) = default;
	areaReference_t& operator=(const areaReference_t&) = default;
	areaReference_t(areaReference_t&&) = default;
	areaReference_t& operator=(areaReference_t&&) = default;

	std::shared_ptr<areaReference_t> areaNext;				// chain in the area
	std::shared_ptr<areaReference_t> areaPrev;
	std::shared_ptr<areaReference_t> ownerNext;				// chain on either the entityDef or lightDef
	idRenderEntityLocal* entity;					// only one of entity / light will be non-NULL
	portalArea_t *area;					// so owners can find all the areas they are in
};

class idRenderEntity
{
public:
	idRenderEntity() = default;
	virtual ~idRenderEntity() = default;
	idRenderEntity(const idRenderEntity&) = default;
	idRenderEntity& operator=(const idRenderEntity&) = default;
	idRenderEntity(idRenderEntity&&) = default;
	idRenderEntity& operator=(idRenderEntity&&) = default;

	/*virtual void FreeRenderEntity() = 0;
	virtual void UpdateRenderEntity(const renderEntity_s *re, bool forceUpdate = false) = 0;
	virtual void GetRenderEntity(renderEntity_s *re) = 0;
	virtual void ForceUpdate() = 0;
	virtual int	GetIndex() = 0;*/
};

class idRenderEntityLocal : public idRenderEntity
{
public:
	idRenderEntityLocal();
	~idRenderEntityLocal();
	idRenderEntityLocal(const idRenderEntityLocal&) = default;
	idRenderEntityLocal& operator=(const idRenderEntityLocal&) = default;
	idRenderEntityLocal(idRenderEntityLocal&&) = default;
	idRenderEntityLocal& operator=(idRenderEntityLocal&&) = default;

	/*virtual void FreeRenderEntity();
	virtual void UpdateRenderEntity(const renderEntity_t *re, bool forceUpdate = false);
	virtual void GetRenderEntity(renderEntity_t *re);
	virtual void ForceUpdate();
	virtual int	GetIndex();*/

	renderEntity_t parms;

	idRenderWorldLocal* world;
	int index; // in world entityDefs

	// a viewEntity_t is created whenever a idRenderEntityLocal is considered for inclusion
	// in a given view, even if it turns out to not be visible
	int viewCount;				// if tr.viewCount == viewCount, viewEntity is valid,
													// but the entity may still be off screen
	std::shared_ptr<viewEntity_t> viewEntity;				// in frame temporary memory

	std::shared_ptr<areaReference_t> entityRefs;				// chain of all references
};

//#define DEBUG_PRINT_Ctor_Dtor

// a viewEntity is created whenever a idRenderEntityLocal is considered for inclusion
// in the current view, but it may still turn out to be culled.
// viewEntity are allocated on the frame temporary stack memory
// a viewEntity contains everything that the back end needs out of a idRenderEntityLocal,
// which the front end may be modifying simultaneously if running in SMP mode.
// A single entityDef can generate multiple viewEntity_t in a single frame, as when seen in a mirror
struct viewEntity_t {
	viewEntity_t() 	{
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "viewEntity_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	~viewEntity_t() 	{
		clean();
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s dtor\n", "viewEntity_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	viewEntity_t(const viewEntity_t&) = default;
	viewEntity_t& operator=(const viewEntity_t&) = default;
	viewEntity_t(viewEntity_t&&) = default;
	viewEntity_t& operator=(viewEntity_t&&) = default;

	void clean() noexcept {
		while (next) {
			next = std::move(next->next);
		}
	}

	std::shared_ptr<viewEntity_t> next;

	// back end should NOT reference the entityDef, because it can change when running SMP
	idRenderEntityLocal* entityDef;

	// for scissor clipping, local inside renderView viewport
	// scissorRect.Empty() is true if the viewEntity_t was never actually
	// seen through any portals, but was created for shadow casting.
	// a viewEntity can have a non-empty scissorRect, meaning that an area
	// that it is in is visible, and still not be visible.
	//idScreenRect			scissorRect;

	// parallelAddModels will build a chain of surfaces here that will need to
	// be linked to the lights or added to the drawsurf list in a serial code section
	//drawSurf_t *			drawSurfs;
};

// viewDefs are allocated on the frame temporary stack memory
struct viewDef_t {
	viewDef_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "viewDef_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	~viewDef_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s dtor\n", "viewDef_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	viewDef_t(const viewDef_t&) = default;
	viewDef_t& operator=(const viewDef_t&) = default;
	viewDef_t(viewDef_t&&) = default;
	viewDef_t& operator=(viewDef_t&&) = default;

	// specified in the call to DrawScene()
	//renderView_t		renderView;

	//viewEntity_t		worldSpace;

	std::shared_ptr<idRenderWorldLocal> renderWorld;

	//idScreenRect		viewport;				// in real pixels and proper Y flip

	//idScreenRect		scissor;
	// for scissor clipping, local inside renderView viewport
	// subviews may only be rendering part of the main view
	// these are real physical pixel values, possibly scaled and offset from the
	// renderView x/y/width/height

	//viewDef_t *			superView;				// never go into an infinite subview loop 
	//const std::shared_ptr<drawSurf_t> subviewSurface;

	// drawSurfs are the visible surfaces of the viewEntities, sorted
	// by the material sort parameter
	//drawSurf_t **		drawSurfs;				// we don't use an idList for this, because

	std::shared_ptr<viewEntity_t> viewEntitys;			// chain of all viewEntities effecting view, including off screen ones casting shadows

	int					areaNum;				// -1 = not in a valid area
};

class idRenderSystemLocal : public idRenderSystem {
public:
	idRenderSystemLocal();
	~idRenderSystemLocal();
	idRenderSystemLocal(const idRenderSystemLocal&) = default;
	idRenderSystemLocal& operator=(const idRenderSystemLocal&) = default;
	idRenderSystemLocal(idRenderSystemLocal&&) = default;
	idRenderSystemLocal& operator=(idRenderSystemLocal&&) = default;

	void Clear();

	void Init() override;
	void Shutdown() override;

	std::shared_ptr<idRenderWorld> AllocRenderWorld() override;
	void FreeRenderWorld(std::shared_ptr<idRenderWorld> rw) override;
	void BeginLevelLoad() override;
	void EndLevelLoad() override;

	void SetColor(const Screen::color_type color) noexcept override;

	void DrawStretchPic(int x, int y, int w, int h, int s1, int t1, const idMaterial* material) override;
	void DrawBigChar(int x, int y, int ch) override;
	void DrawBigStringExt(int x, int y, const std::string& string, const Screen::color_type setColor, bool forceColor) override;

	void DrawPositionedString(Vector2 pos, const std::string& str, Screen::color_type color) override;
	void DrawString(const std::string& text, const Screen::color_type color) override;
	void RenderCommandBuffers() override;

	void SetHeight(int h) noexcept override { height = h; }
	void SetWidth(int w) noexcept override { width = w; }
	int GetWidth() const noexcept override { return width; }
	int GetHeight() const noexcept override { return height; }

	void Display() noexcept;

	void ClearScreen();

	void UpdateTimers() override;

	void SetConsoleTextTitle(const std::string& str) override;

	int frameCount;		// incremented every frame
	int viewCount;		// incremented every view (twice a scene if subviewed)
											// and every R_MarkFragments call

	std::list<std::shared_ptr<idRenderWorldLocal>> worlds;

	std::shared_ptr<idMaterial> charSetMaterial;
	//std::shared_ptr<idMaterial> defaultMaterial;

	std::shared_ptr<viewDef_t> viewDef;

	// GUI drawing variables for surface creation
	Screen::color_type currentColorNativeBytesOrder;

	Screen screen;

	Screen::pos_type width, height;// , borderWidth, borderHeight;
	Screen::Pixel borderPixel;

	int update_frame_time = 0;

	bool update_frame;
};

extern idRenderSystemLocal tr;

extern idCVar r_skipBackEnd; // don't draw anything
extern idCVar r_console_pos;

/*
============================================================

RENDERWORLD_DEFS

============================================================
*/

void R_CreateEntityRefs(idRenderEntityLocal* def);
void R_FreeEntityDefDerivedData(idRenderEntityLocal* def, bool keepDecals, bool keepCachedDynamicModel) noexcept;

void R_FreeDerivedData() noexcept;
void R_CheckForEntityDefsUsingModel(idRenderModel* model) noexcept;

/*
============================================================

RENDERWORLD_PORTALS

============================================================
*/

std::shared_ptr<viewEntity_t> R_SetEntityDefViewEntity(idRenderEntityLocal* def);

/*
====================================================================

TR_FRONTEND_MAIN

====================================================================
*/

void R_RenderView(std::shared_ptr<viewDef_t>);

/*
============================================================

TR_FRONTEND_ADDMODELS

============================================================
*/

void R_AddModels();

/*
=============================================================

BACKEND

=============================================================
*/

void RB_ExecuteBackEndCommands();

/*
============================================================

TR_BACKEND_DRAW

============================================================
*/

void RB_DrawViewInternal(/*const viewDef_t * viewDef, const int stereoEye*/);
void RB_DrawView(/*const void *data, const int stereoEye*/);

/*
=============================================================

TR_BACKEND_RENDERTOOLS

=============================================================
*/

void DrawFPS();

void RB_AddDebugText(std::string text, const Vector2 &origin, const Screen::color_type color, const int lifetime = 0);
void RB_ClearDebugText(int time);
void RB_AddDebugLine(const Screen::color_type color, const Vector2& start, const Vector2& end, const int lifeTime, const bool depthTest) noexcept;
void RB_ClearDebugLines(int time) noexcept;
void RB_RenderDebugToolsBefore();
void RB_RenderDebugTools();
void RB_ShowDebugText();
void RB_DrawText(const std::string& text, const Vector2& origin, const Screen::color_type color);

extern idCVar max_debug_text;

#include "RenderWorld_local.h"

#endif
