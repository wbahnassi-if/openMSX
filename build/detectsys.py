# $Id$
# Detect the native CPU and OS.
# Actually we rely on the Python "platform" module and map its output to names
# that the openMSX build understands.

from platform import libc_ver, machine, system

def detectCPU():
	'''Detects the CPU family (not the CPU model) of the machine were are
	running on.
	If no known CPU is detected, None is returned.
	'''
	cpu = machine().lower()
	if cpu in ('x86_64', 'amd64'):
		return 'x86_64'
	elif cpu in ('x86', 'i386', 'i486', 'i586', 'i686'):
		return 'x86'
	elif cpu.startswith('ppc') or cpu.startswith('power'):
		return 'ppc64' if cpu.endswith('64') else 'ppc'
	elif cpu.startswith('arm'):
		return 'arm'
	elif cpu.startswith('mips'):
		return 'mipsel' if cpu.endswith('el') else 'mips'
	elif cpu == 'm68k':
		return 'm68k'
	elif cpu == 'ia64':
		return 'ia64'
	elif cpu.startswith('alpha'):
		return 'alpha'
	elif cpu.startswith('hppa'):
		return 'hppa'
	elif cpu.startswith('s390'):
		return 's390'
	elif cpu.startswith('sparc'):
		return 'sparc'
	else:
		return None

def detectOS():
	'''Detects the operating system of the machine were are running on.
	If no known OS is detected, None is returned.
	'''
	os = system().lower()
	if os in ('linux', 'darwin', 'netbsd', 'openbsd', 'gnu'):
		return os
	elif os == 'freebsd':
		lib, version_ = libc_ver()
		if lib == 'glibc':
			# Debian kFreeBSD is GNU userland on a FreeBSD kernel; for openMSX
			# the kernel is not really relevant, so treat it like a generic
			# GNU system.
			return 'gnu'
		else:
			# Actual FreeBSD.
			return 'freebsd'
	elif os.startswith('mingw') or os == 'windows':
		return 'mingw32'
	else:
		return None

if __name__ == '__main__':
	print '%s-%s' % (detectCPU(), detectOS())
