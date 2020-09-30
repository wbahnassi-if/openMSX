#ifndef INPUTEVENTGENERATOR_HH
#define INPUTEVENTGENERATOR_HH

#include "BooleanSetting.hh"
#include "EventListener.hh"
#include "Command.hh"
#include "Keys.hh"
#include <SDL.h>
#include <memory>

namespace openmsx {

class CommandController;
class EventDistributor;
class GlobalSettings;

class InputEventGenerator final : private EventListener
{
public:
	InputEventGenerator(const InputEventGenerator&) = delete;
	InputEventGenerator& operator=(const InputEventGenerator&) = delete;

	InputEventGenerator(CommandController& commandController,
	                    EventDistributor& eventDistributor,
	                    GlobalSettings& globalSettings);
	~InputEventGenerator();

	/** Wait for event(s) and handle it.
	  * This method should be called from the main thread.
	  */
	void wait();

	/** Input Grab on or off */
	BooleanSetting& getGrabInput() { return grabInput; }
	/** Must be called when 'grabinput' or 'fullscreen' setting changes. */
	void updateGrab(bool grab);

	/** Normally the following two functions simply delegate to
	 * SDL_JoystickNumButtons() and SDL_JoystickGetButton(). Except on
	 * Android, see comments in .cc for more details.
	 */
	static int joystickNumButtons(SDL_Joystick* joystick);
	static bool joystickGetButton(SDL_Joystick* joystick, int button);

	void poll();

private:
	using EventPtr = std::shared_ptr<const Event>;

	void handle(const SDL_Event& evt);
	void handleKeyDown(const SDL_KeyboardEvent& key, uint32_t unicode);
	void handleText(const char* utf8);
	void setGrabInput(bool grab);

	// EventListener
	int signalEvent(const std::shared_ptr<const Event>& event) override;

	EventDistributor& eventDistributor;
	GlobalSettings& globalSettings;
	BooleanSetting grabInput;

	struct EscapeGrabCmd final : Command {
		explicit EscapeGrabCmd(CommandController& commandController);
		void execute(span<const TclObject> tokens, TclObject& result) override;
		std::string help(const std::vector<std::string>& tokens) const override;
	} escapeGrabCmd;

	enum EscapeGrabState {
		ESCAPE_GRAB_WAIT_CMD,
		ESCAPE_GRAB_WAIT_LOST,
		ESCAPE_GRAB_WAIT_GAIN
	} escapeGrabState;

	// Touch
	static const unsigned maxTouchFingers = 5;
	static const unsigned invalidFingerIndex = unsigned(-1);
	unsigned fingers[maxTouchFingers] = {}; // Tracked finger IDs
	unsigned fingerTrackStart(unsigned fingerID);
	unsigned fingerTrackEnd(unsigned fingerID);
	unsigned fingerFindIndex(unsigned fingerID) const;

	// OsdControl
	void setNewOsdControlButtonState(
		unsigned newState, const EventPtr& origEvent);
	void triggerOsdControlEventsFromJoystickAxisMotion(
		unsigned axis, int value, const EventPtr& origEvent);
	void triggerOsdControlEventsFromJoystickHat(
		int value, const EventPtr& origEvent);
	void osdControlChangeButton(
		bool up, unsigned changedButtonMask, const EventPtr& origEvent);
	void triggerOsdControlEventsFromJoystickButtonEvent(
		unsigned button, bool up, const EventPtr& origEvent);
	void triggerOsdControlEventsFromKeyEvent(
		Keys::KeyCode keyCode, bool up, bool repeat, const EventPtr& origEvent);


	unsigned osdControlButtonsState; // 0 is pressed, 1 is released

	// only for Android
	static inline bool androidButtonA = false;
	static inline bool androidButtonB = false;
};

} // namespace openmsx

#endif
