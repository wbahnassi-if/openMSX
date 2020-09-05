#ifndef OSDTOPWIDGET_HH
#define OSDTOPWIDGET_HH

#include "OSDWidget.hh"
#include "TclObject.hh"
#include "EventDistributor.hh"
#include "EventListener.hh"
#include "hash_set.hh"
#include "xxhash.hh"
#include <vector>
#include <string>

namespace openmsx {

class OSDTopWidget final : public OSDWidget, private EventListener
{
public:
	explicit OSDTopWidget(Display& display, EventDistributor& eventDistributor);
	virtual ~OSDTopWidget();

	std::string_view getType() const override;
	gl::vec2 getSize(const OutputSurface& output) const override;

	void queueError(std::string message);
	void showAllErrors();

	OSDWidget* findByName(std::string_view name);
	const OSDWidget* findByName(std::string_view name) const;
	void addName(OSDWidget& widget);
	void removeName(OSDWidget& widget);
	std::vector<std::string_view> getAllWidgetNames() const;

protected:
	void invalidateLocal() override;
	void paintSDL(OutputSurface& output) override;
	void paintGL (OutputSurface& output) override;
	gl::ivec2 getMouseCoordUnscaled() const override { return gl::ivec2(mouseCachedX,mouseCachedY); }

private:
	int signalEvent(const std::shared_ptr<const Event>& event) override;
	int mouseCachedX,mouseCachedY;

	EventDistributor& eventDistributor;
	std::vector<std::string> errors;

	struct NameFromWidget {
		std::string_view operator()(const OSDWidget* w) const {
			return w->getName();
		}
	};
	hash_set<OSDWidget*, NameFromWidget, XXTclHasher> widgetsByName;
};

} // namespace openmsx

#endif
