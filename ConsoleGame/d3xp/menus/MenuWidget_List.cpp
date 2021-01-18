#pragma hdrstop
#include "../../idLib/precompiled.h"
#include "../Game_local.h"

/*
========================
idMenuWidget_List::Update
========================
*/
void idMenuWidget_List::Update() {

	if (!GetSWFObject()) {
		return;
	}

	std::shared_ptr<idSWFScriptObject> root = GetSWFObject()->GetRootObject();

	if (!BindSprite(root)) {
		return;
	}

	for (size_t optionIndex = 0; optionIndex < GetNumVisibleOptions(); ++optionIndex) {
		const int childIndex = GetViewOffset() + optionIndex;
		bool shown = false;

		if (optionIndex < GetChildren().size()) {
			std::shared_ptr<idMenuWidget> child = GetChildByIndex(optionIndex);
			const int controlIndex = GetNumVisibleOptions() - min(GetNumVisibleOptions(), GetTotalNumberOfOptions()) + optionIndex;
			child->SetSpritePath(GetSpritePath(), va("item%d", controlIndex).c_str());
			if (child->BindSprite(root)) {
				//PrepareListElement(child, childIndex);
				child->Update();
				shown = true;
			}
		}

		if (!shown) {
			// hide the item
			std::shared_ptr<idSWFSpriteInstance> const sprite = GetSprite()->GetScriptObject()->GetSprite(va("item%d", optionIndex - GetTotalNumberOfOptions()));
			if (sprite) {
				sprite->SetVisible(false);
			}
		}
	}
}

/*
========================
idMenuWidget_List::HandleAction
========================
*/
bool idMenuWidget_List::HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget> widget, bool forceHandled) {

	const idSWFParmList& parms = action.GetParms();

	/*if (action.GetType() == WIDGET_ACTION_SCROLL_VERTICAL) {
		const scrollType_t scrollType = static_cast<scrollType_t>(event.arg);
		if (scrollType == SCROLL_SINGLE) {
			Scroll(parms[0].ToInteger());
		}
		else if (scrollType == SCROLL_PAGE) {
			ScrollOffset(parms[0].ToInteger() * (GetNumVisibleOptions() - 1));
		}
		else if (scrollType == SCROLL_FULL) {
			ScrollOffset(parms[0].ToInteger() * 999);
		}
		return true;
	}*/

	return idMenuWidget::HandleAction(action, event, widget, forceHandled);
}
