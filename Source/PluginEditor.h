/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "KnobClass.h"
#include "KnobClass2.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};

//==============================================================================
/**
*/
class CopePhattenerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CopePhattenerAudioProcessorEditor (CopePhattenerAudioProcessor&);
    ~CopePhattenerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    myLookAndFeelCope myLookAndFeelCope;
    myLookAndFeelCope2 myLookAndFeelCope2;
    CopePhattenerAudioProcessor& audioProcessor;
    juce::ImageComponent logo;
    CustomRotarySlider subFreqSlider, subThresholdSlider, subWetSlider,
                        noiseFreqSlider, noiseThresholdSlider, noiseWetSlider,
    drySlider,
    volumeSlider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment subFreqSliderAttachment, subThresholdSliderAttachment, subWetSliderAttachment,
    noiseFreqSliderAttachment, noiseThresholdSliderAttachment, noiseWetSliderAttachment,
drySliderAttachment,
volumeSliderAttachment;
    
    
    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CopePhattenerAudioProcessorEditor)
};
