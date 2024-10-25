/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CopePhattenerAudioProcessorEditor::CopePhattenerAudioProcessorEditor (CopePhattenerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
, subFreqSliderAttachment(audioProcessor.apvts, "subFreq", subFreqSlider)
, subThresholdSliderAttachment(audioProcessor.apvts, "subThreshold", subThresholdSlider)
, subWetSliderAttachment(audioProcessor.apvts, "subWet", subWetSlider)
, noiseFreqSliderAttachment(audioProcessor.apvts, "noiseFreq", noiseFreqSlider)
, noiseThresholdSliderAttachment(audioProcessor.apvts, "noiseThreshold", noiseThresholdSlider)
, noiseWetSliderAttachment(audioProcessor.apvts, "noiseWet", noiseWetSlider)
, drySliderAttachment(audioProcessor.apvts, "dry", drySlider)
, volumeSliderAttachment(audioProcessor.apvts, "volume", volumeSlider)
{
    auto myImage = juce::ImageCache::getFromMemory(BinaryData::BG_png, BinaryData::BG_pngSize);
    if (myImage.isValid())
        logo.setImage(myImage, juce::RectanglePlacement::stretchToFit);
    else
        jassertfalse;
    
    subFreqSlider.setLookAndFeel(&myLookAndFeelCope);
    subThresholdSlider.setLookAndFeel(&myLookAndFeelCope);
    subWetSlider.setLookAndFeel(&myLookAndFeelCope);
    noiseFreqSlider.setLookAndFeel(&myLookAndFeelCope);
    noiseThresholdSlider.setLookAndFeel(&myLookAndFeelCope);
    noiseWetSlider.setLookAndFeel(&myLookAndFeelCope);
    drySlider.setLookAndFeel(&myLookAndFeelCope2);
    volumeSlider.setLookAndFeel(&myLookAndFeelCope2);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    addAndMakeVisible(logo);
    
    for ( auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    setSize (300, 525);
}

CopePhattenerAudioProcessorEditor::~CopePhattenerAudioProcessorEditor()
{
}

//==============================================================================
void CopePhattenerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void CopePhattenerAudioProcessorEditor::resized()
{
// lr, ud, lr, ud ( int x, int y, int width, int height)
logo.setBounds(0, 0, 300, 525);
    subFreqSlider.setBounds(147,166, 33,33);
    subThresholdSlider.setBounds(147,207, 33,33);
    subWetSlider.setBounds(147,248, 33,33);
    noiseFreqSlider.setBounds(202,166, 33,33);
    noiseThresholdSlider.setBounds(202,207, 33,33);
    noiseWetSlider.setBounds(202,248, 33,33);
    drySlider.setBounds(254,403, 41,41);
    volumeSlider.setBounds(254,469, 41,41);
//noiseWetSlider.setBounds(238,12, 51,51);
//subWetSlider.setBounds(239,102, 51,51);
//noiseFreqSlider.setBounds(239,191, 51,51);
//subFreqSlider.setBounds(239,276, 51,51);
}

std::vector<juce::Component*> CopePhattenerAudioProcessorEditor::getComps()
{
    return
    {
        &subFreqSlider,
        &subThresholdSlider,
        &subWetSlider,
        &noiseFreqSlider,
        &noiseThresholdSlider,
        &noiseWetSlider,
        &drySlider,
        &volumeSlider
    };
};
