#pragma once
#ifndef ES_APP_VIEWS_GAME_LIST_GRID_GAME_LIST_VIEW_H
#define ES_APP_VIEWS_GAME_LIST_GRID_GAME_LIST_VIEW_H

#include "components/DateTimeComponent.h"
#include "components/MenuIconBar.h"
#include "components/DiskSpaceIndicator.h"
#include "components/RatingComponent.h"
#include "components/PagedTextComponent.h"
#include "components/ScrollableContainer.h"
#include "components/ImageGridComponent.h"
#include "components/MarqueeTextComponent.h"
#include "components/VideoComponent.h"
#include "guis/GuiSuspendPoints.h"
#include "views/gamelist/ISimpleGameListView.h"
#include "SuspendPoints.h"
#include <memory>

class GridGameListView : public ISimpleGameListView
{
public:
	GridGameListView(Window* window, FileData* root);
	virtual ~GridGameListView();

	virtual void onShow() override;

	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

	virtual FileData* getCursor() override;
	virtual void setCursor(FileData*, bool refreshListCursorPos = false) override;
	virtual void setViewportTop(int index) override { ; }
	virtual int getViewportTop() override { return -1; }

	virtual bool input(InputConfig* config, Input input) override;

	virtual const char* getName() const override { return "grid"; }

	virtual std::vector<HelpPrompt> getHelpPrompts() override;
	virtual void launch(FileData* game) override;

	void onFocusLost(void) override;

protected:
	virtual void update(int deltaTime) override;
	virtual std::string getQuickSystemSelectRightButton() override;
	virtual std::string getQuickSystemSelectLeftButton() override;
	virtual void populateList(const std::vector<FileData*>& files) override;
	virtual void remove(FileData* game, bool deleteFile, bool refreshView=true) override;
	virtual void addPlaceholder();

	ImageGridComponent<FileData*> mGrid;

private:
	void updateInfoPanel();
	const std::string getImagePath(FileData* file);

	void initMDLabels();
	void initMDValues();

	void refreshFloating();
	void setHeaderFocus(bool focus);
	void openHeaderMenu();
	bool openSuspendPoints();
	bool canJumpTo(const FileData* game) const;
	void jumpTo(FileData* game);
	void cycleSort();
	FileData* findGame(const std::string& code) const;

	TextComponent mLblRating, mLblReleaseDate, mLblDeveloper, mLblPublisher, mLblGenre, mLblPlayers, mLblLastPlayed, mLblPlayCount;

	ImageComponent mMarquee;
	VideoComponent* mVideo;
	bool mVideoPlaying;
	ImageComponent mImage;
	RatingComponent mRating;
	DateTimeComponent mReleaseDate;
	TextComponent mDeveloper;
	TextComponent mPublisher;
	TextComponent mGenre;
	TextComponent mPlayers;
	DateTimeComponent mLastPlayed;
	TextComponent mPlayCount;
	MarqueeTextComponent mName;

	std::vector<TextComponent*> getMDLabels();
	std::vector<GuiComponent*> getMDValues();

	PagedTextComponent mDescription;

	SuspendPoints mSuspendPoints;
	MenuIconBar mIconBar;
	DiskSpaceIndicator mDiskSpace;
	DiskSpaceIndicator mNandSpace;
	bool mHeaderFocus = false;
	std::shared_ptr<bool> mAlive = std::make_shared<bool>(true);
	SuspendPoint mFloating;
	GuiSuspendPoints::Pad mPad;
	int mPendingResume = 0;
};

#endif // ES_APP_VIEWS_GAME_LIST_GRID_GAME_LIST_VIEW_H
