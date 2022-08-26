#include "idlib/precompiled.h"
#include "../Game_local.h"

/*
========================
idMenuWidget_LobbyList::Update
========================
*/
void idMenuWidget_LobbyList::Update() noexcept {

	if (!GetSWFObject()) {
		return;
	}

	std::shared_ptr<idSWFScriptObject> root = GetSWFObject()->GetRootObject();

	if (!BindSprite(root.get())) {
		return;
	}

	for (int i = 0; i < headings.size(); ++i) {
		std::shared_ptr<idSWFTextInstance> txtHeading = GetSprite()->GetScriptObject()->GetNestedText(va("heading%d", i).c_str());
		if (txtHeading) {
			txtHeading->SetText(headings[i]);
		}
	}

	for (int optionIndex = 0; optionIndex < GetNumVisibleOptions(); ++optionIndex) {
		bool shown = false;
		if (optionIndex < GetChildren().size()) {
			std::shared_ptr<idMenuWidget> child = GetChildByIndex(optionIndex);
			child->SetSpritePath(GetSpritePath(), va("item%d", optionIndex).c_str());
			if (child->BindSprite(root.get())) {
				shown = PrepareListElement(child.get(), optionIndex);
				if (shown) {
					child->GetSprite()->SetVisible(true);
					child->Update();
				}
				else {
					child->GetSprite()->SetVisible(false);
				}
			}
		}
	}
}

/*
========================
idMenuWidget_LobbyList::PrepareListElement
========================
*/
bool idMenuWidget_LobbyList::PrepareListElement(idMenuWidget* widget, const int childIndex) {

	idMenuWidget_LobbyButton* button = dynamic_cast<idMenuWidget_LobbyButton*>(widget);
	if (button == nullptr) {
		return false;
	}

	if (!button->IsValid()) {
		return false;
	}

	return true;

}