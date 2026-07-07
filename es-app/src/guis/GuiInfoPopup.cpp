#include "guis/GuiInfoPopup.h"
#include "Renderer.h"
#include "Window.h"

GuiInfoPopup::GuiInfoPopup(Window* window, std::string message, int duration, int fadein, int fadeout)
	: GuiComponent(window), mMessage(message), mDuration(duration)
{
	setSize(Renderer::getScreenWidth() * 0.6f, Font::get(FONT_SIZE_SMALL)->getLetterHeight() * 2.0f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.05f);
}
