from checksum import verifyFile
from components import requiredLibrariesFor
from configurations import getConfiguration
from download import downloadURL
from extract import TopLevelDirRenamer, extract
from libraries import allDependencies, librariesByName
from packages import getPackage, SDL2
from patch import Diff, patch

from os import makedirs
from os.path import abspath, isdir, isfile, join as joinpath
from shutil import rmtree, copytree, copyfile
import sys
try:
	from os import symlink
except ImportError:
	def symlink(source, link_name):
		raise OSError('OS does not support symlink creation')

def main(sourcesDir, derivedDir, patchesDir):
	sdlPackage = SDL2.getSourceDirName()
	sdlSrcPath = joinpath(sourcesDir, sdlPackage)
	sdlAndroidProjectSrcPath = joinpath(sdlSrcPath, 'android-project')
	sdlAndroidProjectPath = joinpath(derivedDir, 'android-sdl2')
	if not isdir(sdlSrcPath):
		print('SDL2 package not found extracted at: ', sdlAndroidProjectSrcPath, file=sys.stderr)
		sys.exit(2)

	print('Copy SDL2 Android package from: ', sdlAndroidProjectSrcPath, ' -> ', sdlAndroidProjectPath)
	copytree(src=sdlAndroidProjectSrcPath, dst=sdlAndroidProjectPath, dirs_exist_ok=True)

	sdlAndroidProjectJavaSrcPath = joinpath(patchesDir, 'SDL2-android.java')
	sdlAndroidProjectJavaPath = joinpath(sdlAndroidProjectPath, 'app/src/main/java/org/openmsx/android/openmsx')
	print('Copy openMSX Java file from: ', sdlAndroidProjectJavaSrcPath, ' -> ', sdlAndroidProjectJavaPath)
	makedirs(sdlAndroidProjectJavaPath, exist_ok=True)
	copyfile(sdlAndroidProjectJavaSrcPath,joinpath(sdlAndroidProjectJavaPath,'openMSXActivity.java'))

	print('Patch SDL2 Android package at: ', sdlAndroidProjectPath)
	for diff in Diff.load(joinpath(patchesDir, 'SDL2-android.diff')):
		patch(diff, sdlAndroidProjectPath)
		print('Patched:', diff.getPath())

	sdlSymlinkTarget = joinpath(sdlAndroidProjectPath, 'app/jni/SDL')
	print('Symlink SDL2: ', sdlSrcPath, ' -> ', sdlSymlinkTarget)
	symlink(abspath(sdlSrcPath), sdlSymlinkTarget, target_is_directory=True)
    
	# configuration = getConfiguration('3RD_STA')
	# components = configuration.iterDesiredComponents()

	# Compute the set of all directly and indirectly required libraries,
	# then filter out system libraries.
	# thirdPartyLibs = set(
	#	makeName
	#	for makeName in allDependencies(requiredLibrariesFor(components))
	#	if not librariesByName[makeName].isSystemLibrary(platform)
	#	)

	#for makeName in sorted(thirdPartyLibs):
	#	fetchPackageSource(makeName, tarballsDir, sourcesDir, patchesDir)

if __name__ == '__main__':
	main(
		'derived/3rdparty/src',
		'derived',
		'build/3rdparty'
		)