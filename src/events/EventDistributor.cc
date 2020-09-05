#include "EventDistributor.hh"
#include "EventListener.hh"
#include "Reactor.hh"
#include "RTScheduler.hh"
#include "Interpreter.hh"
#include "InputEventGenerator.hh"
#include "Thread.hh"
#include "ranges.hh"
#include "stl.hh"
#include "view.hh"
#include <cassert>
#include <chrono>

using std::string;

namespace openmsx {

EventDistributor::EventDistributor(Reactor& reactor_)
	: reactor(reactor_)
{
}

void EventDistributor::registerEventListener(
		EventType type, EventListener& listener, Priority priority)
{
	std::lock_guard<std::mutex> lock(mutex);
	auto& priorityMap = listeners[type];
	// a listener may only be registered once for each type
	assert(!contains(view::values(priorityMap), &listener));
	// insert at highest position that keeps listeners sorted on priority
	auto it = ranges::upper_bound(priorityMap, priority, LessTupleElement<0>());
	priorityMap.insert(it, {priority, &listener});
}

void EventDistributor::unregisterEventListener(
		EventType type, EventListener& listener)
{
	std::lock_guard<std::mutex> lock(mutex);
	auto& priorityMap = listeners[type];
	priorityMap.erase(rfind_if_unguarded(priorityMap,
		[&](auto& v) { return v.second == &listener; }));
}

void EventDistributor::distributeEvent(const EventPtr& event)
{
	// TODO: Implement a real solution against modifying data structure while
	//       iterating through it.
	//       For example, assign nullptr first and then iterate again after
	//       delivering events to remove the nullptr values.
	// TODO: Is it useful to test for 0 listeners or should we just always
	//       queue the event?
	assert(event);
	std::unique_lock<std::mutex> lock(mutex);
	if (!listeners[event->getType()].empty()) {
		scheduledEvents.push_back(event);
		// must release lock, otherwise there's a deadlock:
		//   thread 1: Reactor::deleteMotherBoard()
		//             EventDistributor::unregisterEventListener()
		//   thread 2: EventDistributor::distributeEvent()
		//             Reactor::enterMainLoop()
		condition.notify_all();
		lock.unlock();
		reactor.enterMainLoop();
	}
}

bool EventDistributor::isRegistered(EventType type, EventListener* listener) const
{
	return contains(view::values(listeners[type]), listener);
}

void EventDistributor::deliverEvents()
{
	static PriorityMap priorityMapCopy; // static to preserve capacity
	static EventQueue eventsCopy;       // static to preserve capacity

	assert(Thread::isMainThread());

	reactor.getInputEventGenerator().poll();
	reactor.getInterpreter().poll();
	reactor.getRTScheduler().execute();

	std::unique_lock<std::mutex> lock(mutex);
	// It's possible that executing an event triggers scheduling of another
	// event. We also want to execute those secondary events. That's why
	// we have this while loop here.
	// For example the 'loadstate' command event, triggers a machine switch
	// event and as reaction to the latter event, AfterCommand will
	// unsubscribe from the ols MSXEventDistributor. This really should be
	// done before we exit this method.
	while (!scheduledEvents.empty()) {
		assert(eventsCopy.empty());
		swap(eventsCopy, scheduledEvents);
		for (auto& event : eventsCopy) {
			auto type = event->getType();
			priorityMapCopy = listeners[type];
			lock.unlock();
			auto blockPriority = unsigned(-1); // allow all
			for (const auto& [priority, listener] : priorityMapCopy) {
				// It's possible delivery to one of the previous
				// Listeners unregistered the current Listener.
				if (!isRegistered(type, listener)) continue;

				if (priority >= blockPriority) break;

				if (unsigned block = listener->signalEvent(event)) {
					assert(block > priority);
					blockPriority = block;
				}
			}
			lock.lock();
		}
		eventsCopy.clear();
	}
}

bool EventDistributor::sleep(unsigned us)
{
	std::chrono::microseconds duration(us);
	std::unique_lock<std::mutex> lock(cvMutex);
	return condition.wait_for(lock, duration) == std::cv_status::timeout;
}

} // namespace openmsx
