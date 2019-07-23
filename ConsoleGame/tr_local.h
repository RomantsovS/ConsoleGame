#ifndef TR_LOCAL_H
#define TR_LOCAL_H

#include <algorithm>

#include "RenderWorld.h"
#include "RenderSystem.h"

class idRenderWorldLocal;
struct viewEntity_t;

// areas have references to hold all the lights and entities in them
struct areaReference_t
{
	std::shared_ptr<areaReference_t> areaNext;				// chain in the area
	std::shared_ptr<areaReference_t> areaPrev;
	std::shared_ptr<areaReference_t> ownerNext;				// chain on either the entityDef or lightDef
	std::shared_ptr<idRenderEntityLocal> entity;					// only one of entity / light will be non-NULL
	struct portalArea_t *area;					// so owners can find all the areas they are in
};

class idRenderEntity
{
public:
	virtual ~idRenderEntity();

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

	/*virtual void FreeRenderEntity();
	virtual void UpdateRenderEntity(const renderEntity_t *re, bool forceUpdate = false);
	virtual void GetRenderEntity(renderEntity_t *re);
	virtual void ForceUpdate();
	virtual int	GetIndex();*/

	renderEntity_t parms;

	std::shared_ptr<idRenderWorldLocal> world;
	int index; // in world entityDefs

	// a viewEntity_t is created whenever a idRenderEntityLocal is considered for inclusion
	// in a given view, even if it turns out to not be visible
	int viewCount;				// if tr.viewCount == viewCount, viewEntity is valid,
													// but the entity may still be off screen
	std::shared_ptr<viewEntity_t> viewEntity;				// in frame temporary memory

	std::shared_ptr<areaReference_t> entityRefs;				// chain of all references
};

// a viewEntity is created whenever a idRenderEntityLocal is considered for inclusion
// in the current view, but it may still turn out to be culled.
// viewEntity are allocated on the frame temporary stack memory
// a viewEntity contains everything that the back end needs out of a idRenderEntityLocal,
// which the front end may be modifying simultaneously if running in SMP mode.
// A single entityDef can generate multiple viewEntity_t in a single frame, as when seen in a mirror
struct viewEntity_t
{
	viewEntity_t()
	{
		++count;
	}

	~viewEntity_t()
	{
		--count;
	}

	std::shared_ptr<viewEntity_t> next;

	// back end should NOT reference the entityDef, because it can change when running SMP
	std::shared_ptr<idRenderEntityLocal> entityDef;

	// for scissor clipping, local inside renderView viewport
	// scissorRect.Empty() is true if the viewEntity_t was never actually
	// seen through any portals, but was created for shadow casting.
	// a viewEntity can have a non-empty scissorRect, meaning that an area
	// that it is in is visible, and still not be visible.
	//idScreenRect			scissorRect;

	// parallelAddModels will build a chain of surfaces here that will need to
	// be linked to the lights or added to the drawsurf list in a serial code section
	//drawSurf_t *			drawSurfs;

	static int count;
};

// viewDefs are allocated on the frame temporary stack memory
struct viewDef_t
{

	// specified in the call to DrawScene()
	renderView_t		renderView;

	viewEntity_t		worldSpace;

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

class idRenderSystemLocal : public idRenderSystem
{
public:
	idRenderSystemLocal();

	~idRenderSystemLocal();

	virtual void Init();
	virtual void Shutdown();

	virtual std::shared_ptr<idRenderWorld> AllocRenderWorld();
	virtual void FreeRenderWorld(std::shared_ptr<idRenderWorld> rw);
	virtual void BeginLevelLoad();
	virtual void EndLevelLoad();

	void SetHeight(size_t h) { height = h; }
	void SetWidth(size_t w) { width = w; }

	virtual void Draw(const renderEntity_t &ent);

	void Display();

	virtual void Clear();

	void FillBorder();
	void ClearScreen();

	int frameCount;		// incremented every frame
	int viewCount;		// incremented every view (twice a scene if subviewed)
											// and every R_MarkFragments call

	std::list<std::shared_ptr<idRenderWorldLocal>> worlds;

	std::shared_ptr<viewDef_t> viewDef;

	Screen screen;

	size_t height, width, borderWidth, borderHeight;
	Screen::Pixel borderPixel;
};

extern idRenderSystemLocal tr;

/*
============================================================

RENDERWORLD_DEFS

============================================================
*/

void R_CreateEntityRefs(std::shared_ptr<idRenderEntityLocal> def);
void R_FreeEntityDefDerivedData(std::shared_ptr<idRenderEntityLocal> def, bool keepDecals, bool keepCachedDynamicModel);

void R_FreeDerivedData();
void R_CheckForEntityDefsUsingModel(std::shared_ptr<idRenderModel> model);

/*
============================================================

RENDERWORLD_PORTALS

============================================================
*/

std::shared_ptr<viewEntity_t> R_SetEntityDefViewEntity(std::shared_ptr<idRenderEntityLocal> def);

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

#endif