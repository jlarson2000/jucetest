#
# Simple nmake targets to do things the VS project doesn't do yet

# Need this only if you're not configuring the Juce plugin host to
# find plugins in build directories
# sigh, fails with permissions error

install:
	@echo Installing VST Plugin
	rm -rf "c:/Program Files/Common Files/VST3/firstplugin.vst3"
	cp -r Builds/VisualStudio2022/x64/Debug/VST3/firstplugin.vst3 "c:/Program Files/Common Files/VST3"
