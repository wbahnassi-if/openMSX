#include "RealTime.hh"
#include "Timer.hh"
#include "EventDistributor.hh"
#include "EventDelay.hh"
#include "Event.hh"
#include "FinishFrameEvent.hh"
#include "GlobalSettings.hh"
#include "MSXMotherBoard.hh"
#include "Reactor.hh"
#include "IntegerSetting.hh"
#include "BooleanSetting.hh"
#include "ThrottleManager.hh"
#include "checked_cast.hh"

namespace openmsx {

const double   SYNC_INTERVAL = 0.08;  // s
const int64_t  MAX_LAG       = 200000; // us
const uint64_t ALLOWED_LAG   =  20000; // us

RealTime::RealTime(
		MSXMotherBoard& motherBoard_, GlobalSettings& globalSettings,
		EventDelay& eventDelay_)
	: Schedulable(motherBoard_.getScheduler())
	, motherBoard(motherBoard_)
	, eventDistributor(motherBoard.getReactor().getEventDistributor())
	, eventDelay(eventDelay_)
	, speedManager   (globalSettings.getSpeedManager())
	, throttleManager(globalSettings.getThrottleManager())
	, pauseSetting   (globalSettings.getPauseSetting())
	, powerSetting   (globalSettings.getPowerSetting())
	, emuTime(EmuTime::zero())
	, enabled(true)
{
	speedManager.attach(*this);
	throttleManager.attach(*this);
	pauseSetting.attach(*this);
	powerSetting.attach(*this);

	resync();

	eventDistributor.registerEventListener(OPENMSX_FINISH_FRAME_EVENT, *this);
	eventDistributor.registerEventListener(OPENMSX_FRAME_DRAWN_EVENT,  *this);
}

RealTime::~RealTime()
{
	eventDistributor.unregisterEventListener(OPENMSX_FRAME_DRAWN_EVENT,  *this);
	eventDistributor.unregisterEventListener(OPENMSX_FINISH_FRAME_EVENT, *this);

	powerSetting.detach(*this);
	pauseSetting.detach(*this);
	throttleManager.detach(*this);
	speedManager.detach(*this);
}

double RealTime::getRealDuration(EmuTime::param time1, EmuTime::param time2)
{
	return (time2 - time1).toDouble() / speedManager.getSpeed();
}

EmuDuration RealTime::getEmuDuration(double realDur)
{
	return EmuDuration(realDur * speedManager.getSpeed());
}

bool RealTime::timeLeft(uint64_t us, EmuTime::param time)
{
	auto realDuration = static_cast<uint64_t>(
		getRealDuration(emuTime, time) * 1000000ULL);
	auto currentRealTime = Timer::getTime();
	return (currentRealTime + us) <
	           (idealRealTime + realDuration + ALLOWED_LAG);
}

void RealTime::sync(EmuTime::param time, bool allowSleep)
{
	if (allowSleep) {
		removeSyncPoint();
	}
	internalSync(time, allowSleep);
	if (allowSleep) {
		setSyncPoint(time + getEmuDuration(SYNC_INTERVAL));
	}
}

void RealTime::internalSync(EmuTime::param time, bool allowSleep)
{
	if (throttleManager.isThrottled()) {
		auto realDuration = static_cast<uint64_t>(
		        getRealDuration(emuTime, time) * 1000000ULL);
		idealRealTime += realDuration;
		auto currentRealTime = Timer::getTime();
		int64_t sleep = idealRealTime - currentRealTime;
		if (allowSleep) {
			// want to sleep for 'sleep' us
			sleep += static_cast<int64_t>(sleepAdjust);
			int64_t delta = 0;
			if (sleep > 0) {
				Timer::sleep(sleep); // request to sleep for 'sleep+sleepAdjust'
				int64_t slept = Timer::getTime() - currentRealTime;
				delta = sleep - slept; // actually slept for 'slept' us
			}
			const double ALPHA = 0.2;
			sleepAdjust = sleepAdjust * (1 - ALPHA) + delta * ALPHA;
		}
		if (-sleep > MAX_LAG) {
			idealRealTime = currentRealTime - MAX_LAG / 2;
		}
	}
	if (allowSleep) {
		eventDelay.sync(time);
	}

	emuTime = time;
}

void RealTime::executeUntil(EmuTime::param time)
{
	internalSync(time, true);
	setSyncPoint(time + getEmuDuration(SYNC_INTERVAL));
}

int RealTime::signalEvent(const std::shared_ptr<const Event>& event)
{
	if (!motherBoard.isActive() || !enabled) {
		// these are global events, only the active machine should
		// synchronize with real time
		return 0;
	}
	if (event->getType() == OPENMSX_FINISH_FRAME_EVENT) {
		auto& ffe = checked_cast<const FinishFrameEvent&>(*event);
		if (!ffe.needRender()) {
			// sync but don't sleep
			sync(getCurrentTime(), false);
		}
	} else if (event->getType() == OPENMSX_FRAME_DRAWN_EVENT) {
		// sync and possibly sleep
		sync(getCurrentTime(), true);
	}
	return 0;
}

void RealTime::update(const Setting& /*setting*/)
{
	resync();
}

void RealTime::update(const SpeedManager& /*speedManager*/)
{
	resync();
}

void RealTime::update(const ThrottleManager& /*throttleManager*/)
{
	resync();
}

void RealTime::resync()
{
	if (!enabled) return;

	idealRealTime = Timer::getTime();
	sleepAdjust = 0.0;
	removeSyncPoint();
	emuTime = getCurrentTime();
	setSyncPoint(emuTime + getEmuDuration(SYNC_INTERVAL));
}

void RealTime::enable()
{
	enabled = true;
	resync();
}

void RealTime::disable()
{
	enabled = false;
	removeSyncPoint();
}

} // namespace openmsx
