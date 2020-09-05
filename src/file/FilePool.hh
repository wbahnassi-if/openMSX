#ifndef FILEPOOL_HH
#define FILEPOOL_HH

#include "EventListener.hh"
#include "FilePoolCore.hh"
#include "StringSetting.hh"
#include "Observer.hh"

namespace openmsx {

class CommandController;
class Reactor;
class Sha1SumCommand;

class FilePool final : private Observer<Setting>, private EventListener
{
public:
	FilePool(CommandController& controller, Reactor& reactor);
	~FilePool();

	/** Search file with the given sha1sum.
	 * If found it returns the (already opened) file,
	 * if not found it returns nullptr.
	 */
	File getFile(FileType fileType, const Sha1Sum& sha1sum);

	/** Calculate sha1sum for the given File object.
	 * If possible the result is retrieved from cache, avoiding the
	 * relatively expensive calculation.
	 */
	Sha1Sum getSha1Sum(File& file);

private:
	FilePoolCore::Directories getDirectories() const;
	void reportProgress(const std::string& message);

	// Observer<Setting>
	void update(const Setting& setting) override;

	// EventListener
	int signalEvent(const std::shared_ptr<const Event>& event) override;

private:
	FilePoolCore core;
	StringSetting filePoolSetting;
	Reactor& reactor;
	std::unique_ptr<Sha1SumCommand> sha1SumCommand;
	bool quit = false;
};

} // namespace openmsx

#endif
