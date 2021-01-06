#include "MenuHandler.h"
#include "MenuWidget.h"
#include "../../renderer/RenderSystem.h"

/*
================================================
idMenuHandler::~idMenuHandler
================================================
*/
idMenuHandler::idMenuHandler() :
	activeScreen(-1),
	nextScreen(-1),
	gui(nullptr),
	cmdBar(nullptr) {
	menuScreens.resize(MAX_SCREEN_AREAS);
}

/*
================================================
idMenuHandler::~idMenuHandler
================================================
*/
idMenuHandler::~idMenuHandler() {
	Cleanup();
}

/*
================================================
idMenuHandler::Initialize
================================================
*/
void idMenuHandler::Initialize(const std::string& filename) {
	Cleanup();
	gui = std::make_shared<GUI>(filename);
	gui->Init();
}

/*
================================================
idMenuHandler::AddChild
================================================
*/
void idMenuHandler::AddChild(std::shared_ptr<idMenuWidget> widget) {
	widget->SetSWFObj(gui);
	//widget->SetHandlerIsParent(true);
	children.push_back(widget);
}

/*
================================================
idMenuHandler::Cleanup
================================================
*/
void idMenuHandler::Cleanup() {
	for (size_t index = 0; index < children.size(); ++index) {
		children[index] = nullptr;
	}
	children.clear();

	for (int index = 0; index < MAX_SCREEN_AREAS; ++index) {
		if (menuScreens[index]) {
			menuScreens[index] = nullptr;
		}
	}

	gui = nullptr;
}

/*
================================================
idMenuHandler::IsActive
================================================
*/
bool idMenuHandler::IsActive() {
	if (!gui) {
		return false;
	}

	return gui->IsActive();
}

/*
================================================
idMenuHandler::ActivateMenu
================================================
*/
void idMenuHandler::ActivateMenu(bool show) {

	if (!show) {
		gui->Activate(show);
		return;
	}

	gui->Activate(show);
}

/*
================================================
idMenuHandler::Update
================================================
*/
void idMenuHandler::Update() {
	if (gui && gui->IsActive()) {
		gui->Render(renderSystem, Sys_Milliseconds());
	}
}

/*
================================================
idMenuHandler::UpdateChildren
================================================
*/
void idMenuHandler::UpdateChildren() {
	for (size_t index = 0; index < children.size(); ++index) {
		if (children[index]) {
			children[index]->Update();
		}
	}
}

/*
================================================
idMenuHandler::HandleGuiEvent
================================================
*/
bool idMenuHandler::HandleGuiEvent(const sysEvent_t* sev) {

	if (gui && activeScreen != -1) {
		return gui->HandleEvent(sev);
	}

	return false;
}
