// $Id$

#ifndef DYNAMICCLOCK_HH
#define DYNAMICCLOCK_HH

#include "EmuTime.hh"
#include "DivModBySame.hh"
#include <cassert>

namespace openmsx {

/** Represents a clock with a variable frequency.
  * The frequency is in Hertz, so every tick is 1/frequency second.
  * A clock has a current time, which can be increased by
  * an integer number of ticks.
  */
class DynamicClock
{
public:
	// Note: default copy constructor and assigment operator are ok.

	/** Create a new clock, which starts ticking at time zero.
	  * The initial frequency is infinite;
	  * in other words, the clock stands still.
	  */
	explicit DynamicClock(const EmuTime& time) : lastTick(time), step(0) { }

	/** Gets the time at which the last clock tick occurred.
	  */
	const EmuTime& getTime() const {
		return lastTick;
	}

	/** Checks whether this clock's last tick is or is not before the
	  * given time stamp.
	  */
	bool before(const EmuTime& e) const {
		return lastTick.time < e.time;
	}

	/** Calculate the number of ticks for this clock until the given time.
	  * It is not allowed to call this method for a time in the past.
	  */
	unsigned getTicksTill(const EmuTime& e) const {
		assert(e.time >= lastTick.time);
		return divmod.div(e.time - lastTick.time);
	}

	/** Calculate the number of ticks this clock has to tick to reach
	  * or go past the given time.
	  * It is not allowed to call this method for a time in the past.
	  */
	unsigned getTicksTillUp(const EmuTime& e) const {
		assert(e.time >= lastTick.time);
		return divmod.div(e.time - lastTick.time + (step - 1));
	}

	/** Change the frequency at which this clock ticks.
	  * @param freq New frequency in Hertz.
	  */
	void setFreq(unsigned freq) {
		step = MAIN_FREQ32 / freq;
		assert(step);
		divmod.setDivisor(step);
	}

	/** Returns the frequency (in Hz) at which this clock ticks.
	  * @see setFreq()
	  */
	unsigned getFreq() const {
		assert(step);
		return MAIN_FREQ32 / step;
	}

	/** Reset the clock to start ticking at the given time.
	  */
	void reset(const EmuTime& e) {
		lastTick.time = e.time;
	}

	/** Advance this clock in time until the last tick which is not past
	  * the given time.
	  * It is not allowed to advance a clock to a time in the past.
	  */
	void advance(const EmuTime& e) {
		assert(lastTick.time <= e.time);
		lastTick.time = e.time - divmod.mod(e.time - lastTick.time);
	}

	/** Advance this clock by the given number of ticks.
	  */
	void operator+=(uint64 n) {
		lastTick.time += n * step;
	}

	/** Advance this clock by the given number of ticks.
	  * This method is similar to operator+=, but it's optimized for
	  * speed. OTOH the amount of ticks should not be too large,
	  * otherwise an overflow occurs. Use operator+() when the duration
	  * of the ticks approaches 1 second.
	  */
	void fastAdd(unsigned n) {
		#ifdef DEBUG
		// we don't even want this overhead in development versions
		assert((uint64(n) * step) < (1ull << 32));
		#endif
		lastTick.time += n * step;
	}
	EmuTime getFastAdd(unsigned n) const {
		#ifdef DEBUG
		assert((uint64(n) * step) < (1ull << 32));
		#endif
		return EmuTime(lastTick.time + n * step);
	}

private:
	/** Time of this clock's last tick.
	  */
	EmuTime lastTick;

	/** Length of a this clock's ticks, expressed in master clock ticks.
	  */
	unsigned step; // changed uint64 -> unsigned for performance reasons
	               // this is _heavily_ used in the CPU code

	DivModBySame divmod;
};

} // namespace openmsx

#endif
