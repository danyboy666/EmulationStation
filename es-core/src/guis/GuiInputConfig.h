#pragma once
#include "GuiComponent.h"
#include "components/ComponentList.h"
#include "InputConfig.h"
#include <memory>
#include <string>
#include <vector>
class TextComponent;
class ButtonComponent;
class GuiInputConfig : public GuiComponent
{
public:
	GuiInputConfig(Window* window, InputConfig* target, bool reconfigureAll, const std::function<void()>& okCallback);
	~GuiInputConfig();
	bool input(InputConfig* config, Input input);
	void update(int deltaTime);
	std::vector<HelpPrompt> getHelpPrompts() { return std::vector<HelpPrompt>(); }
private:
	void rowDone();
	void setPress(const std::shared_ptr<TextComponent>& text);
	void setNotDefined(const std::shared_ptr<TextComponent>& text);
	void setAssignedTo(const std::shared_ptr<TextComponent>& text, Input input);
	bool assign(Input input, int inputId);
	InputConfig* mTargetConfig;
	bool mHoldingInput, mConfiguringAll, mConfiguringRow;
	Input mHeldInput;
	int mHeldTime, mHeldInputId;
	std::shared_ptr<ComponentList> mList;
	std::shared_ptr<TextComponent> mSubtitle2;
	std::vector<std::shared_ptr<TextComponent>> mMappings;
};
