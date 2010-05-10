// Copyright 2009 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Parameter editor.

#ifndef HARDWARE_SHRUTHI_EDITOR_H_
#define HARDWARE_SHRUTHI_EDITOR_H_

#include "hardware/base/base.h"
#include "hardware/shruthi/resources.h"
#include "hardware/shruthi/shruthi.h"

namespace hardware_hal {
  class KeyEvent;
}

namespace hardware_shruthi {

enum EditorMode {
  EDITOR_MODE_PATCH,
  EDITOR_MODE_SEQUENCE,
  EDITOR_MODE_PERFORMANCE,
};

enum DisplayMode {
  // Show the parameter names.
  DISPLAY_MODE_OVERVIEW = 0,
  // Show a single parameter without pesky acronyms.
  DISPLAY_MODE_EDIT = 1,
  // Show a single parameter without pesky acronyms, and revert to overview
  // mode after a while
  DISPLAY_MODE_EDIT_TEMPORARY = 2,
};

enum Group {
  GROUP_OSC,
  GROUP_FILTER,
  GROUP_MOD_SOURCES,
  GROUP_MOD_MATRIX,
  
  GROUP_SEQUENCER_ARPEGGIATOR,
  GROUP_SEQUENCER_TRACKER,
  GROUP_SEQUENCER_STEPS,
  GROUP_SYS,

  GROUP_LOAD_SAVE,
  GROUP_PERFORMANCE,
};

enum Page {
  PAGE_OSC_OSC_1,
  PAGE_OSC_OSC_2,
  PAGE_OSC_OSC_MIX,
  PAGE_FILTER_FILTER,
  PAGE_MOD_ENV_1,
  PAGE_MOD_ENV_2,
  PAGE_MOD_LFO_1,
  PAGE_MOD_LFO_2,
  PAGE_MOD_MATRIX,
  
  PAGE_SEQ_SEQUENCER,
  PAGE_SEQ_ARPEGGIATOR,
  PAGE_SEQ_TRACKER,
  PAGE_SEQ_RHYTHM,
  PAGE_SEQ_CONTROLLER,
  
  PAGE_SYS_KBD,
  PAGE_SYS_MIDI,
  PAGE_LOAD_SAVE,
  PAGE_PERFORMANCE,
};

enum LoadSavePage {
  LOAD_SAVE_PATCH,
  LOAD_SAVE_SEQUENCE,
  LOAD_SAVE_SYSTEM_SETTINGS,
};

enum Unit {
  UNIT_RAW_UINT8,
  UNIT_UINT8,
  UNIT_INT8,
  UNIT_BOOLEAN,
  UNIT_WAVEFORM,
  UNIT_SUB_OSC_WAVEFORM,
  UNIT_OPERATOR,
  UNIT_LFO_WAVEFORM,
  UNIT_LFO_RATE,
  UNIT_INDEX,
  UNIT_MODULATION_SOURCE,
  UNIT_MODULATION_DESTINATION,
  UNIT_ARPEGGIO_DIRECTION,
  UNIT_RAGA,
  UNIT_TEMPO_WITH_EXTERNAL_CLOCK,
  UNIT_MIDI_MODE,
  UNIT_SEQUENCER_MODE,
  UNIT_SEQUENCER_FLOW,
  UNIT_GROOVE_TEMPLATE,
  UNIT_ARPEGGIO_PATTERN,
  UNIT_LFO_RETRIGGER_MODE,
};

enum Action {
  ACTION_LOAD,
  ACTION_EXIT,
  ACTION_SAVE
};

// We do not use enums here because they take 2 bytes, not 1.
typedef uint8_t ParameterGroup;
typedef uint8_t ParameterPage;
typedef uint8_t ParameterUnit;

static const uint8_t kNumPages = PAGE_PERFORMANCE + 1;
static const uint8_t kNumGroups = GROUP_PERFORMANCE + 1;

// Size (in char) of the display elements.
static const uint8_t kCaptionWidth = 10;
static const uint8_t kValueWidth = 6;
static const uint8_t kColumnWidth = 4;

struct ParameterDefinition {
  uint8_t id;
  uint8_t min_value;
  uint8_t max_value;
  ParameterUnit unit;
  uint8_t short_name;
  uint8_t long_name;
};

enum PageUiType {
  PARAMETER_EDITOR,
  TRACKER_EDITOR,
  PAGE_R_EDITOR,
  STEP_SEQUENCER,
  LOAD_SAVE,
};

typedef uint8_t UiType;

struct PageDefinition {
  ParameterPage next;
  ParameterGroup group;

  // Previous and next page when cycling with encoder.
  ParameterPage overall_previous;
  ParameterPage overall_next;

  ResourceId name;
  UiType ui_type;
  uint8_t first_parameter_index;
  uint8_t leds_pattern;
};

struct ParameterAssignment {
  uint8_t id;
  uint8_t subpage;
};

// For each type of page (basic parameter editor, step sequencer, load/save
// dialog, 4 functions must be defined:
// - a function displaying the "overview" page.
// - a function displaying a specific parameter value ("details").
// - a function handling a change in one of the 4 editing pots.
// - a function handling presses on the increment/decrement buttons.
struct UiHandler {
  void (*overview_page)();
  void (*edit_page)();
  void (*input_handler)(uint8_t knob_index, uint16_t value);
  void (*increment_handler)(int8_t direction);
};

class Editor {
 public:
  Editor() { }
  static void Init();

  // Handles a press on a key.
  static void HandleKeyEvent(const hardware_hal::KeyEvent& event);

  // Handles the modification of one of the editing pots.
  static void HandleInput(uint8_t knob_index, uint16_t value);

  // Handles a rotation of the encoder.
  static void HandleIncrement(int8_t direction);

  // Handles a click on the encoder.
  static void HandleClick();

  // Displays variants of the current page.
  static void Refresh();
  
  // Notify the editor that the user has not provided any input for 2s.
  static void Relax();

  // Displays two lines of text read from a resource.
  static void DisplaySplashScreen(ResourceId first_line);

  static inline ParameterPage current_page() { return current_page_; }
  static inline uint8_t current_mode() { return editor_mode_; }
  static inline uint8_t leds_pattern() {
    return page_definition_[current_page_].leds_pattern;
  }
  static inline uint8_t cursor() { return cursor_; }
  static inline uint8_t subpage() { return subpage_; }

 private:
  static void PrettyPrintParameterValue(const ParameterDefinition& parameter,
                                        char* buffer, uint8_t width);

  // A bunch of hacks for special values/pages.
  static void SetParameterValue(uint8_t id, uint8_t value);
  static uint8_t GetParameterValue(uint8_t id);

  // Returns the parameter id of the parameter that should be edited when
  // touching knob #knob_index.
  static uint8_t KnobIndexToParameterId(uint8_t knob_index);

  static void JumpToPageGroup(uint8_t id);

  // Output and Input handling for all the different category of pages.
  static void DisplayEditOverviewPage();
  static void DisplayEditDetailsPage();
  static void HandleEditInput(uint8_t knob_index, uint16_t value);
  static void HandleEditIncrement(int8_t direction);

  static void MoveSequencerCursor(int8_t direction);
  static void HandleSequencerNavigation(uint8_t knob, uint16_t value);
  
  static void DisplayTrackerPage();
  static void HandleTrackerInput(uint8_t knob_index, uint16_t value);
  static void HandleTrackerIncrement(int8_t direction);

  static void DisplayPageRPage();
  static void HandlePageRInput(uint8_t knob_index, uint16_t value);
  static void HandlePageRIncrement(int8_t direction);

  static void DisplayStepSequencerPage();
  static void HandleStepSequencerInput(uint8_t knob_index, uint16_t value);
  static void HandleStepSequencerIncrement(int8_t direction);

  static void DisplayLoadSavePage();
  static void HandleLoadSaveInput(uint8_t knob_index, uint16_t value);
  static void EnterLoadSaveMode();
  static void HandleLoadSaveIncrement(int8_t direction);
  
  static void LoadPatch(uint8_t index);
  static void LoadSequence(uint8_t index);
  static void RandomizeParameter(uint8_t subpage, uint8_t parameter_index);
  static void RandomizePatch();
  static void RandomizeSequence();

  static const ParameterDefinition& parameter_definition(uint8_t index);

  static const PageDefinition page_definition_[];
  static const UiHandler ui_handler_[];

  // Parameter definitions are stored in program memory and need to be copied
  // in SRAM when read. This temporary storage space holds them.
  static ParameterDefinition parameter_definition_;
  static uint8_t parameter_definition_index_;

  static ParameterPage current_page_;
  static ParameterPage last_visited_page_[kNumGroups];
  static uint8_t last_visited_group_[3];
  static uint8_t display_mode_;
  static uint8_t editor_mode_;
  static uint8_t cursor_;
  static uint8_t last_visited_subpage_;

  static char line_buffer_[kLcdWidth * kLcdHeight + 1];

  static uint8_t subpage_;
  static uint8_t action_;
  static uint8_t current_patch_number_;
  static uint8_t previous_patch_number_;
  static uint8_t current_sequence_number_;
  static uint8_t previous_sequence_number_;

  static uint8_t assign_in_progress_; 
  static uint8_t test_note_playing_;
  static ParameterAssignment assigned_parameters_[kNumEditingPots];
  static ParameterAssignment parameter_to_assign_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

extern Editor editor;

}  // namespace hardware_shruthi

#endif  // HARDWARE_SHRUTHI_EDITOR_H_
