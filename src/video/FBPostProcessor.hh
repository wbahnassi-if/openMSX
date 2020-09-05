#ifndef FBPOSTPROCESSOR_HH
#define FBPOSTPROCESSOR_HH

#include "PostProcessor.hh"
#include "RenderSettings.hh"
#include "PixelOperations.hh"
#include "ScalerOutput.hh"
#include <vector>

namespace openmsx {

class MSXMotherBoard;
class Display;
template<typename Pixel> class Scaler;

/** Rasterizer using SDL.
  */
template <class Pixel>
class FBPostProcessor final : public PostProcessor
{
public:
	FBPostProcessor(
		MSXMotherBoard& motherBoard, Display& display,
		OutputSurface& screen, const std::string& videoSource,
		unsigned maxWidth, unsigned height, bool canDoInterlace);
	~FBPostProcessor() override;

	// Layer interface:
	void paint(OutputSurface& output) override;

	std::unique_ptr<RawFrame> rotateFrames(
		std::unique_ptr<RawFrame> finishedFrame, EmuTime::param time) override;

private:
	void preCalcNoise(float factor);
	void drawNoise(OutputSurface& output);
	void drawNoiseLine(Pixel* buf, signed char* noise,
	                   size_t width);

	// Observer<Setting>
	void update(const Setting& setting) override;

	/** The currently active scaler.
	  */
	std::unique_ptr<Scaler<Pixel>> currScaler;

	/** The currently active stretch-scaler (horizontal stretch setting).
	 */
	std::unique_ptr<ScalerOutput<Pixel>> stretchScaler;

	/** Currently active scale algorithm, used to detect scaler changes.
	  */
	RenderSettings::ScaleAlgorithm scaleAlgorithm;

	/** Currently active scale factor, used to detect scaler changes.
	  */
	unsigned scaleFactor;

	/** Currently active stretch factor, used to detect setting changes.
	  */
	unsigned stretchWidth;

	/** Remember the noise values to get a stable image when paused.
	 */
	std::vector<unsigned> noiseShift;

	PixelOperations<Pixel> pixelOps;
};

} // namespace openmsx

#endif // FBPOSTPROCESSOR_HH
