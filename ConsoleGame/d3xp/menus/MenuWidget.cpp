#include "MenuWidget.h"

/*
========================
idMenuWidget::idMenuWidget
========================
*/
idMenuWidget::idMenuWidget() :
	menuData(nullptr),
	guiObj(nullptr) {
	parent.reset();
}

/*
========================
idMenuWidget::~idMenuWidget
========================
*/
idMenuWidget::~idMenuWidget() {
	Cleanup();
}

void idMenuWidget::Cleanup() {
	// free all children
	for (size_t i = 0; i < children.size(); ++i) {
		children[i] = nullptr;
	}

	children.clear();
}

/*
========================
idMenuWidget::AddChild
========================
*/
void idMenuWidget::AddChild(std::shared_ptr<idMenuWidget> widget) {
	if (widget->GetParent() != NULL) {
		// take out of previous parent
		widget->GetParent()->RemoveChild(widget);
	}

	widget->SetParent(shared_from_this());
	children.push_back(widget);
}

/*
========================
idMenuWidget::RemoveChild
========================
*/
void idMenuWidget::RemoveChild(std::shared_ptr<idMenuWidget> widget) {
	children.erase(std::remove(children.begin(), children.end(), widget), children.end());
	
	widget->SetParent(nullptr);
	widget = nullptr;
}

/*
========================
idMenuWidget::GetSWFObject
========================
*/
std::shared_ptr<GUI> idMenuWidget::GetGUIObject() {

	if (guiObj) {
		return guiObj;
	}

	/*if (parent != NULL) {
		return parent->GetSWFObject();
	}*/

	if (menuData != NULL) {
		return menuData->GetGUI();
	}

	return nullptr;
}

/*
========================
idMenuWidget::BindSprite

Takes the sprite path strings and resolves it to an actual sprite relative to a given root.

This is setup in this manner, because we can't resolve from path -> sprite immediately since
SWFs aren't necessarily loaded at the time widgets are instantiated.
========================
*/
bool idMenuWidget::BindSprite(std::shared_ptr<GUIScriptObject> root) {
	boundSprite = root->GetNestedSprite();
	return boundSprite != nullptr;
}

