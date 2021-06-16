#ifndef IDLIB_MAPFILE_H_
#define IDLIB_MAPFILE_H_

#include <string>
#include <memory>
#include <vector>

#include "Dict.h"

const int OLD_MAP_VERSION = 1;
const int CURRENT_MAP_VERSION = 2;

class idMapPrimitive {
public:
	enum { TYPE_INVALID = -1, TYPE_BRUSH, TYPE_PATCH };

	idDict					epairs;

	idMapPrimitive() { type = TYPE_INVALID; }
	virtual					~idMapPrimitive() { }
	int						GetType() const { return type; }

protected:
	int						type;
};

class idMapBrushSide {
	friend class idMapBrush;

public:
	idMapBrushSide();
	~idMapBrushSide() { }
	/*const char* GetMaterial() const { return material; }
	void					SetMaterial(const char* p) { material = p; }
	const idPlane& GetPlane() const { return plane; }
	void					SetPlane(const idPlane& p) { plane = p; }
	void					SetTextureMatrix(const idVec3 mat[2]) { texMat[0] = mat[0]; texMat[1] = mat[1]; }
	void					GetTextureMatrix(idVec3& mat1, idVec3& mat2) { mat1 = texMat[0]; mat2 = texMat[1]; }
	void					GetTextureVectors(idVec4 v[2]) const;
	*/
	const std::vector<Vector2>& GetPoints() const { return points; }
protected:
	//idStr					material;
	//idPlane					plane;
	//idVec3					texMat[2];
	std::vector<Vector2> points;
	Vector2 origin;
};

inline idMapBrushSide::idMapBrushSide() {
	//plane.Zero();
	//texMat[0].Zero();
	//texMat[1].Zero();
	points.resize(2);
	origin.Zero();
}


class idMapBrush : public idMapPrimitive {
public:
	idMapBrush() { type = TYPE_BRUSH; sides.reserve(8); }
	~idMapBrush() { sides.clear(); }
	static std::shared_ptr<idMapBrush> Parse(idLexer& src, const Vector2& origin, bool newFormat = true, float version = CURRENT_MAP_VERSION);
	//bool					Write(idFile* fp, int primitiveNum, const Vector2& origin) const;
	int						GetNumSides() const { return sides.size(); }
	int						AddSide(std::shared_ptr<idMapBrushSide> side) { sides.push_back(side); return sides.size(); }
	std::shared_ptr<idMapBrushSide> GetSide(int i) const { return sides[i]; }
	unsigned int			GetGeometryCRC() const;

protected:
	//int						numSides;
	std::vector<std::shared_ptr<idMapBrushSide>> sides;
};

class idMapEntity {
	friend class			idMapFile;

public:
	idDict					epairs;

public:
	idMapEntity() {  }
	~idMapEntity() { primitives.clear(); }
	static std::shared_ptr<idMapEntity> Parse(idLexer& src, bool worldSpawn = false, float version = CURRENT_MAP_VERSION);
	//bool					Write(idFile* fp, int entityNum) const;
	int						GetNumPrimitives() const { return primitives.size(); }
	std::shared_ptr<idMapPrimitive> GetPrimitive(int i) const { return primitives[i]; }
	void					AddPrimitive(std::shared_ptr<idMapPrimitive> p) { primitives.push_back(p); }
	unsigned int			GetGeometryCRC() const;
	void					RemovePrimitiveData();

protected:
	std::vector<std::shared_ptr<idMapPrimitive>> primitives;
};

class idMapFile {
public:
	idMapFile();
	~idMapFile() { entities.clear(); }

	// filename does not require an extension
	// normally this will use a .reg file instead of a .map file if it exists,
	// which is what the game and dmap want, but the editor will want to always
	// load a .map file
	bool					Parse(const std::string& filename, bool ignoreRegion = false, bool osPath = false);
	bool					Write(const char* fileName, const char* ext, bool fromBasePath = true);
	// get the number of entities in the map
	int						GetNumEntities() const { return entities.size(); }
	// get the specified entity
	std::shared_ptr<idMapEntity> GetEntity(int i) const { return entities[i]; }
	// get the name without file extension
	const std::string& GetName() const { return name; }
	// get the file time
	ID_TIME_T					GetFileTime() const { return fileTime; }
	// get CRC for the map geometry
	// texture coordinates and entity key/value pairs are not taken into account
	//unsigned int			GetGeometryCRC() const { return geometryCRC; }
	// returns true if the file on disk changed
	bool					NeedsReload();

	int						AddEntity(std::shared_ptr<idMapEntity> mapentity);
	std::shared_ptr<idMapEntity> FindEntity(const std::string& name);
	void					RemoveEntity(std::shared_ptr<idMapEntity> mapEnt);
	void					RemoveEntities(const std::string& classname);
	void					RemoveAllEntities();
	void					RemovePrimitiveData();
	//bool					HasPrimitiveData() { return hasPrimitiveData; }

protected:
	float					version;
	ID_TIME_T				fileTime;
	//unsigned int			geometryCRC;
	std::vector<std::shared_ptr<idMapEntity>>	entities;
	std::string name;
	//bool					hasPrimitiveData;

private:
	void					SetGeometryCRC();
};

inline idMapFile::idMapFile() {
	version = CURRENT_MAP_VERSION;
	fileTime = 0;
	//geometryCRC = 0;
	entities.resize(1024);
	//hasPrimitiveData = false;
}

#endif // !IDLIB_MAPFILE_H_
