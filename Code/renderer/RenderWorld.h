#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

using deferredEntityCallback_t = bool(*)(renderEntity_t*, const renderView_t*);

struct renderEntity_t {
  idRenderModel* hModel;

  int entityNum;

  // Entities that are expensive to generate, like skeletal models, can be
  // deferred until their bounds are found to be in view, in the frustum
  // of a shadowing light that is in view, or contacted by a trace / overlay
  // test. This is also used to do visual cueing on items in the view The
  // renderView may be NULL if the callback is being issued for a non-view
  // related source. The callback function should clear renderEntity->callback
  // if it doesn't want to be called again next time the entity is referenced
  // (ie, if the callback has now made the entity valid until the next
  // updateEntity)
  idBounds bounds;  // only needs to be set for deferred models and md5s
  deferredEntityCallback_t callback;

  // positioning
  // axis rotation vectors must be unit length for many
  // R_LocalToGlobal functions to work, so don't scale models!
  // axis vectors are [0] = forward, [1] = left, [2] = up
  Vector2 origin;
  Vector2 axis;

  Screen::color_type color;

  Vector2* text_coords;
};

struct renderView_t {
  // player views will set this to a non-zero integer for model suppress / allow
  // subviews (mirrors, cameras, etc) will always clear it to zero
  int viewID;

  // time in milliseconds for shader effects and other time dependent rendering
  // issues
  int time[2];
};

class idRenderWorld {
 public:
  idRenderWorld() = default;
  virtual ~idRenderWorld() = default;
  idRenderWorld(const idRenderWorld&) = default;
  idRenderWorld& operator=(const idRenderWorld&) = default;
  idRenderWorld(idRenderWorld&&) = default;
  idRenderWorld& operator=(idRenderWorld&&) = default;

  // The same render world can be reinitialized as often as desired
  // a NULL or empty mapName will create an empty, single area world
  virtual bool InitFromMap(const std::string& mapName) = 0;

  virtual int AddEntityDef(const renderEntity_t* re) = 0;
  virtual void UpdateEntityDef(int entityHandle,
                               gsl::not_null<const renderEntity_t*> re) = 0;
  virtual void FreeEntityDef(int entityHandle) = 0;
  virtual const renderEntity_t* GetRenderEntity(int entityHandle) const = 0;

  // virtual void AddEntity(const renderEntity_s *ent) = 0;

  virtual void RenderScene(const renderView_t* renderView) = 0;

  //-------------- Tracing  -----------------

  // Checks a ray trace against any gui surfaces in an entity, returning the
  // fraction location of the trace on the gui surface, or -1,-1 if no hit.
  // This doesn't do any occlusion testing, simply ignoring non-gui surfaces.
  // start / end are in global world coordinates.
  // virtual guiPoint_t		GuiTrace(qhandle_t entityHandle, const idVec3
  // start, const idVec3 end) const = 0;

  //-------------- Debug Visualization  -----------------
  // Line drawing for debug visualization
  virtual void DebugClearLines(
      int time) = 0;  // a time of 0 will clear all lines and text
  virtual void DebugLine(const Screen::color_type color, const Vector2& start,
                         const Vector2& end, const int lifetime = 0,
                         const bool depthTest = false) = 0;
  virtual void DebugBounds(const Screen::color_type color,
                           const idBounds& bounds,
                           const Vector2& org = vec2_origin,
                           const int lifetime = 0) = 0;

  // Text drawing for debug visualization.
  virtual void DrawTextToScreen(std::string text, const Vector2& origin,
                                const Screen::color_type color,
                                const int lifetime) = 0;

  virtual std::shared_ptr<idRenderWorld> getptr() = 0;
};

#endif