from checksum import verifyFile
from components import requiredLibrariesFor
from configurations import getConfiguration
from download import downloadURL
from extract import TopLevelDirRenamer, extract
from libraries import allDependencies, librariesByName
from packages import getPackage, SDL2
from patch import Diff, patch

from os import makedirs, system
from os.path import abspath, isdir, isfile, join as joinpath
from shutil import rmtree, copytree, copyfile, make_archive
import sys

def main(sourcesDir, derivedDir, patchesDir):
	# Build the shared openMSX.so lib
	print('Build shared library')
	system('make staticbindist OPENMSX_TARGET_OS=android OPENMSX_TARGET_CPU=arm OPENMSX_FLAVOUR=debug')

	# Copy the shared lib to its proper location in the android project directory
	androidJavaProjectRootDir = joinpath(derivedDir, 'android-sdl2')
	androidJavaProjectDir = joinpath(androidJavaProjectRootDir, 'app/src/main')
	sharedLibSourceDir = joinpath(derivedDir, 'arm-android-debug-3rd/lib')
	sharedLibTargetDir = joinpath(androidJavaProjectDir, 'jniLibs/armeabi-v7a')
	print('Copy shared library to Java project at: ' + sharedLibTargetDir)
	makedirs(sharedLibTargetDir, exist_ok=True)
	copyfile(joinpath(sharedLibSourceDir,'openmsx.so'),joinpath(sharedLibTargetDir,'libopenmsx.so'))

	# Zip the assets
	assetsSourceDir = joinpath(derivedDir, 'arm-android-debug-3rd/bindist/install/share')
	assetsZipFile = joinpath(androidJavaProjectDir, 'assets/appdata')
	print('Compress openMSX assets from: ' + assetsSourceDir + ' to zip archive: ' + assetsZipFile)
	makedirs(joinpath(androidJavaProjectDir, 'assets'), exist_ok=True)
	make_archive(assetsZipFile,'zip',assetsSourceDir)

	print('Build SDL Android project')
	system(joinpath(androidJavaProjectRootDir, 'gradlew -p ') + androidJavaProjectRootDir + ' assembleDebug')

if __name__ == '__main__':
	main(
		'derived/3rdparty/src',
		'derived',
		'build/3rdparty'
		)