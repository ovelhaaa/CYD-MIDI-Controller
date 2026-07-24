#ifndef MIDI_UTILS_H
#define MIDI_UTILS_H

#include "common_definitions.h"

// Scale definitions
Scale scales[] = {
  {"Major", {0, 2, 4, 5, 7, 9, 11, 0}, 7},
  {"Minor", {0, 2, 3, 5, 7, 8, 10, 0}, 7},
  {"Dorian", {0, 2, 3, 5, 7, 9, 10, 0}, 7},
  {"Penta", {0, 2, 4, 7, 9, 0, 0, 0}, 5},
  {"Blues", {0, 3, 5, 6, 7, 10, 0, 0}, 6},
  {"Chrome", {0, 1, 2, 3, 4, 5, 6, 7}, 8}
};
const int NUM_SCALES = 6;

byte midiStatusWithChannel(byte status, byte channel) {
  return (status & 0xF0) | (channel & 0x0F);
}

// MIDI utility functions
void sendMIDIOnChannel(byte status, byte channel, byte data1, byte data2) {
  if (!deviceConnected) return;

  midiPacket[2] = midiStatusWithChannel(status, channel);
  midiPacket[3] = data1 & 0x7F;
  midiPacket[4] = data2 & 0x7F;
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
}

void sendMIDI(byte cmd, byte note, byte vel) {
  sendMIDIOnChannel(cmd & 0xF0, cmd & 0x0F, note, vel);
}

void sendNote(byte channel, byte note, byte velocity, bool on) {
  sendMIDIOnChannel(on ? 0x90 : 0x80, channel, note, on ? velocity : 0);
}

void sendCC(byte channel, byte cc, byte value) {
  sendMIDIOnChannel(0xB0, channel, cc, value);
}

void sendPitchBend(byte channel, int value) {
  value = constrain(value, 0, 16383);
  sendMIDIOnChannel(0xE0, channel, value & 0x7F, (value >> 7) & 0x7F);
}

int getNoteInScale(int scaleIndex, int degree, int octave) {
  if (scaleIndex >= NUM_SCALES) scaleIndex = 0;
  
  // If degree exceeds scale notes, wrap to next octave
  int actualDegree = degree % scales[scaleIndex].numNotes;
  int octaveOffset = degree / scales[scaleIndex].numNotes;
  
  int rootNote = 60 + performance.root; // C4 transposed by global root
  return rootNote + scales[scaleIndex].intervals[actualDegree] + ((octave - 4 + octaveOffset) * 12);
}

String getNoteNameFromMIDI(int midiNote) {
  String noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  int noteIndex = midiNote % 12;
  int octave = (midiNote / 12) - 1;
  return noteNames[noteIndex] + String(octave);
}

String getPitchClassName(int pitchClass) {
  String noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  return noteNames[(pitchClass + 12) % 12];
}

String getRootName() {
  return getPitchClassName(performance.root);
}

void setGlobalScale(int scaleIndex) {
  performance.scale = constrain(scaleIndex, 0, NUM_SCALES - 1);
}

void nudgeGlobalScale(int amount) {
  performance.scale = (performance.scale + amount + NUM_SCALES) % NUM_SCALES;
}

void nudgeGlobalRoot(int semitones) {
  performance.root = (performance.root + semitones + 12) % 12;
}

void nudgeGlobalBpm(int amount) {
  performance.bpm = constrain(performance.bpm + amount, 40, 240);
}

void stopChannelNotes(byte channel) {
  sendCC(channel, 64, 0);   // Sustain off
  sendCC(channel, 120, 0);  // All sound off
  sendCC(channel, 123, 0);  // All notes off
  for (int note = 0; note < 128; note++) {
    sendNote(channel, note, 0, false);
  }
}

void stopAllModes() {
  stopChannelNotes(performance.keysChannel);
  stopChannelNotes(performance.chordsChannel);
  stopChannelNotes(performance.arpChannel);
  stopChannelNotes(performance.generativeChannel);
  stopChannelNotes(performance.drumsChannel);
}

#endif
