#pragma once
#include "GuiComponent.h"
#include <string>
class GuiInfoPopup : public GuiComponent {
public:
	GuiInfoPopup(Window* window, std::string message, int duration = 3000, int fadein = 250, int fadeout = 250);
	void render() {}
	void update(int deltaTime) {}
private:
	std::string mMessage;
	int mDuration;
};
