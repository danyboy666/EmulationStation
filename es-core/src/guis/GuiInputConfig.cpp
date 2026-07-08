#include "guis/GuiInputConfig.h"
#include "guis/GuiMsgBox.h"
#include "components/TextComponent.h"
#include "components/ButtonComponent.h"
#include "InputManager.h"
#include "Renderer.h"
#include "Log.h"
#include "Window.h"
#include "Util.h"
#include <sstream>

static const int inputCount = 25;
static const char* inputName[inputCount] = { "Up","Down","Left","Right","Start","Select","A","B","X","Y","LeftShoulder","RightShoulder","LeftTrigger","RightTrigger","LeftThumb","RightThumb","LeftAnalogUp","LeftAnalogDown","LeftAnalogLeft","LeftAnalogRight","RightAnalogUp","RightAnalogDown","RightAnalogLeft","RightAnalogRight","HotKeyEnable" };
static const bool inputSkippable[inputCount] = { false,false,false,false,true,true,false,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true };
static const char* inputDispName[inputCount] = { "D-PAD UP","D-PAD DOWN","D-PAD LEFT","D-PAD RIGHT","START","SELECT","BUTTON A / EAST","BUTTON B / SOUTH","BUTTON X / NORTH","BUTTON Y / WEST","LEFT SHOULDER","RIGHT SHOULDER","LEFT TRIGGER","RIGHT TRIGGER","LEFT THUMB","RIGHT THUMB","LEFT ANALOG UP","LEFT ANALOG DOWN","LEFT ANALOG LEFT","LEFT ANALOG RIGHT","RIGHT ANALOG UP","RIGHT ANALOG DOWN","RIGHT ANALOG LEFT","RIGHT ANALOG RIGHT","HOTKEY" };
static const char* inputIcon[inputCount] = { ":/help/dpad_up.svg",":/help/dpad_down.svg",":/help/dpad_left.svg",":/help/dpad_right.svg",":/help/button_start.svg",":/help/button_select.svg",":/help/button_a.svg",":/help/button_b.svg",":/help/button_x.svg",":/help/button_y.svg",":/help/button_l.svg",":/help/button_r.svg",":/help/button_l.svg",":/help/button_r.svg",":/help/button_select.svg",":/help/button_select.svg",":/help/dpad_up.svg",":/help/dpad_down.svg",":/help/dpad_left.svg",":/help/dpad_right.svg",":/help/dpad_up.svg",":/help/dpad_down.svg",":/help/dpad_left.svg",":/help/dpad_right.svg",":/help/button_select.svg" };
#define HOLD_TO_SKIP_MS 1000

GuiInputConfig::GuiInputConfig(Window* window, InputConfig* target, bool reconfigureAll, const std::function<void()>& okCallback) : GuiComponent(window), mTargetConfig(target), mHoldingInput(false)
{
	LOG(LogInfo) << "Configuring device " << target->getDeviceId() << " (" << target->getDeviceName() << ").";
	if(reconfigureAll) target->clear();
	mConfiguringAll = reconfigureAll;
	mConfiguringRow = mConfiguringAll;
	mList = std::make_shared<ComponentList>(mWindow);
	mList->setSize(Renderer::getScreenWidth() * 0.6f, Renderer::getScreenHeight() * 0.7f);
	mList->setPosition((Renderer::getScreenWidth() - mList->getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
	addChild(mList.get());

	for(int i = 0; i < inputCount; i++)
	{
		ComponentListRow row;
		auto text = std::make_shared<TextComponent>(mWindow, inputDispName[i], Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
		row.addElement(text, true);
		auto mapping = std::make_shared<TextComponent>(mWindow, "-NOT DEFINED-", Font::get(FONT_SIZE_MEDIUM), 0x999999FF, ALIGN_RIGHT);
		setNotDefined(mapping);
		row.addElement(mapping, true);
		mMappings.push_back(mapping);
		row.input_handler = [this, i, mapping](InputConfig* config, Input input) -> bool {
			if(config != mTargetConfig) return false;
			if(!mConfiguringRow) { if(config->isMappedTo("a", input) && input.value) { mConfiguringRow = true; setPress(mapping); return true; } return false; }
			if(input.value != 0) { if(mHoldingInput) return true; mHoldingInput = true; mHeldInput = input; mHeldTime = 0; mHeldInputId = i; return true; }
			else { if(!mHoldingInput || mHeldInput.device != input.device || mHeldInput.id != input.id || mHeldInput.type != input.type) return true; mHoldingInput = false; if(assign(mHeldInput, i)) rowDone(); return true; }
		};
		mList->addRow(row);
	}
	if(mConfiguringAll) setPress(mMappings.front());
	mList->setCursorChangedCallback([this](CursorState state) { mSubtitle2->setOpacity(inputSkippable[mList->getCursorId()] * 255); });

	auto subtitle = std::make_shared<TextComponent>(mWindow, "HOLD TO SKIP", Font::get(FONT_SIZE_SMALL), 0x999999FF, ALIGN_CENTER);
	subtitle->setPosition((Renderer::getScreenWidth() - subtitle->getSize().x()) / 2, Renderer::getScreenHeight() * 0.85f);
	mSubtitle2 = subtitle;
	addChild(subtitle.get());

	std::vector<std::shared_ptr<ButtonComponent>> buttons;
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "OK", "ok", [this, okCallback] { InputManager::getInstance()->writeDeviceConfig(mTargetConfig); if(okCallback) okCallback(); delete this; }));
}

void GuiInputConfig::update(int deltaTime)
{
	if(mConfiguringRow && mHoldingInput && inputSkippable[mHeldInputId])
	{
		mHeldTime += deltaTime;
		if(mHeldTime >= HOLD_TO_SKIP_MS) { setNotDefined(mMappings.at(mHeldInputId)); mTargetConfig->unmapInput(inputName[mHeldInputId]); mHoldingInput = false; rowDone(); }
		else { int cur = mHeldTime / 1000; std::stringstream ss; ss << "HOLD FOR " << HOLD_TO_SKIP_MS/1000 - cur << "S TO SKIP"; mMappings.at(mHeldInputId)->setText(ss.str()); }
	}
}

void GuiInputConfig::rowDone()
{
	if(mConfiguringAll) { if(!mList->moveCursor(1)) { mConfiguringAll = false; mConfiguringRow = false; } else setPress(mMappings.at(mList->getCursorId())); }
	else mConfiguringRow = false;
}

void GuiInputConfig::setPress(const std::shared_ptr<TextComponent>& t) { t->setText("PRESS ANYTHING"); t->setColor(0x656565FF); }
void GuiInputConfig::setNotDefined(const std::shared_ptr<TextComponent>& t) { t->setText("-NOT DEFINED-"); t->setColor(0x999999FF); }
void GuiInputConfig::setAssignedTo(const std::shared_ptr<TextComponent>& t, Input input) { t->setText(strToUpper(input.string())); t->setColor(0x777777FF); }

bool GuiInputConfig::assign(Input input, int inputId)
{
	if(mTargetConfig->getMappedTo(input).size() > 0 && !mTargetConfig->isMappedTo(inputName[inputId], input)) { setAssignedTo(mMappings.at(inputId), input); mMappings.at(inputId)->setText("ALREADY TAKEN"); mMappings.at(inputId)->setColor(0x656565FF); return false; }
	setAssignedTo(mMappings.at(inputId), input); input.configured = true; mTargetConfig->mapInput(inputName[inputId], input); return true;
}
GuiInputConfig::~GuiInputConfig() {}
bool GuiInputConfig::input(InputConfig* config, Input input) { if(GuiComponent::input(config, input)) return true; if(config->isMappedTo("b", input) && input.value) { delete this; return true; } return false; }
