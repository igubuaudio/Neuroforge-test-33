#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class NeuroforgeAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit NeuroforgeAudioProcessorEditor(NeuroforgeAudioProcessor&);
    ~NeuroforgeAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    NeuroforgeAudioProcessor& processor;
    juce::Label title, subtitle;
    juce::ComboBox personality, algorithm;
    juce::Label personalityLabel, algorithmLabel;
    juce::Slider drive, tone, mix, output, movement, metal, intensity, complexity, destruction;
    juce::Label driveLabel, toneLabel, mixLabel, outputLabel, movementLabel, metalLabel,
                intensityLabel, complexityLabel, destructionLabel;
    juce::TextButton mutateButton { "MUTATE" }, evolveButton { "EVOLVE" }, variationButton { "VARIATION" };
    std::array<juce::ToggleButton, 8> lockButtons;
    std::array<std::unique_ptr<SliderAttachment>, 9> sliderAttachments;
    std::unique_ptr<ComboAttachment> personalityAttachment, algorithmAttachment;

    void configureSlider(juce::Slider&, juce::Label&, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuroforgeAudioProcessorEditor)
};
