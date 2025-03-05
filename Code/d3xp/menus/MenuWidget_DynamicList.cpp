
#include "idlib/precompiled.h"
#include "../Game_local.h"

/*
========================
idMenuWidget_DynamicList::Initialize
========================
*/
void idMenuWidget_DynamicList::Initialize(
    std::shared_ptr<idMenuHandler> data) noexcept {
  idMenuWidget::Initialize(data);
}

/*
========================
idMenuWidget_DynamicList::Update
========================
*/
void idMenuWidget_DynamicList::Update() noexcept {
  if (!GetSWFObject()) {
    return;
  }

  std::shared_ptr<idSWFScriptObject> root = GetSWFObject()->GetRootObject();

  if (!BindSprite(root.get())) {
    return;
  }

  for (size_t optionIndex = 0; optionIndex < GetNumVisibleOptions();
       ++optionIndex) {
    if (optionIndex >= children.size()) {
      std::shared_ptr<idSWFSpriteInstance> item =
          GetSprite()->GetScriptObject()->GetNestedSprite(
              va("item%d", optionIndex).c_str());
      if (item) {
        item->SetVisible(false);
        continue;
      }
    }

    std::shared_ptr<idMenuWidget> child = GetChildByIndex(optionIndex);
    const int childIndex = GetViewOffset() + optionIndex;
    bool shown = false;
    child->SetSpritePath(GetSpritePath(), va("item%d", optionIndex).c_str());
    if (child->BindSprite(root.get())) {
      if (optionIndex >= GetTotalNumberOfOptions()) {
        child->ClearSprite();
        continue;
      } else {
        shown = PrepareListElement(child, childIndex);
        child->Update();
      }

      if (!shown) {
        child->SetState(widgetState_t::WIDGET_STATE_HIDDEN);
      } else {
        if (optionIndex == focusIndex) {
          child->SetState(widgetState_t::WIDGET_STATE_SELECTING);
        } else {
          child->SetState(widgetState_t::WIDGET_STATE_NORMAL);
        }
      }
    }
  }
}

/*
========================
idMenuWidget_DynamicList::GetTotalNumberOfOptions
========================
*/
size_t idMenuWidget_DynamicList::GetTotalNumberOfOptions() const noexcept {
  if (controlList) {
    return GetChildren().size();
  }

  return listItemInfo.size();
}

/*
========================
idMenuWidget_DynamicList::PrepareListElement
========================
*/
bool idMenuWidget_DynamicList::PrepareListElement(
    std::shared_ptr<idMenuWidget> widget, const size_t childIndex) {
  if (listItemInfo.size() == 0) {
    return true;
  }

  if (childIndex > listItemInfo.size()) {
    return false;
  }

  std::shared_ptr<idMenuWidget_Button> const button =
      std::dynamic_pointer_cast<idMenuWidget_Button>(widget);
  if (button) {
    // button->SetIgnoreColor(ignoreColor);
    button->SetValues(listItemInfo[childIndex]);
    if (listItemInfo[childIndex].size() > 0) {
      return true;
    }
  }

  return false;
}

/*
========================
idMenuWidget_DynamicList::SetListData
========================
*/
void idMenuWidget_DynamicList::SetListData(
    std::vector<std::vector<std::string>>& list) {
  listItemInfo.clear();
  for (size_t i = 0; i < list.size(); ++i) {
    std::vector<std::string> values;
    for (size_t j = 0; j < list[i].size(); ++j) {
      values.push_back(list[i][j]);
    }
    listItemInfo.push_back(values);
  }
}
