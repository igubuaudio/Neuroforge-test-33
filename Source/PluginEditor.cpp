#include "PluginEditor.h"

NeuroforgeAudioProcessorEditor::NeuroforgeAudioProcessorEditor(NeuroforgeAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(820, 590);
    title.setText("NEUROFORGE", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(32.0f)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffe9f1f5));
    addAndMakeVisible(title);

    subtitle.setText("BASS MUTATION SYSTEM  /  ALPHA 0.1", juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xff8da1ad));
    subtitle.setFont(juce::Font(juce::FontOptions(13.0f)));
    addAndMakeVisible(subtitle);

    configureSlider(drive, driveLabel, "DRIVE");
    configureSlider(tone, toneLabel, "TONE");
    configureSlider(mix, mixLabel, "MIX");
    configureSlider(output, outputLabel, "OUTPUT");
    configureSlider(movement, movementLabel, "MOVEMENT");
    configureSlider(metal, metalLabel, "METAL");
    configureSlider(intensity, intensityLabel, "MUTATION");
    configureSlider(complexity, complexityLabel, "COMPLEXITY");
    configureSlider(destruction, destructionLabel, "DESTRUCTION");

    const std::array<const char*, 9> ids { "drive", "tone", "mix", "output", "movement",
                                          "metal", "intensity", "complexity", "destruction" };
    std::array<juce::Slider*, 9> sliders { &drive, &tone, &mix, &output, &movement,
                                         &metal, &intensity, &complexity, &destruction };
    for (size_t i = 0; i < sliders.size(); ++i)
        sliderAttachments[i] = std::make_unique<SliderAttachment>(processor.parameters, ids[i], *sliders[i]);

    personality.addItemList({ "AUDIO", "ED RUSH", "ALIEN", "DESTRUCTION" }, 1);
    algorithm.addItemList({ "SOFT CLIP", "HARD CLIP", "ASYMMETRIC", "FOLDBACK",
                            "RECTIFY", "DIODE", "TUBE", "DIGITAL CRUSH" }, 1);
    personalityLabel.setText("PERSONALITY", juce::dontSendNotification);
    algorithmLabel.setText("DISTORTION", juce::dontSendNotification);
    addAndMakeVisible(personality);
    addAndMakeVisible(algorithm);
    addAndMakeVisible(personalityLabel);
    addAndMakeVisible(algorithmLabel);
    personalityAttachment = std::make_unique<ComboAttachment>(processor.parameters, "personality", personality);
    algorithmAttachment = std::make_unique<ComboAttachment>(processor.parameters, "algorithm", algorithm);

    auto styleButton = [](juce::TextButton& b, juce::Colour colour)
    {
        b.setColour(juce::TextButton::buttonColourId, colour);
        b.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        b.setClickingTogglesState(false);
    };
    styleButton(mutateButton, juce::Colour(0xffb6ef55));
    mutateButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff14200d));
    styleButton(evolveButton, juce::Colour(0xff344957));
    styleButton(variationButton, juce::Colour(0xff344957));
    addAndMakeVisible(mutateButton);
    addAndMakeVisible(evolveButton);
    addAndMakeVisible(variationButton);

    mutateButton.onClick = [this] { processor.mutate(4); };
    evolveButton.onClick = [this] { processor.mutate((int) *processor.parameters.getRawParameterValue("personality")); };
    variationButton.onClick = [this] { processor.mutate((int) *processor.parameters.getRawParameterValue("personality")); };

    const std::array<const char*, 8> lockNames { "ALGO", "DRIVE", "TONE", "MOVE", "METAL", "LOW", "HIGH", "STYLE" };
    for (size_t i = 0; i < lockButtons.size(); ++i)
    {
        lockButtons[i].setButtonText(juce::String("LOCK ") + lockNames[i]);
        lockButtons[i].setColour(juce::ToggleButton::textColourId, juce::Colour(0xffb8c5cc));
        addAndMakeVisible(lockButtons[i]);
        lockButtons[i].onClick = [this, i]
        {
            processor.setLock((int) i, lockButtons[i].getToggleState());
        };
    }
}

void NeuroforgeAudioProcessorEditor::configureSlider(juce::Slider& slider, juce::Label& label,
                                                      const juce::String& name)
{
    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colour(0xffaab9c2));
    label.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(label);

    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb6ef55));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffeaf4f7));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff202d36));
    addAndMakeVisible(slider);
}

void NeuroforgeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111a21));
    g.setColour(juce::Colour(0xff25343e));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(12.0f), 14.0f, 1.0f);
    g.setColour(juce::Colour(0xffb6ef55));
    g.fillRoundedRectangle(24.0f, 82.0f, 772.0f, 2.0f, 1.0f);
    g.setColour(juce::Colour(0xff72838d));
    g.setFont(12.0f);
    g.drawText("THREE-BAND PROCESSING  /  REAL-TIME MUTATION", 26, 555, 500, 18,
               juce::Justification::left);
    g.drawText("ALPHA 0.1  •  EXPERIMENTAL DSP", 570, 555, 220, 18,
               juce::Justification::right);
}

void NeuroforgeAudioProcessorEditor::resized()
{
    title.setBounds(26, 16, 340, 42);
    subtitle.setBounds(28, 54, 340, 20);

    personalityLabel.setBounds(390, 20, 130, 18);
    personality.setBounds(390, 40, 170, 28);
    algorithmLabel.setBounds(580, 20, 130, 18);
    algorithm.setBounds(580, 40, 210, 28);

    mutateButton.setBounds(26, 100, 360, 68);
    evolveButton.setBounds(398, 100, 190, 68);
    variationButton.setBounds(600, 100, 190, 68);

    const int y1 = 205, y2 = 365, w = 150, h = 130;
    std::array<juce::Slider*, 5> first { &drive, &tone, &movement, &metal, &destruction };
    std::array<juce::Label*, 5> firstLabels { &driveLabel, &toneLabel, &movementLabel, &metalLabel, &destructionLabel };
    for (int i = 0; i < 5; ++i)
    {
        const int x = 20 + i * 158;
        firstLabels[(size_t) i]->setBounds(x, y1, w, 20);
        first[(size_t) i]->setBounds(x, y1 + 18, w, h);
    }

    std::array<juce::Slider*, 4> second { &mix, &output, &intensity, &complexity };
    std::array<juce::Label*, 4> secondLabels { &mixLabel, &outputLabel, &intensityLabel, &complexityLabel };
    for (int i = 0; i < 4; ++i)
    {
        const int x = 80 + i * 178;
        secondLabels[(size_t) i]->setBounds(x, y2, w, 20);
        second[(size_t) i]->setBounds(x, y2 + 18, w, h);
    }

    for (size_t i = 0; i < lockButtons.size(); ++i)
    {
        const int row = (int) i / 4;
        const int col = (int) i % 4;
        lockButtons[i].setBounds(24 + col * 195, 500 + row * 25, 185, 22);
    }
}
