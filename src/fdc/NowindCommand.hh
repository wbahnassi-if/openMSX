#ifndef NOWINDCOMMAND_HH
#define NOWINDCOMMAND_HH

#include "NowindHost.hh"
#include "Command.hh"
#include <memory>
#include <string_view>

namespace openmsx {

class NowindInterface;
class DiskChanger;
class MSXMotherBoard;

class NowindCommand final : public Command
{
public:
	NowindCommand(const std::string& basename,
	              CommandController& commandController,
	              NowindInterface& interface);
	void execute(span<const TclObject> tokens, TclObject& result) override;
	std::string help(const std::vector<std::string>& tokens) const override;
	void tabCompletion(std::vector<std::string>& tokens) const override;

	std::unique_ptr<DiskChanger> createDiskChanger(
		const std::string& basename, unsigned n,
		MSXMotherBoard& motherBoard) const;

private:
	unsigned searchRomdisk(const NowindHost::Drives& drives) const;
	void processHdimage(const std::string& hdimage,
	                    NowindHost::Drives& drives) const;
	NowindInterface& interface;
};

} // namespace openmsx

#endif
