#include "idlib/precompiled.h"

#include "../Game_local.h"

MeshAnim::MeshAnim() {
  numFrames = 0;
  frameRate = 24;
  animLength = 0;
}

MeshAnim::~MeshAnim() { Free(); }

void MeshAnim::Free() {
  numFrames = 0;
  frameRate = 24;
  animLength = 0;
}

int MeshAnim::NumFrames() const { return numFrames; }

bool MeshAnim::LoadAnim(const std::string& filename) {
  std::string extension;
  idStr::ExtractFileExtension(filename, extension);
  if (extension != "meshanim") {
    return false;
  }

  idLexer parser;
  idToken token;

  if (!parser.LoadFile(filename)) {
    return false;
  }

  name = filename;

  Free();

  parser.ExpectTokenString("numFrames");
  numFrames = parser.ParseInt();
  if (numFrames <= 0) {
    parser.Error("Invalid number of frames: %d", numFrames);
  }

  // parse frame rate
  parser.ExpectTokenString("frameRate");
  frameRate = parser.ParseInt();
  if (frameRate < 0) {
    parser.Error("Invalid frame rate: %d", frameRate);
  }

  // parse base frame
  parser.ExpectTokenString("baseframe");
  parser.ExpectTokenString("{");
  baseFrame.x = parser.ParseInt();
  baseFrame.y = parser.ParseInt();
  parser.ExpectTokenString("}");

  // parse frames
  componentFrames.resize(numFrames);

  for (int i = 0; i < numFrames; i++) {
    parser.ExpectTokenString("frame");
    int num = parser.ParseInt();
    if (num != i) {
      parser.Error("Expected frame number %d", i);
    }
    parser.ExpectTokenString("{");
    componentFrames[i].x = parser.ParseInt();
    componentFrames[i].y = parser.ParseInt();
    parser.ExpectTokenString("}");
  }

  return true;
}

void MeshAnim::GetSingleFrame(int framenum, Vector2& text_coords) const {
  // copy the baseframe
  text_coords = baseFrame;

  if (framenum == 0) {
    // just use the base frame
    return;
  }

  text_coords = componentFrames.at(framenum);
}
