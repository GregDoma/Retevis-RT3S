#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "functions/dmrRxAgc.h"

int main(void)
{
	dmrRxAgcState_t state;
	float gain;

	dmrRxAgcStateInit(&state);
	assert(state.peakAverage == 0.0f);
	assert(state.gain == 1.0f);

	// Noise and silence must not initialize an unknown station.
	gain = dmrRxAgcProcessPeak(&state, 4U, 200U);
	assert(gain == 1.0f);
	assert(state.peakAverage == 0.0f);
	gain = dmrRxAgcProcessPeak(&state, 4U, 500U);
	assert(gain == 1.0f);
	assert(state.peakAverage == 0.0f);

	// The first qualifying speech buffer initializes from the floor and starts
	// the gain smoothly, rather than applying a large remembered/default gain.
	gain = dmrRxAgcProcessPeak(&state, 4U, 1000U);
	assert(state.peakAverage == 1500.0f);
	assert(gain > 1.0f);
	assert(gain < 2.0f);

	// A remembered station level survives a setting transition, while the
	// transient history and applied gain return to a safe state.
	dmrRxAgcSetStationLevel(&state, 6000U);
	state.history[0] = 1000U;
	state.gain = 2.0f;
	state.applied = 1.5f;
	dmrRxAgcSettingsChanged(&state);
	assert(state.peakAverage == 6000.0f);
	assert(state.history[0] == 0U);
	assert(state.gain == 1.0f);
	assert(state.applied == 1.0f);

	// All eight menu settings remain finite and usable.
	for (uint8_t setting = 1U; setting <= DMR_RX_AGC_SETTING_COUNT; setting++)
	{
		gain = dmrRxAgcProcessPeak(&state, setting, 6000U);
		assert(isfinite(gain));
		assert(gain > 0.0f);
	}

	// Invalid persisted values are safely bounded by the production algorithm.
	gain = dmrRxAgcProcessPeak(&state, 255U, 6000U);
	assert(isfinite(gain));
	assert(gain > 0.0f);
	assert(dmrRxAgcProcessPeak(&state, 0U, 6000U) == 1.0f);

	// Prompt/disabled bypass resets only the interpolation origin, not station
	// learning or the smoothed target gain.
	state.peakAverage = 4321.0f;
	state.gain = 0.75f;
	state.history[0] = 1234U;
	state.applied = 0.5f;
	dmrRxAgcSetBypass(&state);
	assert(state.peakAverage == 4321.0f);
	assert(state.gain == 0.75f);
	assert(state.history[0] == 1234U);
	assert(state.applied == 1.0f);

	// Processed audio saturates correctly, including INT16_MIN; bypass callers
	// copy the raw bits without using this scaling helper.
	assert(dmrRxAgcScaleSample(INT16_MAX, 2.0f) == INT16_MAX);
	assert(dmrRxAgcScaleSample(INT16_MIN, 2.0f) == INT16_MIN);
	assert(dmrRxAgcScaleSample(-100, 0.5f) == -50);

	puts("dmr_rx_agc: ok");
	return 0;
}
