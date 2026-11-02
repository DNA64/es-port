#include "components/VideoVlcComponent.h"

#include "renderers/Renderer.h"

// Stub for builds without libvlc (NES/SNES Classic): no video is ever played, the snapshot image is shown instead.

libvlc_instance_t*                       VideoVlcComponent::mVLC = nullptr;
std::thread                              VideoVlcComponent::sCleanupThread;
std::mutex                               VideoVlcComponent::sCleanupMutex;
std::condition_variable                  VideoVlcComponent::sCleanupCond;
std::deque<std::function<void()>>        VideoVlcComponent::sCleanupQueue;
bool                                     VideoVlcComponent::sCleanupRunning = false;
bool                                     VideoVlcComponent::sCleanupExit = false;

void VideoVlcComponent::setupVLC(std::string) {}

VideoVlcComponent::VideoVlcComponent(Window* window, std::string) :
	VideoComponent(window),
	mMedia(nullptr),
	mMediaPlayer(nullptr),
	mContext(nullptr),
	mMediaParsing(false)
{
}

VideoVlcComponent::~VideoVlcComponent() {}

void VideoVlcComponent::deinit() {}

void VideoVlcComponent::setResize(float width, float height)
{
	mTargetSize = Vector2f(width, height);
	mTargetIsMax = false;
	mStaticImage.setResize(width, height);
}

void VideoVlcComponent::setMaxSize(float width, float height)
{
	mTargetSize = Vector2f(width, height);
	mTargetIsMax = true;
	mStaticImage.setMaxSize(width, height);
}

void VideoVlcComponent::resize() {}

void VideoVlcComponent::render(const Transform4x4f& parentTrans)
{
	if (!isVisible())
		return;

	VideoComponent::render(parentTrans);
	GuiComponent::renderChildren(parentTrans * getTransform());
	VideoComponent::renderSnapshot(parentTrans);
}

void VideoVlcComponent::startVideo() {}
void VideoVlcComponent::stopVideo() {}
void VideoVlcComponent::handleLooping() {}
void VideoVlcComponent::setMuteMode() {}
void VideoVlcComponent::setupContext() {}
void VideoVlcComponent::freeContext() {}
void VideoVlcComponent::handleParsing() {}
void VideoVlcComponent::onMediaParsed() {}
void VideoVlcComponent::postCleanupTask(std::function<void()>) {}
void VideoVlcComponent::cleanupWorker() {}
