#include "views/gamelist/ISimpleGameListView.h"

#include "guis/GuiSuspendPoints.h"
#include "utils/FileSystemUtil.h"
#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "Scripting.h"
#include "Settings.h"
#include "Sound.h"
#include "SystemData.h"
#include "Window.h"
#include <cstdlib>

namespace
{
	GuiSuspendPoints::Pad environmentPad()
	{
		const char* pad = std::getenv("ES_PAD");
		return pad != nullptr && std::string(pad) == "nes" ? GuiSuspendPoints::Pad::Nes : GuiSuspendPoints::Pad::Snes;
	}

	std::string gameCode(const FileData* game)
	{
		return Utils::FileSystem::getStem(game->getPath());
	}
}

ISimpleGameListView::ISimpleGameListView(Window* window, FileData* root) : IGameListView(window, root),
	mHeaderText(window), mHeaderImage(window), mBackground(window), mHeaderStatus(window),
	mSuspendPoints(SuspendPoints::fromEnvironment()), mPad(environmentPad())
{
	mHeaderText.setText("Logo Text");
	mHeaderText.setSize(mSize.x(), 0);
	mHeaderText.setPosition(0, 0);
	mHeaderText.setHorizontalAlignment(ALIGN_CENTER);
	mHeaderText.setDefaultZIndex(50);

	mHeaderImage.setResize(0, mSize.y() * 0.185f);
	mHeaderImage.setOrigin(0.5f, 0.0f);
	mHeaderImage.setPosition(mSize.x() / 2, 0);
	mHeaderImage.setDefaultZIndex(50);

	mBackground.setResize(mSize.x(), mSize.y());
	mBackground.setDefaultZIndex(0);

	addChild(&mHeaderText);
	addChild(&mBackground);

	mHeaderStatus.setDefaultZIndex(70);
	mHeaderStatus.setZIndex(70);
	addChild(&mHeaderStatus);
}

void ISimpleGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	using namespace ThemeFlags;
	mBackground.applyTheme(theme, getName(), "background", ALL);
	mHeaderImage.applyTheme(theme, getName(), "logo", ALL);
	mHeaderText.applyTheme(theme, getName(), "logoText", ALL);
	mHeaderStatus.applyTheme(theme, getName(), mDefaultHeaderStatus);

	// Remove old theme extras
	for (auto extra : mThemeExtras)
	{
		removeChild(extra);
		delete extra;
	}
	mThemeExtras.clear();

	// Add new theme extras
	mThemeExtras = ThemeData::makeExtras(theme, getName(), mWindow);
	for (auto extra : mThemeExtras)
	{
		addChild(extra);
	}

	if(mHeaderImage.hasImage())
	{
		removeChild(&mHeaderText);
		addChild(&mHeaderImage);
	}else{
		addChild(&mHeaderText);
		removeChild(&mHeaderImage);
	}
}

void ISimpleGameListView::onFileChanged(FileData* /*file*/, FileChangeType /*change*/)
{
	// we could be tricky here to be efficient;
	// but this shouldn't happen very often so we'll just always repopulate
	FileData* cursor = getCursor();
	if (!cursor->isPlaceHolder()) {
		populateList(cursor->getParent()->getChildrenListToDisplay());
		setCursor(cursor);
	}
	else
	{
		populateList(mRoot->getChildrenListToDisplay());
		setCursor(cursor);
	}
}

bool ISimpleGameListView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo("b", input) && mFloating.valid && jumpToFloating())
			return true;

		if(config->isMappedTo("a", input))
		{
			FileData* cursor = getCursor();
			if(cursor->getType() == GAME)
			{
				Sound::getFromTheme(getTheme(), getName(), "launch")->play();
				launch(cursor);
			}else{
				// it's a folder
				if(cursor->getChildren().size() > 0)
				{
					mCursorStack.push(cursor);
					populateList(cursor->getChildrenListToDisplay());
					FileData* cursor = getCursor();
					setCursor(cursor);
				}
			}

			return true;
		}else if(config->isMappedTo("b", input))
		{
			if(mCursorStack.size())
			{
				populateList(mCursorStack.top()->getParent()->getChildrenListToDisplay());
				setCursor(mCursorStack.top());
				mCursorStack.pop();
				Sound::getFromTheme(getTheme(), getName(), "back")->play();
			}else{
				onFocusLost();
				SystemData* systemToView = getCursor()->getSystem();
				if (systemToView->isCollection())
				{
					systemToView = CollectionSystemManager::get()->getSystemToView(systemToView);
				}
				ViewController::get()->goToSystemView(systemToView);
			}

			return true;
		}else if(config->isMappedLike(getQuickSystemSelectRightButton(), input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				ViewController::get()->goToNextGameList();
				return true;
			}
		}else if(config->isMappedLike(getQuickSystemSelectLeftButton(), input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				ViewController::get()->goToPrevGameList();
				return true;
			}
		}else if (config->isMappedTo("x", input))
		{
			if (mRoot->getSystem()->isGameSystem())
			{
				// go to random system game
				FileData* randomGame = getCursor()->getSystem()->getRandomGame();
				if (randomGame)
				{
					setCursor(randomGame);
				}
				return true;
			}
		}else if (config->isMappedTo("y", input) && !UIModeController::getInstance()->isUIModeKid())
		{
			if(mRoot->getSystem()->isGameSystem())
			{
				if (CollectionSystemManager::get()->toggleGameInCollection(getCursor()))
				{
					return true;
				}
			}
		}
	}

	FileData* cursor = getCursor();
	SystemData* system = this->mRoot->getSystem();
    	if (system != NULL) {
            Scripting::fireEvent("game-select", system->getName(), cursor->getPath(), cursor->getName(), "input");
        }
	else
	{
	    Scripting::fireEvent("game-select", "NULL", "NULL", "NULL", "input");
	}
	return IGameListView::input(config, input);
}

void ISimpleGameListView::onShow()
{
	GuiComponent::onShow();
	refreshFloating();
}

bool ISimpleGameListView::isFloating(const FileData* file) const
{
	return mFloating.valid && file != nullptr && file->getType() == GAME && gameCode(file) == mFloating.code;
}

void ISimpleGameListView::refreshFloating()
{
	mFloating = mSuspendPoints.floating();
	onFloatingChanged();
	updateHelpPrompts();
}

FileData* ISimpleGameListView::findGame(const std::string& code) const
{
	for(FileData* game : mRoot->getFilesRecursive(GAME))
		if(gameCode(game) == code)
			return game;
	return nullptr;
}

bool ISimpleGameListView::canJumpTo(const FileData* game)
{
	const FileData* cursor = getCursor();
	if(game == nullptr || game == cursor)
		return false;
	return game->getParent() == (mCursorStack.empty() ? mRoot : mCursorStack.top());
}

void ISimpleGameListView::jumpTo(FileData* game)
{
	std::vector<FileData*> folders;
	for(FileData* parent = game->getParent(); parent != nullptr && parent != mRoot; parent = parent->getParent())
		folders.push_back(parent);

	mCursorStack = std::stack<FileData*>();
	for(auto folder = folders.rbegin(); folder != folders.rend(); ++folder)
		mCursorStack.push(*folder);

	populateList(game->getParent()->getChildrenListToDisplay());
	setCursor(game);
}

bool ISimpleGameListView::jumpToFloating()
{
	FileData* target = findGame(mFloating.code);
	if(!canJumpTo(target))
		return false;
	jumpTo(target);
	return true;
}

bool ISimpleGameListView::openSuspendPoints()
{
	FileData* game = getCursor();
	if(game == nullptr || game->getType() != GAME)
		return false;

	auto* panel = new GuiSuspendPoints(mWindow, game->getName(), gameCode(game), mSuspendPoints, mPad,
		[this, game](int slot) {
			mPendingResume = slot;
			launch(game);
		},
		[this] { refreshFloating(); });
	panel->loadTheme(getTheme(), getName());
	mWindow->pushGui(panel);
	return true;
}

void ISimpleGameListView::applyPendingResume()
{
	if(mPendingResume > 0)
		setenv("ES_RESUME", std::to_string(mPendingResume).c_str(), 1);
	else
		unsetenv("ES_RESUME");
	mPendingResume = 0;
}
