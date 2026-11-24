/*
 * Copyright (C) 2025 Roger Clark, VK3KYY / G4KYF
 *
 * DMR receive AGC tuning profile adapted from TA8BMO's OpenGD77 work.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OPENGD77_DMR_RX_AGC_H
#define OPENGD77_DMR_RX_AGC_H

#include <stdbool.h>
#include <stdint.h>

#define DMR_RX_AGC_SETTING_COUNT 8U
#define DMR_RX_AGC_DEFAULT_PEAK_SAMPLES 0.0f

typedef struct
{
	const void *entry;
	uint32_t stationId;
	uint32_t stationToken;
} dmrRxAgcOwnerSnapshot_t;

typedef struct
{
	const void *entry;
	uint32_t stationId;
	uint32_t stationToken;
	uint32_t sequence;
	bool valid;
} dmrRxAgcOwner_t;

typedef struct
{
	float peakAverage;
	float gain;
	float applied;
	uint16_t history[3];
} dmrRxAgcState_t;

void dmrRxAgcStateInit(volatile dmrRxAgcState_t *state);
void dmrRxAgcStateCopy(volatile dmrRxAgcState_t *destination, const volatile dmrRxAgcState_t *source);
void dmrRxAgcClearHistory(volatile dmrRxAgcState_t *state);
void dmrRxAgcSetStationLevel(volatile dmrRxAgcState_t *state, uint16_t rememberedPeak);
void dmrRxAgcSettingsChanged(volatile dmrRxAgcState_t *state);
void dmrRxAgcSetBypass(volatile dmrRxAgcState_t *state);
void dmrRxAgcSetAppliedGain(volatile dmrRxAgcState_t *state, float gain);
float dmrRxAgcProcessPeak(volatile dmrRxAgcState_t *state, uint8_t setting, uint32_t peak);
float dmrRxAgcTargetForSetting(uint8_t setting);

static inline int16_t dmrRxAgcReadPcmSample(const volatile uint8_t *buffer, uint32_t sampleIndex)
{
	uint16_t bits = (uint16_t)buffer[2U * sampleIndex] |
			((uint16_t)buffer[(2U * sampleIndex) + 1U] << 8);

	return (int16_t)bits;
}

static inline float dmrRxAgcInterpolatedGain(float from, float gain, float gainStep, uint32_t sampleIndex)
{
	return (gainStep > 0.0f) ? (from + (gainStep * (float)(sampleIndex + 1U))) : gain;
}

static inline int16_t dmrRxAgcScaleSample(int16_t sample, float gain)
{
	float value = (float)sample * gain;

	if (value > 32767.0f)
	{
		return INT16_MAX;
	}
	if (value < -32768.0f)
	{
		return INT16_MIN;
	}

	return (int16_t)value;
}

void dmrRxAgcOwnerInvalidate(volatile dmrRxAgcOwner_t *owner);
void dmrRxAgcOwnerPublish(volatile dmrRxAgcOwner_t *owner, const void *entry, uint32_t stationId, uint32_t stationToken);
bool dmrRxAgcOwnerSnapshot(const volatile dmrRxAgcOwner_t *owner, dmrRxAgcOwnerSnapshot_t *snapshot);
bool dmrRxAgcOwnerStillCurrent(const volatile dmrRxAgcOwner_t *owner, const dmrRxAgcOwnerSnapshot_t *snapshot);

#endif /* OPENGD77_DMR_RX_AGC_H */
