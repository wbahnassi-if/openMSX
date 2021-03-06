#ifdef _WIN32

#include "win32-arggen.hh"
#include "MSXException.hh"
#include "utf8_checked.hh"
#include <windows.h>
#include <shellapi.h>

namespace openmsx {

ArgumentGenerator::~ArgumentGenerator()
{
	for (int i = 0; i < argc; ++i) {
		free(argv[i]);
	}
}

char** ArgumentGenerator::GetArguments(int& argc_)
{
	if (argv.empty()) {
		LPWSTR* pszArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (!pszArglist) {
			throw MSXException("Failed to obtain command line arguments");
		}

		argv.resize(argc);
		for (int i = 0; i < argc; ++i) {
			argv[i] = strdup(utf8::utf16to8(pszArglist[i]).c_str());
		}
		LocalFree(pszArglist);
	}

	argc_ = argc;
	return argv.data();
}

} // namespace openmsx

#endif
