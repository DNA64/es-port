
#pragma once
#ifndef ES_APP_VIEWS_GAME_LIST_ISIMPLE_GAME_LIST_VIEW_H
#define ES_APP_VIEWS_GAME_LIST_ISIMPLE_GAME_LIST_VIEW_H

#include "components/HeaderStatus.h"
#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "guis/GuiSuspendPoints.h"
#include "views/gamelist/IGameListView.h"
#include "SuspendPoints.h"
#include <stack>
#include <string>

class ISimpleGameListView : public IGameListView
{
public:
	ISimpleGameListView(Window* window, FileData* root);
	virtual ~ISimpleGameListView();

	// Called when a new file is added, a file is removed, a file's metadata changes, or a file's children are sorted.
	// NOTE: FILE_SORTED is only reported for the topmost FileData, where the sort started.
	//       Since sorts are recursive, that FileData's children probably changed too.
	virtual void onFileChanged(FileData* file, FileChangeType change) override;

	// Called whenever the theme changes.
	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

	virtual FileData* getCursor() override = 0;
	virtual void setCursor(FileData*, bool refreshListCursorPos = false) override = 0;
	virtual int getViewportTop() override = 0;
	virtual void setViewportTop(int index) override = 0;

	virtual bool input(InputConfig* config, Input input) override;
	virtual void launch(FileData* game) override = 0;
	virtual void onShow() override;

	virtual bool openSuspendPoints() override;
	virtual bool supportsSuspendPoints() const override { return true; }

protected:
	static const int DESCRIPTION_SCROLL_DELAY = 5 * 1000; // five secs

	virtual std::string getQuickSystemSelectRightButton() = 0;
	virtual std::string getQuickSystemSelectLeftButton() = 0;
	virtual void populateList(const std::vector<FileData*>& files) = 0;

	TextComponent mHeaderText;
	ImageComponent mHeaderImage;
	ImageComponent mBackground;
	HeaderStatus mHeaderStatus;
	bool mDefaultHeaderStatus = false;

	std::vector<GuiComponent*> mThemeExtras;

	std::stack<FileData*> mCursorStack;

	void refreshFloating();
	virtual void onFloatingChanged() {}
	bool isFloating(const FileData* file) const;
	bool jumpToFloating();
	bool canJumpTo(const FileData* game);
	void jumpTo(FileData* game);
	FileData* findGame(const std::string& code) const;
	void applyPendingResume();

	SuspendPoints mSuspendPoints;
	SuspendPoint mFloating;
	GuiSuspendPoints::Pad mPad;
	int mPendingResume = 0;
};

#endif // ES_APP_VIEWS_GAME_LIST_ISIMPLE_GAME_LIST_VIEW_H
