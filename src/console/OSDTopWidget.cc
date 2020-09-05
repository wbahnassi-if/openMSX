#include "OSDTopWidget.hh"
#include "OutputSurface.hh"
#include "Display.hh"
#include "CliComm.hh"
#include "EventDistributor.hh"
#include "InputEvents.hh"
#include "view.hh"
#include "checked_cast.hh"

namespace openmsx {

OSDTopWidget::OSDTopWidget(Display& display_, EventDistributor& eventDistributor_)
	: OSDWidget(display_, TclObject()),
	  mouseCachedX(0),mouseCachedY(0),
	  eventDistributor(eventDistributor_)
{
	addName(*this);
	eventDistributor.registerEventListener(OPENMSX_MOUSE_MOTION_EVENT,*this,EventDistributor::CONSOLE);
}

OSDTopWidget::~OSDTopWidget()
{
	eventDistributor.unregisterEventListener(OPENMSX_MOUSE_MOTION_EVENT,*this);
}

std::string_view OSDTopWidget::getType() const
{
	return "top";
}

gl::vec2 OSDTopWidget::getSize(const OutputSurface& output) const
{
	return gl::vec2(output.getLogicalSize()); // int -> float
}

void OSDTopWidget::invalidateLocal()
{
	// nothing
}

void OSDTopWidget::paintSDL(OutputSurface& /*output*/)
{
	// nothing
}

void OSDTopWidget::paintGL (OutputSurface& /*output*/)
{
	// nothing
}

int OSDTopWidget::signalEvent(const std::shared_ptr<const Event>& event)
{
	auto& mme = checked_cast<const MouseMotionEvent&>(*event);
	mouseCachedX = mme.getAbsX();
	mouseCachedY = mme.getAbsY();
	return 0;
}

void OSDTopWidget::queueError(std::string message)
{
	errors.push_back(std::move(message));
}

void OSDTopWidget::showAllErrors()
{
	auto& cliComm = getDisplay().getCliComm();
	for (auto& message : errors) {
		cliComm.printWarning(std::move(message));
	}
	errors.clear();
}

OSDWidget* OSDTopWidget::findByName(std::string_view widgetName)
{
	auto it = widgetsByName.find(widgetName);
	return (it != end(widgetsByName)) ? *it : nullptr;
}

const OSDWidget* OSDTopWidget::findByName(std::string_view widgetName) const
{
	return const_cast<OSDTopWidget*>(this)->findByName(widgetName);
}

void OSDTopWidget::addName(OSDWidget& widget)
{
	assert(!widgetsByName.contains(widget.getName()));
	widgetsByName.emplace_noDuplicateCheck(&widget);
}

void OSDTopWidget::removeName(OSDWidget& widget)
{
	auto it = widgetsByName.find(widget.getName());
	assert(it != end(widgetsByName));
	for (auto& child : (*it)->getChildren()) {
		removeName(*child);
	}
	widgetsByName.erase(it);
}

std::vector<std::string_view> OSDTopWidget::getAllWidgetNames() const
{
	return to_vector(view::transform(widgetsByName,
	                                 [](auto* p) { return p->getName(); }));
}

} // namespace openmsx
