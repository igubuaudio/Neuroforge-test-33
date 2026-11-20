#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    constexpr float pi = juce::MathConstants<float>::pi;
    float clamp01(float x) { return juce::jlimit(0.0f, 1.0f, x); }
}

NeuroforgeAudioProcessor::NeuroforgeAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "NEUROFORGE_STATE", createParameterLayout())
{
}

NeuroforgeAudioProcessor::APVTS::ParameterLayout NeuroforgeAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    auto addFloat = [&layout](const juce::String& id, const juce::String& name,
                              float min, float max, float def, float step = 0.0f)
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { id, 1 }, name,
            juce::NormalisableRange<float>(min, max, step), def));
    };

    addFloat("mix", "Mix", 0.0f, 1.0f, 1.0f);
    addFloat("output", "Output", -18.0f, 12.0f, 0.0f);
    addFloat("drive", "Drive", 0.0f, 36.0f, 12.0f);
    addFloat("tone", "Tone", 500.0f, 16000.0f, 9000.0f);
    addFloat("lowCut", "Low Crossover", 40.0f, 350.0f, 120.0f);
    addFloat("highCut", "High Crossover", 500.0f, 6000.0f, 2500.0f);
    addFloat("movement", "Movement", 0.0f, 1.0f, 0.35f);
    addFloat("metal", "Metal", 0.0f, 1.0f, 0.25f);
    addFloat("intensity", "Mutation", 0.0f, 1.0f, 0.65f);
    addFloat("complexity", "Complexity", 0.0f, 1.0f, 0.55f);
    addFloat("destruction", "Destruction", 0.0f, 1.0f, 0.8f);

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "algorithm", 1 }, "Distortion",
        juce::StringArray { "Soft Clip", "Hard Clip", "Asymmetric", "Foldback",
                            "Rectify", "Diode", "Tube", "Digital Crush" }, 3));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "personality", 1 }, "Personality",
        juce::StringArray { "Audio", "Ed Rush", "Alien", "Destruction" }, 0));
    return layout;
}

void NeuroforgeAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, 1, 1 };
    for (int ch = 0; ch < 2; ++ch)
    {
        lowPass[(size_t) ch].prepare(spec);
        lowHighPass[(size_t) ch].prepare(spec);
        midLowPass[(size_t) ch].prepare(spec);
        midHighPass[(size_t) ch].prepare(spec);

        lowPass[(size_t) ch].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        lowHighPass[(size_t) ch].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        midLowPass[(size_t) ch].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        midHighPass[(size_t) ch].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        lfoPhase[(size_t) ch] = 0.0f;
        toneState[(size_t) ch] = 0.0f;
    }
    smoothedMix = *parameters.getRawParameterValue("mix");
    smoothedOutput = juce::Decibels::decibelsToGain(*parameters.getRawParameterValue("output"));
}

void NeuroforgeAudioProcessor::releaseResources() {}

bool NeuroforgeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo())
        && in == out;
}

float NeuroforgeAudioProcessor::shape(float x, int algorithm, float drive, float bias) const noexcept
{
    const float g = juce::Decibels::decibelsToGain(drive);
    const float v = x * g + bias;
    switch (algorithm)
    {
        case 0: return std::tanh(v);
        case 1: return juce::jlimit(-1.0f, 1.0f, v);
        case 2: return std::tanh(v + 0.18f) - std::tanh(0.18f);
        case 3:
        {
            const float a = std::fmod(v + 3.0f, 4.0f);
            const float folded = a < 2.0f ? a - 1.0f : 3.0f - a;
            return juce::jlimit(-1.0f, 1.0f, folded);
        }
        case 4: return std::abs(std::tanh(v)) * 2.0f - 0.65f;
        case 5: return (2.0f / pi) * std::atan(v * 2.4f);
        case 6: return 0.75f * std::tanh(v) + 0.25f * std::tanh(v * 3.0f);
        case 7:
        {
            const float q = std::round(v * 18.0f) / 18.0f;
            return juce::jlimit(-1.0f, 1.0f, q);
        }
        default: return std::tanh(v);
    }
}

float NeuroforgeAudioProcessor::smooth(float current, float target, float coefficient) const noexcept
{
    return current + coefficient * (target - current);
}

void NeuroforgeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = juce::jmin(getTotalNumInputChannels(), buffer.getNumChannels());
    const int samples = buffer.getNumSamples();

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, samples);

    const float lowC = parameters.getRawParameterValue("lowCut")->load();
    const float highCutValue = parameters.getRawParameterValue("highCut")->load();
    const float highC = juce::jmax(lowC + 100.0f, highCutValue);
    const float drive = *parameters.getRawParameterValue("drive");
    const float tone = *parameters.getRawParameterValue("tone");
    const float movement = *parameters.getRawParameterValue("movement");
    const float metal = *parameters.getRawParameterValue("metal");
    const float destruction = *parameters.getRawParameterValue("destruction");
    const int algorithm = (int) *parameters.getRawParameterValue("algorithm");
    const float targetMix = *parameters.getRawParameterValue("mix");
    const float targetOutput = juce::Decibels::decibelsToGain(*parameters.getRawParameterValue("output"));

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        auto channel = (size_t) juce::jmin(ch, 1);

        lowPass[channel].setCutoffFrequency(lowC);
        lowHighPass[channel].setCutoffFrequency(lowC);
        midLowPass[channel].setCutoffFrequency(highC);
        midHighPass[channel].setCutoffFrequency(highC);

        const float lfoRate = 0.17f + movement * 3.8f;
        const float phaseInc = lfoRate / (float) currentSampleRate;
        const float toneAlpha = 1.0f - std::exp(-2.0f * pi * tone / (float) currentSampleRate);

        for (int i = 0; i < samples; ++i)
        {
            const float dry = data[i];
            const float low = lowPass[channel].processSample(0, dry);
            const float upper = lowHighPass[channel].processSample(0, dry);
            const float mid = midLowPass[channel].processSample(0, upper);
            const float high = midHighPass[channel].processSample(0, upper);

            float phase = lfoPhase[channel];
            const float lfo = 0.5f + 0.5f * std::sin(phase * 2.0f * pi);
            lfoPhase[channel] += phaseInc;
            if (lfoPhase[channel] >= 1.0f) lfoPhase[channel] -= 1.0f;

            const float dynamicDrive = drive + (movement * lfo * 9.0f);
            const float bias = (metal * 0.22f * std::sin(phase * 2.0f * pi * 2.0f));
            const float midWet = shape(mid, algorithm, dynamicDrive * (0.35f + destruction * 0.065f), bias);
            const float highWet = shape(high, (algorithm + 3) % 8,
                                        dynamicDrive * (0.2f + destruction * 0.075f), -bias);
            const float highTone = toneState[channel] = smooth(toneState[channel], highWet, toneAlpha);
            const float texturedHigh = highTone + (highWet - highTone) * (0.25f + metal);

            const float wet = low + midWet * 0.95f + texturedHigh * 0.55f;
            smoothedMix = smooth(smoothedMix, targetMix, 0.0025f);
            smoothedOutput = smooth(smoothedOutput, targetOutput, 0.0025f);
            data[i] = (dry + (wet - dry) * smoothedMix) * smoothedOutput;
        }
    }
}

void NeuroforgeAudioProcessor::mutate(int mode)
{
    // Called by the UI thread. Parameter writes use the host-notifying API.
    auto set = [this](const char* id, float value)
    {
        if (auto* p = parameters.getParameter(id))
            p->setValueNotifyingHost(p->convertTo0to1(value));
    };
    auto random = [this](float lo, float hi)
    {
        std::uniform_real_distribution<float> d(lo, hi);
        return d(randomEngine);
    };

    const float intensity = *parameters.getRawParameterValue("intensity");
    const float complexity = *parameters.getRawParameterValue("complexity");
    const float destruction = *parameters.getRawParameterValue("destruction");
    const float amount = 0.12f + intensity * 0.88f;

    auto maybeSet = [&](int lockIndex, const char* id, float lo, float hi, float current)
    {
        if (!locks[(size_t) lockIndex])
            set(id, juce::jlimit(lo, hi, current + random(-amount, amount) * (hi - lo)));
    };

    if (!locks[0])
        set("algorithm", (float) std::uniform_int_distribution<int>(0, 7)(randomEngine));
    if (!locks[1])
        set("drive", random(5.0f, 12.0f + destruction * 24.0f));
    maybeSet(2, "tone", 1800.0f, 16000.0f, *parameters.getRawParameterValue("tone"));
    maybeSet(3, "movement", 0.0f, 1.0f, *parameters.getRawParameterValue("movement"));
    maybeSet(4, "metal", 0.0f, 1.0f, *parameters.getRawParameterValue("metal"));
    maybeSet(5, "lowCut", 50.0f, 220.0f, *parameters.getRawParameterValue("lowCut"));
    maybeSet(6, "highCut", 900.0f, 5000.0f, *parameters.getRawParameterValue("highCut"));

    if (!locks[7])
    {
        int personality = mode;
        if (mode == 4)
            personality = std::uniform_int_distribution<int>(0, 3)(randomEngine);
        if (auto* p = parameters.getParameter("personality"))
            p->setValueNotifyingHost(p->convertTo0to1((float) personality));
    }

    if (!locks[1])
        set("destruction", juce::jlimit(0.0f, 1.0f, random(0.25f, 1.0f) * (0.45f + complexity * 0.55f)));
}

void NeuroforgeAudioProcessor::setLock(int index, bool shouldLock)
{
    if (juce::isPositiveAndBelow(index, (int) locks.size()))
        locks[(size_t) index] = shouldLock;
}

bool NeuroforgeAudioProcessor::getLock(int index) const
{
    return juce::isPositiveAndBelow(index, (int) locks.size()) ? locks[(size_t) index] : false;
}

void NeuroforgeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    juce::MemoryOutputStream stream(destData, false);
    state.writeToStream(stream);
}

void NeuroforgeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto state = juce::ValueTree:: readFromData(data, (size_t) sizeInBytes);
    if (state.isValid())
        parameters.replaceState(state);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeuroforgeAudioProcessor();
}
