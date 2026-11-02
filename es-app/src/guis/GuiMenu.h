#pragma once
#ifndef ES_APP_GUIS_GUI_MENU_H
#define ES_APP_GUIS_GUI_MENU_H

#include "components/MenuIconBar.h"
#include "components/MenuComponent.h"
#include "GuiComponent.h"
#include "components/OptionListComponent.h"
#include "FileData.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class GuiMenu : public GuiComponent
{
public:
	GuiMenu(Window* window, int category = 0, std::function<void(int)> onClose = nullptr);
	~GuiMenu() override;

	bool input(InputConfig* config, Input input) override;
	void onSizeChanged() override;
	void render(const Transform4x4f& parentTrans) override;
	std::vector<HelpPrompt> getHelpPrompts() override;
	HelpStyle getHelpStyle() override;

private:
	enum Category { Display, System, Network, Library, Help, CategoryCount };

	void showCategory(int category);
	void addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func);
	void addVersionInfo();
	void addToggle(const char* name, const std::string& stateArguments, const std::string& toggleArguments);
	void showToolResult(const std::string& arguments);
	void openController();
	void openNetworkList();
	static bool isFamicomPad();
	void openAbout();
	void openCollectionSystemSettings();
	void openConfigInput();
	void openOtherSettings();
	void openQuitMenu();
	void openScraperSettings();
	void openScreensaverOptions();
	void openSoundSettings();
	void openUISettings();

	MenuComponent mMenu;
	TextComponent mVersion;
	MenuIconBar mBar;
	std::function<void(int)> mOnClose;
	int mCategory = Display;
	bool mFullUI = true;

	typedef OptionListComponent<const FileData::SortType*> SortList;
	std::shared_ptr<SortList> mListSort;
};

#endif // ES_APP_GUIS_GUI_MENU_H
