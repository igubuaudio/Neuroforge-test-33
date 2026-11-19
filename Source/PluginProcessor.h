#pragma once
#include <JuceHeader.h>
#include <array>
#include <random>

class NeuroforgeAudioProcessor final : public juce::AudioProcessor
{
public:
    NeuroforgeAudioProcessor();
    ~NeuroforgeAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Neuroforge"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;

    void mutate(int mode);
    void setLock(int index, bool shouldLock);
    bool getLock(int index) const;

private:
    static APVTS::ParameterLayout createParameterLayout();
    float shape(float x, int algorithm, float drive, float bias) const noexcept;
    float smooth(float current, float target, float coefficient) const noexcept;

    double currentSampleRate = 44100.0;
    std::array<juce::dsp::LinkwitzRileyFilter<float>, 2> lowPass {};
    std::array<juce::dsp::LinkwitzRileyFilter<float>, 2> lowHighPass {};
    std::array<juce::dsp::LinkwitzRileyFilter<float>, 2> midLowPass {};
    std::array<juce::dsp::LinkwitzRileyFilter<float>, 2> midHighPass {};
    std::array<float, 2> lfoPhase {};
    std::array<float, 2> toneState {};
    std::array<bool, 8> locks {};

    std::mt19937 randomEngine { 0x4E465247u };
    float smoothedMix = 1.0f;
    float smoothedOutput = 1.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuroforgeAudioProcessor)
};
