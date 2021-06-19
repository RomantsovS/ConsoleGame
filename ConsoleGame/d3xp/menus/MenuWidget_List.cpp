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

	if (action.GetType() == widgetAction_t::WIDGET_ACTION_SCROLL_VERTICAL) {
		const scrollType_t scrollType = static_cast<scrollType_t>(event.arg);
		if (scrollType == scrollType_t::SCROLL_SINGLE) {
			Scroll(parms[0]->ToInteger());
		}
		else if (scrollType == scrollType_t::SCROLL_PAGE) {
			ScrollOffset(parms[0]->ToInteger() * (GetNumVisibleOptions() - 1));
		}
		else if (scrollType == scrollType_t::SCROLL_FULL) {
			ScrollOffset(parms[0]->ToInteger() * 999);
		}
		return true;
	}

	return idMenuWidget::HandleAction(action, event, widget, forceHandled);
}

/*
========================
idMenuWidget_List::CalculatePositionFromIndexDelta

Pure functional encapsulation of how to calculate a new index and offset based on how the user
chose to move through the list.
========================
*/
void idMenuWidget_List::CalculatePositionFromIndexDelta(int& outIndex, int& outOffset, const int currentIndex, const int currentOffset, const int windowSize, const int maxSize, const int indexDelta, const bool allowWrapping, const bool wrapAround) const {
	assert(indexDelta != 0);

	int newIndex = currentIndex + indexDelta;
	bool wrapped = false;

	if (indexDelta > 0) {
		// moving down the list
		if (newIndex > maxSize - 1) {
			if (allowWrapping) {
				if (wrapAround) {
					wrapped = true;
					newIndex = 0 + (newIndex - maxSize);
				}
				else {
					newIndex = 0;
				}
			}
			else {
				newIndex = maxSize - 1;
			}
		}
	}
	else {
		// moving up the list
		if (newIndex < 0) {
			if (allowWrapping) {
				if (wrapAround) {
					newIndex = maxSize + newIndex;
				}
				else {
					newIndex = maxSize - 1;
				}
			}
			else {
				newIndex = 0;
			}
		}
	}

	// calculate the offset
	if (newIndex - currentOffset >= windowSize) {
		outOffset = newIndex - windowSize + 1;
	}
	else if (currentOffset > newIndex) {
		if (wrapped) {
			outOffset = 0;
		}
		else {
			outOffset = newIndex;
		}
	}
	else {
		outOffset = currentOffset;
	}

	outIndex = newIndex;

	// the intended behavior is that outOffset and outIndex are always within maxSize of each
	// other, as they are meant to model a window of items that should be visible in the list.
	assert(outIndex - outOffset < windowSize);
	assert(outIndex >= outOffset && outIndex >= 0 && outOffset >= 0);
}

/*
========================
idMenuWidget_List::CalculatePositionFromOffsetDelta
========================
*/
void idMenuWidget_List::CalculatePositionFromOffsetDelta(int& outIndex, int& outOffset, const int currentIndex, const int currentOffset, const int windowSize, const int maxSize, const int offsetDelta) const {
	// shouldn't be setting both indexDelta AND offsetDelta
	// FIXME: make this simpler code - just pass a boolean to control it?
	assert(offsetDelta != 0);

	const int newOffset = max(currentIndex + offsetDelta, 0);

	if (newOffset >= maxSize) {
		// scrolling past the end - just scroll all the way to the end
		outIndex = maxSize - 1;
		outOffset = max(maxSize - windowSize, 0);
	}
	else if (newOffset >= maxSize - windowSize) {
		// scrolled to the last window
		outIndex = newOffset;
		outOffset = max(maxSize - windowSize, 0);
	}
	else {
		outIndex = outOffset = newOffset;
	}

	// the intended behavior is that outOffset and outIndex are always within maxSize of each
	// other, as they are meant to model a window of items that should be visible in the list.
	assert(outIndex - outOffset < windowSize);
	assert(outIndex >= outOffset && outIndex >= 0 && outOffset >= 0);
}

/*
========================
idMenuWidget_List::Scroll
========================
*/
void idMenuWidget_List::Scroll(const int scrollAmount, const bool wrapAround) {

	if (GetTotalNumberOfOptions() == 0) {
		return;
	}

	int newIndex, newOffset;

	CalculatePositionFromIndexDelta(newIndex, newOffset, GetViewIndex(), GetViewOffset(), GetNumVisibleOptions(), GetTotalNumberOfOptions(), scrollAmount, IsWrappingAllowed(), wrapAround);
	if (newOffset != GetViewOffset()) {
		SetViewOffset(newOffset);
		if (auto spMenuData = menuData.lock()) {
			//menuData->PlaySound(GUI_SOUND_FOCUS);
		}
		Update();
	}

	if (newIndex != GetViewIndex()) {
		SetViewIndex(newIndex);
		SetFocusIndex(newIndex - newOffset);
	}
}

/*
========================
idMenuWidget_List::ScrollOffset
========================
*/
void idMenuWidget_List::ScrollOffset(const int scrollAmount) {

	if (GetTotalNumberOfOptions() == 0) {
		return;
	}

	int newIndex, newOffset;

	CalculatePositionFromOffsetDelta(newIndex, newOffset, GetViewIndex(), GetViewOffset(), GetNumVisibleOptions(), GetTotalNumberOfOptions(), scrollAmount);
	if (newOffset != GetViewOffset()) {
		SetViewOffset(newOffset);
		Update();
	}

	if (newIndex != GetViewIndex()) {
		SetViewIndex(newIndex);
		SetFocusIndex(newIndex - newOffset);
	}
}
