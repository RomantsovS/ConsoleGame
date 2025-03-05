#include "idlib/precompiled.h"

#include "../Game_local.h"

/*
========================
idMenuWidget_MenuBar::Initialize
========================
*/
void idMenuWidget_MenuBar::Initialize(
    std::shared_ptr<idMenuHandler> data) noexcept {
  idMenuWidget::Initialize(data);
}

/*
========================
idMenuWidget_MenuBar::Update
========================
*/
void idMenuWidget_MenuBar::Update() noexcept {
  if (!GetSWFObject()) {
    return;
  }

  std::shared_ptr<idSWFScriptObject> root = GetSWFObject()->GetRootObject();

  if (!BindSprite(root.get())) {
    return;
  }

  totalWidth = 0.0f;
  buttonPos = 0.0f;

  for (size_t index = 0; index < GetNumVisibleOptions(); ++index) {
    if (index >= children.size()) {
      break;
    }

    if (index != 0) {
      totalWidth += rightSpacer;
    }

    std::shared_ptr<idMenuWidget> child = GetChildByIndex(index);
    child->SetSpritePath(GetSpritePath(), va("btn%d", index).c_str());
    if (child->BindSprite(root.get())) {
      PrepareListElement(child, index);
      child->Update();
    }
  }

  // 640 is half the size of our flash files width
  float xPos = 640.0f - (totalWidth / 2.0f);
  GetSprite()->SetXPos(xPos);
}

/*
========================
idMenuWidget_MenuBar::SetListHeadings
========================
*/
void idMenuWidget_MenuBar::SetListHeadings(std::vector<std::string>& list) {
  headings.clear();
  for (size_t index = 0; index < list.size(); ++index) {
    headings.push_back(list[index]);
  }
}

/*
========================
idMenuWidget_MenuBar::GetTotalNumberOfOptions
========================
*/
size_t idMenuWidget_MenuBar::GetTotalNumberOfOptions() const noexcept {
  // return GetChildren().size();
  return headings.size();
}

/*
========================
idMenuWidget_MenuBar::PrepareListElement
========================
*/
bool idMenuWidget_MenuBar::PrepareListElement(
    std::shared_ptr<idMenuWidget> widget, const size_t navIndex) {
  if (navIndex >= GetNumVisibleOptions()) {
    return false;
  }

  std::shared_ptr<idMenuWidget_MenuButton> const button =
      std::dynamic_pointer_cast<idMenuWidget_MenuButton>(widget);
  if (!button || !button->GetSprite()) {
    return false;
  }

  if (navIndex >= headings.size()) {
    button->SetLabel("");
  } else {
    button->SetLabel(headings[navIndex]);
    std::shared_ptr<idSWFTextInstance> ti =
        button->GetSprite()->GetScriptObject()->GetNestedText("txtVal");
    if (ti) {
      // ti->SetStrokeInfo(true, 0.7f, 1.25f);
      ti->SetText(headings[navIndex]);
      button->SetPosition(buttonPos);
      totalWidth += ti->GetTextLength();
      buttonPos += rightSpacer + ti->GetTextLength();
    }
  }

  return true;
}
