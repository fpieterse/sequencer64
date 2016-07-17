#ifndef SEQ64_PERFORM_HPP
#define SEQ64_PERFORM_HPP

/*
 *  This file is part of seq24/sequencer64.
 *
 *  seq24 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  seq24 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with seq24; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * \file          perform.hpp
 *
 *  This module declares/defines the base class for handling many facets
 *  of performing (playing) a full MIDI song.
 *
 * \library       sequencer64 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2016-07-16
 * \license       GNU GPLv2 or above
 *
 *  This class still has way too many members, even with the JACK and
 *  key-binding support moved to separate modules.  Items that could
 *  profitably be partitioned into separate modules are:
 *
 *      -   Mute-group support.
 *      -   MIDI control support.
 *      -   The remaining portions of trigger support.
 */

#include <vector>                       /* std::vector                      */
#include <pthread.h>                    /* pthread_t C structure            */

#include "globals.h"                    /* globals, nullptr, & more         */
#include "jack_assistant.hpp"           /* optional seq64::jack_assistant   */
#include "gui_assistant.hpp"            /* seq64::gui_assistant             */
#include "keys_perform.hpp"             /* seq64::keys_perform              */
#include "mastermidibus.hpp"            /* seq64::mastermidibus             */
#include "midi_control.hpp"             /* seq64::midi_control "struct"     */
#include "sequence.hpp"                 /* seq64::sequence                  */

/**
 *  Try to highlight the selected pattern using black-on-cyan
 *  coloring, in addition to the red progress bar marking that already exists.
 *  Moved from seqmenu.  Seems to work pretty well now.
 */

#define SEQ64_EDIT_SEQUENCE_HIGHLIGHT

/**
 *  We have offloaded the keybinding support to another class, derived
 *  from keys_perform.  These macros make the code easier to read, or a least
 *  shorter.  :-)
 */

#define PERFKEY(x)              m_mainperf->keys().x()
#define PERFKEY_ADDR(x)         m_mainperf->keys().at_##x()

/**
 *  Uses a function returning a reference.  These macros make the code easier
 *  to read, or a least shorter.  :-)
 */

#define PREFKEY(x)              perf().keys().x()
#define PREFKEY_ADDR(x)         perf().keys().at_##x()

/**
 *  Used in the options module to indicate a "key-labels-on-sequence" setting.
 *  Kind of weird, but we'll follow it blindly in adding the new
 *  "num-labels-on-sequence" setting, since it allows for immediate updating
 *  of the user-interface when the File / Options / Keyboard / Show Keys or
 *  Show Sequence Number settings change.
 */

#define PERFORM_KEY_LABELS_ON_SEQUENCE  9998
#define PERFORM_NUM_LABELS_ON_SEQUENCE  9999

namespace seq64
{

class keystroke;

/**
 *      Provides for notification of events.  Provide a response to a
 *      group-learn change event.
 */

struct performcallback
{

/*
 * ca 2015-07-24
 * Eliminate this annoying warning.  Will do it for Microsoft's bloddy
 * compiler later.  Actually, this pragma affect any module that includes this
 * header file.
 */

#ifdef PLATFORM_GNU
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

    /**
     *  A do-nothing callback.  "state" is an Unused parameter.
     */

    virtual void on_grouplearnchange (bool /* state */)
    {
        // Empty body
    }

};

/**
 *  This class supports the performance mode.  It has way too many data
 *  members, one of them public.  Might be ripe for refactoring.  That has its
 *  own dangers, of course.
 */

class perform
{

    friend class jack_assistant;
    friend class keybindentry;
    friend class midifile;
    friend class optionsfile;           // needs cleanup
    friend class options;

#ifdef SEQ64_JACK_SUPPORT

    friend int jack_sync_callback       // accesses perform::inner_start()
    (
        jack_transport_state_t state,
        jack_position_t * pos,
        void * arg
    );

#endif

private:

    /**
     *  Provides a dummy, inactive midi_control object to handle
     *  out-of-range midi_control indicies.
     */

    static midi_control sm_mc_dummy;

    /**
     *  Support for a wide range of GUI-related operations.
     */

    gui_assistant & m_gui_support;

    /**
     *  Mute group support.  This value determines whether a particular track
     *  will be muted or unmuted, and it can handle all tracks available in
     *  the application (currently 1024).  Note that the current state of
     *  playing can be "learned", and stored herein as the desired state for
     *  the track.
     */

    bool m_mute_group[c_gmute_tracks];

    /**
     *  Holds the current mute states of each track.  Unlike the
     *  m_mute_group[] array, this holds the current state, rather than the
     *  state desired by activating a mute group, and it applies to only one
     *  screen-set.
     */

    bool m_tracks_mute_state[c_seqs_in_set];

    /**
     *  If true, indicates that a mode group is selected, and playing statuses
     *  will be "memorized".  This value starts out true.  It is altered by
     *  the c_midi_control_mod_gmute handler or when the keys().group_off()
     *  or the keys().group_on() keys are struck.
     */

    bool m_mode_group;

    /**
     *  If true, indicates that a group learn is selected, which also
     *  "memorizes" a mode group, and notifies subscribers of a group-learn
     *  change.
     */

    bool m_mode_group_learn;

    /**
     *  Selects a group to mute.  It seems like a "group" is essentially a
     *  "set" that is selected for the saving and restoring of the status of
     *  all patterns in that set.
     */

    int m_mute_group_selected;

    /**
     *  Playing screen support.  In seq24, this value is altered by
     *  set_playing_screenset(), which is called by
     *  handle_midi_control(c_midi_control_play_ss, state).
     */

    int m_playing_screen;

    /**
     *  Playing screen sequence number offset.  Saves some multiplications,
     *  should make the code easier to grok, and centralizes the use of
     *  c_seqs_in_set, which we want to be able to change at run-time, as a
     *  future enhancement.
     */

    int m_playscreen_offset;

    /**
     *  Provides a "vector" of patterns/sequences.
     *
     * \todo
     *      First, make the sequence array a vector, and second, put allof
     *      these flags into a structure and access those members indirectly.
     */

    sequence * m_seqs[c_max_sequence];

    /**
     *  Each boolean value in this array is set to true if a sequence is
     *  active, meaning that it will be used to hold some kind of MIDI data,
     *  even if only Meta events.  This array can have "holes" with inactive
     *  sequences, so every sequence needs to be checked before using it.
     */

    bool m_seqs_active[c_max_sequence];

    /**
     *  Each boolean value in this array is set to true if a sequence was
     *  active, meaning that it was found to be active at the time we were
     *  setting it to inactive.  This value seems to be used only in
     *  maintaining dirtiness-status; did some process modify the sequence?
     *  Was it's mute/unmute status changed?
     */

    bool m_was_active_main[c_max_sequence];

    /**
     *  Each boolean value in this array is set to true if a sequence was
     *  active, meaning that it was found to be active at the time we were
     *  setting it to inactive.  This value seems to be used only in
     *  maintaining dirtiness-status for editing the mute/unmute status during
     *  pattern editing.
     */

    bool m_was_active_edit[c_max_sequence];

    /**
     *  Each boolean value in this array is set to true if a sequence was
     *  active, meaning that it was found to be active at the time we were
     *  setting it to inactive.  This value seems to be used only in
     *  maintaining dirtiness-status for editing the mute/unmute status during
     *  performance/song editing.
     */

    bool m_was_active_perf[c_max_sequence];

    /**
     *  Each boolean value in this array is set to true if a sequence was
     *  active, meaning that it was found to be active at the time we were
     *  setting it to inactive.  This value seems to be used only in
     *  maintaining dirtiness-status for editing the mute/unmute status during
     *  performance names editing.  Not sure that it serves a real purpose;
     *  perhaps created with an eye to editing the pattern name in the song
     *  editor?
     */

    bool m_was_active_names[c_max_sequence];

    /**
     *  Saves the current playing state of each pattern.
     */

    bool m_sequence_state[c_max_sequence];

    /**
     *  Provides our MIDI buss.
     */

    mastermidibus m_master_bus;

#ifdef SEQ64_STAZED_TRANSPOSE

    /**
     *  Holds the global MIDI transposition value.
     */

    int m_transpose;

#endif

private:

    /**
     *  Provides information for managing pthreads.  Provides a "handle" to
     *  the output thread.
     */

    pthread_t m_out_thread;

    /**
     *  Provides a "handle" to the input thread.
     */

    pthread_t m_in_thread;

    /**
     *  Indicates that the output thread has been started.
     */

    bool m_out_thread_launched;

    /**
     *  Indicates that the input thread has been started.
     */

    bool m_in_thread_launched;

    /*
     *  Indicates that playback is running.
     */

    bool m_running;

    /**
     *  Indicates that events are being written to the MIDI input busses in
     *  the input thread.
     */

    bool m_inputing;

    /**
     *  Indicates that events are being written to the MIDI output busses in
     *  the output thread.
     */

    bool m_outputing;

    /**
     *  Indicates that status of the "loop" button in the performance editor.
     *  If true, the performance will loop between the L and R markers in the
     *  performance editor.
     */

    bool m_looping;

    /**
     *  Specifies the playback mode.  There are two, "live" and "song",
     *  indicated by the following values:
     *
\verbatim
        m_playback_mode == false:       live mode
        m_playback_mode == true:        playback/song mode
\endverbatim
     *
     */

    bool m_playback_mode;

    /**
     *  Holds the current PPQN for usage in various actions.
     */

    int m_ppqn;

    /**
     *  Holds the beats/bar value as obtained from the MIDI file.
     *  The default value is SEQ64_DEFAULT_BEATS_PER_MEASURE (4).
     */

    int m_beats_per_bar;

    /**
     *  Holds the beat width value as obtained from the MIDI file.
     *  The default value is SEQ64_DEFAULT_BEAT_WIDTH (4).
     */

    int m_beat_width;

    /**
     *  Holds the "one measure's worth" of pulses (ticks), which is
     *  normally m_ppqn * 4.  We can save some multiplications, and, more
     *  importantly, later define a more flexible definition of "one measure's
     *  worth" than simply four quarter notes.
     */

    midipulse m_one_measure;

    /**
     *  Holds the position of the left (L) marker, and it is first defined as
     *  0.  Note that "tick" is actually "pulses".
     */

    midipulse m_left_tick;

    /**
     *  Holds the position of the right (R) marker, and it is first defined as
     *  the end of the fourth measure.  Note that "tick" is actually "pulses".
     */

    midipulse m_right_tick;

    /**
     *  Holds the starting tick for playing.  By default, this value is always
     *  reset to the value of the "left tick".  We want to eventually be able
     *  to leave it at the last playing tick, to support a "pause"
     *  functionality. Note that "tick" is actually "pulses".
     */

    midipulse m_starting_tick;

    /**
     *  MIDI Clock support.  The m_tick member holds the tick to be used in
     *  displaying the progress bars and the maintime pill.  It is mutable
     *  because sometimes we want to adjust it in a const function for pause
     *  functionality.
     */

    mutable midipulse m_tick;

#ifdef SEQ64_PAUSE_SUPPORT

    /**
     *  Let's try to save the last JACK pad structure tick for re-use with
     *  resume after pausing.
     */

    midipulse m_jack_tick;

#endif

    /**
     *  More MIDI clock support.
     */

    bool m_usemidiclock;

    /**
     *  More MIDI clock support.
     */

    bool m_midiclockrunning;            // stopped or started

    /**
     *  More MIDI clock support.
     */

    int m_midiclocktick;

    /**
     *  More MIDI clock support.
     */

    int m_midiclockpos;

    /**
     *  Support for pause, which does not reset the "last tick" when playback
     *  stops/starts.
     */

    bool m_is_paused;

private:

    /**
     *  Used in the mainwnd class to set the notepad text for the given set.
     */

    std::string m_screen_set_notepad[c_max_sets];

    /**
     *  Provides the settings of MIDI Toggle, as read from the "rc" file.
     */

    midi_control m_midi_cc_toggle[c_midi_controls];

    /**
     *  Provides the settings of MIDI On, as read from the "rc" file.
     */

    midi_control m_midi_cc_on[c_midi_controls];

    /**
     *  Provides the settings of MIDI Off, as read from the "rc" file.
     */

    midi_control m_midi_cc_off[c_midi_controls];

    /**
     *  Holds the current offset into the screen-sets.  It is used in the MIDI
     *  control of the playback status of the sequences in the current
     *  screen-set.  It is also used to offset the sequence numbers so that
     *  the control (mute/unmute) keys can be shown on any screen-set.
     */

    int m_offset;

    /**
     *  Holds the OR'ed control status values.  Need to learn more about this
     *  one.  It is used in the replace, snapshot, and queue functionality.
     */

    int m_control_status;

    /**
     *  Indicates the number of the currently-selected screen-set.  This is
     *  merely the screen-set that is in view.  The fix of tdeagan substitutes
     *  the "in-view" screen-set for the "playing" screen-set.
     */

    int m_screenset;

    /**
     *  New.  Attempting to provide a feature where moving to another
     *  screenset automatically cues the current screenset for turning off,
     *  and the new screenset for turning on.  EXPERIMENTAL.  Will be a menu
     *  option once it works.
     */

    bool m_auto_screenset_queue;

    /**
     *  We will eventually replace c_seqs_in_set with this member, which
     *  defaults to the value of c_seqs_in_set.  This change will require some
     *  arrays to be dynamically allocated (vectors).
     */

    int m_seqs_in_set;                  /* replaces global c_seqs_in_set    */

    /**
     *  A replacement for the c_max_sets constant.  Again, currently set to
     *  the old value, which is used in hard-wired array sizes.  To make it
     *  variable will require a move from arrays to vectors.
     */

    int m_max_sets;

    /**
     *  Keeps track of created sequences, whether or not they are active.
     *  Used by the install_sequence() function.  Note that this value is
     *  not a suitable replacement for c_max_sequence/m_sequence_max, because
     *  there can be inactive sequences amidst the active sequences.
     */

    int m_sequence_count;

    /**
     *  A replacement for the c_max_sequence constant.  However, this value is
     *  already 32 * 32 = 1024, and is probably enough for any usage.  Famous
     *  last words?
     */

    int m_sequence_max;

#ifdef SEQ64_EDIT_SEQUENCE_HIGHLIGHT

    /**
     *  Hold the number of the currently-in-edit sequence.  Moving this
     *  status from seqmenu into perform for better centralized management.
     */

    int m_edit_sequence;

#endif

    /**
     *  It may be a good idea to eventually centralize all of the dirtiness of
     *  a performance here.  All the GUIs seem to use a perform object.
     */

    bool m_is_modified;

    /**
     *  A condition variable to protect playback.  It is signalled if playback
     *  has been started.  The output thread function waits on this variable
     *  until m_running and m_outputing are false.  This variable is also
     *  signalled in the perform destructor.
     */

    condition_var m_condition_var;

#ifdef SEQ64_JACK_SUPPORT

    /**
     *  A wrapper object for the JACK support of this application.
     */

    jack_assistant m_jack_asst;         // implements most of the JACK stuff

#endif

    /*
     *  Can register here for events.  Used in mainwnd and perform.
     *  Now wrapped in the enregister() function, so no longer public.
     */

    std::vector<performcallback *> m_notify;

public:

    perform (gui_assistant & mygui, int ppqn = SEQ64_USE_DEFAULT_PPQN);
    ~perform ();

    /**
     * \getter m_is_modfied
     */

    bool is_modified () const
    {
        return m_is_modified;
    }

    /**
     * \setter m_is_modified
     *      This setter only sets the modified-flag to true.
     *      The setter that will, is_modified(), is private.  No one but
     *      perfrom and its friends should falsify this flag.
     */

    void modify ()
    {
        m_is_modified = true;
    }

    /**
     * \getter m_sequence_count
     *      It is better to call this getter before bothering to even try to
     *      use a sequence.  In many cases at startup, or when loading a file,
     *      there are no sequences yet, and still the code calls functions
     *      that try to access them.
     */

    int sequence_count () const
    {
        return m_sequence_count;
    }

    /**
     * \getter m_sequence_max
     */

    int sequence_max () const
    {
        return m_sequence_max;
    }

    /**
     * \getter m_control_status
     *
     * \return
     *      Returns true if the m_control_status value is non-zero, which
     *      means that there is a queue, replace, or snapshot functionality in
     *      progress. 
     */

    bool is_control_status () const
    {
        return m_control_status != 0;
    }

#ifdef SEQ64_EDIT_SEQUENCE_HIGHLIGHT

    /**
     * \setter m_edit_sequence
     *
     * \param seqnum
     *      Pass in -1 to disable the edit-sequence number unconditionally.
     *      Use unset_edit_sequence() to disable it if it matches the current
     *      edit-sequence number.
     */

    void set_edit_sequence (int seqnum)
    {
        m_edit_sequence = seqnum;
    }

    /**
     * \setter m_edit_sequence
     *
     *      Disables the edit-sequence number if it matches the parameter.
     *
     * \param seqnum
     *      The sequence number of the sequence to unset.
     */

    void unset_edit_sequence (int seqnum)
    {
        if (is_edit_sequence(seqnum))
            set_edit_sequence(-1);
    }

    /**
     * \getter m_edit_sequence
     *
     * \param seqnum
     *      Tests the parameter against m_edit_sequence.  Returns true
     *      if that member is not -1, and the parameter matches it.
     */

    bool is_edit_sequence (int seqnum) const
    {
        return (m_edit_sequence != (-1)) && (seqnum == m_edit_sequence);
    }

#endif  // SEQ64_EDIT_SEQUENCE_HIGHLIGHT

    /**
     * \getter m_beats_per_bar
     */

    int get_beats_per_bar () const
    {
        return m_beats_per_bar;
    }

    /**
     * \setter m_beats_per_bar
     *
     * \param bpm
     *      Provides the value for beats/measure.  Also used to set the
     *      beats/measure in the JACK assistant object.
     */

    void set_beats_per_bar (int bpm)
    {
        m_beats_per_bar = bpm;
#ifdef SEQ64_JACK_SUPPORT
        m_jack_asst.set_beats_per_measure(bpm);
#endif
    }

    /**
     * \getter m_beat_width
     */

    int get_beat_width () const
    {
        return m_beat_width;
    }

    /**
     * \setter m_beat_width
     *
     * \param bw
     *      Provides the value for beat-width.  Also used to set the
     *      beat-width in the JACK assistant object.
     */

    void set_beat_width (int bw)
    {
        m_beat_width = bw;
#ifdef SEQ64_JACK_SUPPORT
        m_jack_asst.set_beat_width(bw);
#endif
    }

    /**
     * \getter m_gui_support
     *      The const getter.
     */

    const gui_assistant & gui () const
    {
        return m_gui_support;
    }

    /**
     * \getter m_gui_support
     *      The un-const getter.
     */

    gui_assistant & gui ()
    {
        return m_gui_support;
    }

    /**
     * \getter m_gui_support.keys()
     *      The const getter.
     */

    const keys_perform & keys () const
    {
        return gui().keys();
    }

    /**
     * \getter m_gui_support.keys()
     *      The un-const getter.
     */

    keys_perform & keys ()
    {
        return gui().keys();
    }

    /**
     * \getter m_master_bus
     */

    mastermidibus & master_bus ()
    {
        return m_master_bus;
    }

    /**
     * \getter m_running
     *      Could also be called "is_playing()".
     */

    bool is_running () const
    {
        return m_running;
    }

    /**
     * \getter m_jack_asst.is_running()
     *      This function is useful for announcing the status of JACK in
     *      user-interface items that only have access to the perform object.
     */

    bool is_jack_running () const
    {
        return m_jack_asst.is_running();
    }

    /**
     * \getter m_is_paused
     */

    bool is_paused () const
    {
        return m_is_paused;
    }

    /**
     * \getter m_is_paused and ! m_jack_asst.is_running()
     *      We might just make this internal.
     */

    bool is_pausable () const
    {
        return m_is_paused && ! m_jack_asst.is_running();
    }

    /*
     * \getter m_mode_group_learn
     *  Redundant, use is_group_learn().

    bool is_learn_mode () const
    {
        return m_mode_group_learn;
    }
     */

    /**
     *  Adds a pointer to an object to be notified by this perform object.
     *
     * \param pfcb
     *      Provides the pointer to the performance callback.
     */

    void enregister (performcallback * pfcb)
    {
        if (not_nullptr(pfcb))
            m_notify.push_back(pfcb);
    }

public:

    void clear_all ();
    void launch (int ppqn);
    void new_sequence (int seq);                    /* seqmenu & mainwid    */
    void add_sequence (sequence * seq, int perf);   /* midifile             */
    void delete_sequence (int seq);                 /* seqmenu & mainwid    */
    bool is_sequence_in_edit (int seq);
    void clear_sequence_triggers (int seq);
    void print_triggers () const;

    /**
     *  The rough opposite of launch(); it doesn't stop the threads.  A minor
     *  simplification for the main() routine, hides the JACK support macro.
     */

    void finish ()
    {
        deinit_jack();
    }

    /**
     * \getter m_tick
     */

    midipulse get_tick () const
    {
        return m_tick;
    }

#ifdef SEQ64_PAUSE_SUPPORT

    /**
     * \getter m_jack_tick
     */

    midipulse get_jack_tick () const
    {
        return m_jack_tick;
    }

    /**
     * \setter m_jack_tick
     *
     * \param tick
     *      Provides the current JACK tick (pulse) value to set.
     */

    void set_jack_tick (midipulse tick)
    {
        m_jack_tick = tick;
    }

#endif

    void set_left_tick (midipulse tick, bool setstart = true);

    /**
     * \getter m_left_tick
     */

    midipulse get_left_tick () const
    {
        return m_left_tick;
    }

    /**
     * \setter m_starting_tick
     *
     * \param tick
     *      Provides the starting JACK tick (pulse) value to set.
     */

    void set_start_tick (midipulse tick)
    {
        m_starting_tick = tick;
    }

    /*
     * Obsolete:  midipulse get_max_tick () const;
     */

    void set_right_tick (midipulse tick, bool setstart = true);

    /**
     * \getter m_right_tick
     */

    midipulse get_right_tick () const
    {
        return m_right_tick;
    }

    void move_triggers (bool direction);
    void copy_triggers ();
    void push_trigger_undo ();
    void pop_trigger_undo ();
    void split_trigger (int seqnum, midipulse tick);
    midipulse get_max_trigger ();

    /**
     *  Convenience function for perfedit's collapse functionality.
     */

    void collapse ()
    {
        push_trigger_undo();
        move_triggers(false);
        is_modified(true);
    }

    /**
     *  Convenience function for perfedit's copy functionality.
     */

    void copy ()
    {
        push_trigger_undo();
        copy_triggers();
    }

    /**
     *  Convenience function for perfedit's expand functionality.
     */

    void expand ()
    {
        push_trigger_undo();
        move_triggers(true);
        is_modified(true);
    }

    midi_control & midi_control_toggle (int seq);
    midi_control & midi_control_on (int seq);
    midi_control & midi_control_off (int seq);
    void handle_midi_control (int control, bool state);
    const std::string & get_screen_set_notepad (int screen_set) const;

    /**
     *  Returns the notepad text for the current screen-set.
     */

    const std::string & current_screen_set_notepad () const
    {
        return get_screen_set_notepad(m_screenset);
    }

    void set_screen_set_notepad (int screenset, const std::string & note);

    /**
     *  Sets the notepad text for the current screen-set.
     *
     * \param note
     *      The string value to set into the notepad text.
     */

    void set_screen_set_notepad (const std::string & note)
    {
        set_screen_set_notepad(m_screenset, note);
    }

    /**
     * \getter m_screenset
     */

    int get_screenset () const
    {
        return m_screenset;
    }

    void set_playing_screenset ();
    void set_screenset (int ss);
    void swap_screenset_queues (int ss0, int ss1);

    /**
     * \getter m_playing_screen
     */

    int get_playing_screenset () const
    {
        return m_playing_screen;
    }

    bool any_group_unmutes () const;
    void mute_group_tracks ();
    void select_and_mute_group (int g_group);

    /**
     * \setter m_mode_group
     */

    void set_mode_group_mute ()
    {
        m_mode_group = true;
    }

    /**
     * \setter m_mode_group
     *      Unsets this member.
     */

    void unset_mode_group_mute ()
    {
        m_mode_group = false;
    }

    void select_group_mute (int g_mute);
    void set_mode_group_learn ();
    void unset_mode_group_learn ();

    bool is_group_learning ()
    {
        return m_mode_group_learn;
    }

    void set_and_copy_mute_group (int group);
    void start (bool state);
    void stop ();

    /**
     *  If JACK is supported, starts the JACK transport.
     */

    void start_jack ()
    {
#ifdef SEQ64_JACK_SUPPORT
        m_jack_asst.start();
#endif
    }

    /**
     *  If JACK is supported, stops the JACK transport.
     */

    void stop_jack ()
    {
#ifdef SEQ64_JACK_SUPPORT
        m_jack_asst.stop();
#endif
    }

    void position_jack (bool state);
    void off_sequences ();
    void all_notes_off ();
    void set_active (int seq, bool active);
    void set_was_active (int seq);
    bool is_dirty_main (int seq);
    bool is_dirty_edit (int seq);
    bool is_dirty_perf (int seq);
    bool is_dirty_names (int seq);

    /**
     *  Checks the pattern/sequence for activity.
     *
     * \todo
     *      We should have the sequence object keep track of its own activity
     *      and access that via a reference or pointer.
     *
     * \param seq
     *      The pattern number.  It is checked for invalidity.  This can
     *      lead to "too many" (i.e. redundant) checks, but we're trying to
     *      centralize such checks in this function.
     *
     * \return
     *      Returns the value of the active-flag, or false if the sequence was
     *      invalid or null.
     */

    bool is_active (int seq) const
    {
        return is_mseq_valid(seq) ? m_seqs_active[seq] : false ;
    }

    /**
     *  Retrieves the actual sequence, based on the pattern/sequence number.
     *
     * \param seq
     *      The prospective sequence number.
     *
     * \return
     *      Returns the value of m_seqs[seq] if seq is valid.  Otherwise, a
     *      null pointer is returned.
     */

    sequence * get_sequence (int seq)
    {
        return is_mseq_valid(seq) ? m_seqs[seq] : nullptr ;
    }

    void reset_sequences (bool pause = false);

    /**
     *  Plays all notes to the current tick.
     */

    void play (midipulse tick);
    void set_orig_ticks (midipulse tick);
    void set_beats_per_minute (int bpm);        /* more than just a setter  */

    /**
     * \getter m_master_bus.get_beats_per_minute
     *      Retrieves the BPM setting of the master MIDI buss.
     *
     * \return
     *      Returns the value of beats/minute from the master buss.
     */

    int get_beats_per_minute ()
    {
        return m_master_bus.get_beats_per_minute();
    }

    /**
     * \setter m_looping
     *
     * \param looping
     *      The boolean value to set for looping, used in the performance
     *      editor.
     */

    void set_looping (bool looping)
    {
        m_looping = looping;
    }

#ifdef SEQ64_STAZED_TRANSPOSE

    void apply_song_transpose ();

    /**
     * \setter m_transpose
     */

    void set_transpose (int transpose)
    {
        m_transpose = transpose;
    }

    /**
     * \getter m_transpose
     */

    int get_transpose () const
    {
        return m_transpose;
    }

    /**
     *  Gets the transposition value stored in the master MIDI buss.
     *  A convenience function.  We've moved the transpose variable into
     *  perform, where it belongs.

    int get_midi_transpose () const
    {
        return m_master_bus->get_transpose();
    }
     */

    /**
     *  Sets the transposition value in the master MIDI buss.

    void set_midi_transpose (int transpose)
    {
        m_master_bus->set_transpose(transpose);
    }
     */

#endif

    void set_sequence_control_status (int status);
    void unset_sequence_control_status (int status);
    void sequence_playing_toggle (int seq);
    void sequence_playing_change (int seq, bool on);

    /**
     *  Calls sequence_playing_change() with a value of true.
     *
     * \param seq
     *      The sequence number of the sequence to turn on.
     */

    void sequence_playing_on (int seq)
    {
        sequence_playing_change(seq, true);
    }

    /**
     *  Calls sequence_playing_change() with a value of false.
     *
     * \param seq
     *      The sequence number of the sequence to turn off.
     */

    void sequence_playing_off (int seq)
    {
        sequence_playing_change(seq, false);
    }

    void mute_all_tracks (bool flag = true);
    void output_func ();
    void input_func ();

    /**
     *  This function sets the mute state of an element in the m_mute_group
     *  array.  The index value is the track number offset by the number of
     *  the selected mute group (which is equivalent to a set number) times
     *  the number of sequences in a set.  This function is used in midifile
     *  and optionsfile when parsing the file to get the initial mute-groups.
     *
     * \param gtrack
     *      The number of the track to be muted/unmuted.
     *
     * \param muted
     *      This boolean indicates the state to which the track should be set.
     */

    void set_group_mute_state (int gtrack, bool muted)
    {
        m_mute_group[mute_group_offset(gtrack)] = muted;
    }

    /**
     *  The opposite of set_group_mute_state(), it gets the value of the
     *  desired track.  Uses the mute_group_offset() function.  This function
     *  is used in midifile and optionsfile when writing the file to get the
     *  initial mute-groups.
     *
     * \param gtrack
     *      The number of the track for which the state is to be obtained.
     *      Like set_group_mute_state(), this value is offset by adding
     *      m_mute_group_selected * m_seqs_in_set.
     *
     * \return
     *      Returns the desired m_mute_group[] value.
     */

    bool get_group_mute_state (int gtrack)
    {
        return m_mute_group[mute_group_offset(gtrack)];
    }

    /**
     *  Calculates the offset into the screen sets.
     *  Sets <code>m_offset = offset * c_mainwnd_rows * c_mainwnd_cols</code>.
     *
     * \param offset
     *      The desired offset.
     */

    void set_offset (int offset)
    {
        m_offset = offset * c_mainwnd_rows * c_mainwnd_cols;
    }

    /**
     * \getter m_offset
     */

    int get_offset () const
    {
        return m_offset;
    }

    void save_playing_state ();
    void restore_playing_state ();

    /**
     * Here follows a few forwarding functions for the keys_perform-derived
     * classes.
     *
     * \param k
     *      The key number for which to return the string name of the key.
     */

    std::string key_name (unsigned int k) const
    {
        return keys().key_name(k);
    }

    /**
     *  Forwarding function for key events.
     */

    keys_perform::SlotMap & get_key_events ()
    {
        return keys().get_key_events();
    }

    /**
     *  Forwarding function for key groups.
     */

    keys_perform::SlotMap & get_key_groups ()
    {
        return keys().get_key_groups();
    }

    /**
     *  Forwarding function for reverse key events.
     */

    keys_perform::RevSlotMap & get_key_events_rev ()
    {
        return keys().get_key_events_rev();
    }

    /**
     *  Forwarding function for reverse key groups.
     */

    keys_perform::RevSlotMap & get_key_groups_rev ()
    {
        return keys().get_key_groups_rev();
    }

    /**
     * \getter m_show_ui_sequency_key
     *      Provides access to keys().show_ui_sequence_key().
     *      Used in mainwid, options, optionsfile, userfile, and perform.
     */

    bool show_ui_sequence_key () const
    {
        return keys().show_ui_sequence_key();
    }

    /**
     * \setter m_show_ui_sequency_key
     *
     * \param flag
     *      Provides the flag to set into keys().show_ui_sequence_key().
     */

    void show_ui_sequence_key (bool flag)
    {
        keys().show_ui_sequence_key(flag);
    }

    /**
     * \getter m_show_ui_sequency_number
     *      Provides access to keys().show_ui_sequence_number().
     *      Used in mainwid, optionsfile, and perform.
     */

    bool show_ui_sequence_number () const
    {
        return keys().show_ui_sequence_number();
    }

    /**
     * \getter m_show_ui_sequency_number
     *
     * \param flag
     *      Provides the value to set into keys().show_ui_sequence_number().
     */

    void show_ui_sequence_number (bool flag)
    {
        keys().show_ui_sequence_number(flag);
    }

    /*
     * Getters of keyboard mapping for sequence and groups.
     * If not found, returns something "safe" [so use get_key()->count()
     * to see if it's there first]
     */

    unsigned int lookup_keyevent_key (int seqnum);

    /**
     *  Gets the sequence number for the given event key.  The inverse of
     *  lookup_keyevent_key().
     *
     * \param keycode
     *      The number of the event key for which to return the configured
     *      sequence number.
     *
     * \return
     *      Returns the desired sequence.  If there is no such value, then
     *      a sequence number of 0 is returned.
     */

    long lookup_keyevent_seq (unsigned int keycode)
    {
        long result = 0;
        if (get_key_events().count(keycode) > 0)
            result = get_key_events()[keycode];

        return result;
    }

    /**
     *  Gets the group key for the given sequence.
     *
     * \param groupnum
     *      The number of the sequence for which to return the group key.
     *
     * \return
     *      Returns the desired key.  If there is no such value, then the
     *      period ('.') character is returned.
     */

    unsigned int lookup_keygroup_key (long groupnum)
    {
        unsigned int result = '.';                      /* '?' */
        if (get_key_groups_rev().count(groupnum) > 0)
            result = get_key_groups_rev()[groupnum];

        return result;
    }

    /**
     *  Gets the group number for the given group key.  The inverse of
     *  lookup_keygroup_key().
     *
     * \param keycode
     *      The number of the group key for which to return the configured
     *      sequence number.
     *
     * \return
     *      Returns the desired group number.  If there is no such value, then
     *      a group number of 0 is returned.
     */

    long lookup_keygroup_group (unsigned int keycode)
    {
        long result = 0;
        if (get_key_groups().count(keycode))
            result = get_key_groups()[keycode];

        return result;
    }

    void start_playing (bool songmode = false);
    void pause_playing ();
    void stop_playing ();
    void start_key (bool songmode = false);
    void pause_key (bool songmode = false);
    void stop_key ();

    /**
     *  Encapsulates some calls used in mainwnd.
     */

    void learn_toggle ()
    {
        if (is_group_learning())
            unset_mode_group_learn();
        else
            set_mode_group_learn();
    }

    /**
     *  Encapsulates some calls used in mainwnd.  Actually does a lot of
     *  work in those function calls.
     */

    int decrement_beats_per_minute ()
    {
        int result = get_beats_per_minute() - 1;
        set_beats_per_minute(result);
        return result;
    }

    /**
     *  Encapsulates some calls used in mainwnd.  Actually does a lot of
     *  work in those function calls.
     */

    int increment_beats_per_minute ()
    {
        int result = get_beats_per_minute() + 1;
        set_beats_per_minute(result);
        return result;
    }

    /**
     *  Encapsulates some calls used in mainwnd.
     */

    int decrement_screenset ()
    {
        int result = get_screenset() - 1;
        set_screenset(result);
        return result;
    }

    /**
     *  Encapsulates some calls used in mainwnd.
     */

    int increment_screenset ()
    {
        int result = get_screenset() + 1;
        set_screenset(result);
        return result;
    }

    /**
     *  True if a sequence is empty and should be highlighted.  This setting
     *  is currently a build-time option, but could be made a run-time option
     *  later.
     *
     * \param seq
     *      Provides a reference to the desired sequence.
     */

#if SEQ64_HIGHLIGHT_EMPTY_SEQS

    bool highlight (const sequence & seq) const
    {
        return seq.event_count() == 0;
    }

#else

    bool highlight (const sequence & /*seq*/) const
    {
        return false;
    }

#endif

    /**
     *  True if the sequence is an SMF 0 sequence.
     *
     * \param seq
     *      Provides a reference to the desired sequence.
     */

    bool is_smf_0 (const sequence & seq) const
    {
        return seq.is_smf_0();
    }

    void sequence_key (int seq);                        // encapsulation
    std::string sequence_label(const sequence & seq);
    void set_input_bus (int bus, bool input_active);    // used in options
    bool mainwnd_key_event (const keystroke & k);
    bool perfroll_key_event (const keystroke & k, int drop_sequence);
    bool playback_key_event (const keystroke & k, bool songmode = false);

private:

    /*
     * See launch() instead.
     */

    void launch_input_thread ();
    void launch_output_thread ();

    /**
     *  Initializes JACK support, if SEQ64_JACK_SUPPORT is defined.  Who calls
     *  this routine?  The main() routine of the application [via launch()],
     *  and the options module, when the Connect button is pressed.
     *
     * \return
     *      Returns the result of the init() call; true if JACK sync is now
     *      running.  If JACK support is not built into the application, then
     *      this function returns false, to indicate that JACK is (definitely)
     *      not running.
     */

    bool init_jack ()
    {
#ifdef SEQ64_JACK_SUPPORT
        return m_jack_asst.init();
#else
        return false;
#endif
    }

    /**
     *  Tears down the JACK infrastructure.  Called by launch() and in the
     *  options module, when the Disconnect button is pressed.
     *
     * \return
     *      Returns the result of the init() call; false if JACK sync is now
     *      no longer running.  If JACK support is not built into the
     *      application, then this function returns true, to indicate that
     *      JACK is (definitely) not running.
     */

    bool deinit_jack ()
    {
#ifdef SEQ64_JACK_SUPPORT
        return m_jack_asst.deinit();
#else
        return true;
#endif
    }

    bool seq_in_playing_screen (int seq);

    /**
     * \setter m_is_modified
     *
     * \param flag
     *      The value of the modified flag to be set.
     */

    void is_modified (bool flag)
    {
        m_is_modified = flag;
    }

    /**
     *  Checks the parameter against c_midi_controls.
     *
     * \param seq
     *      The value that should be in the c_midi_controls range.
     *
     * \return
     *      Returns true if the parameter is valid.  For this function, no
     *      error print-out is generated.
     */

    bool is_midi_control_valid (int seq) const
    {
        return seq < c_midi_controls;
    }

    /**
     *  Checks the screenset against m_max_sets.
     *
     * \param screenset
     *      The prospective screenset value.
     *
     * \return
     *      Returns true if the parameter is valid.  For this function, no
     *      error print-out is generated.
     */

    bool is_screenset_valid (int screenset) const
    {
        return screenset >= 0 && screenset < m_max_sets;
    }

    /**
     * \setter m_running
     *
     * \param running
     *      The value of the running flag to be set.
     */

    void set_running (bool running)
    {
        m_running = running;
    }

    /**
     * \setter m_playback_mode
     *
     * \param playbackmode
     *      The value of the playback mode flag to be set.
     */

    void set_playback_mode (bool playbackmode)
    {
        m_playback_mode = playbackmode;
    }

    /**
     *  A helper function to calculate the index into the mute-group array,
     *  based on the desired track.
     *
     * \param track
     *      The number of the desired track.
     */

    int mute_group_offset (int track)
    {
        return clamp_track(track) + m_mute_group_selected * c_seqs_in_set;
    }

    bool is_seq_valid (int seq) const;
    bool is_mseq_valid (int seq) const;
    bool install_sequence (sequence * seq, int seqnum);
    void inner_start (bool state);
    void inner_stop ();
    int clamp_track (int track) const;

    /**
     *  Pass-along function for keys().set_all_key_events.
     */

    void set_all_key_events ()
    {
        keys().set_all_key_events();
    }

    /**
     *  Pass-along function for keys().set_all_key_events.
     */

    void set_all_key_groups ()
    {
        keys().set_all_key_groups();
    }

    /**
     *  At construction time, this function sets up one keycode and one event
     *  slot.  It is called 32 times, corresponding to the pattern/sequence
     *  slots in the Patterns window.  It first removes the given key-code
     *  from the regular and reverse slot-maps.  Then it removes the
     *  sequence-slot from the regular and reverse slot-maps.  Finally, it
     *  adds the sequence-slot with a key value of key-code, and adds the
     *  key-code with a value of sequence-slot.
     *
     * \param keycode
     *      The keycode for which to set the sequence slot.
     *
     * \param sequence_slot
     *      The sequence slot to be set.
     */

    void set_key_event (unsigned int keycode, long sequence_slot)
    {
        keys().set_key_event(keycode, sequence_slot);
    }

    /**
     *  At construction time, this function sets up one keycode and one group
     *  slot.  It is called 32 times, corresponding the pattern/sequence slots
     *  in the Patterns window.  Compare it to the set_key_events() function.
     *
     * \param keycode
     *      The keycode for which to set the group slot.
     *
     * \param group_slot
     *      The group slot to be set.
     */

    void set_key_group (unsigned int keycode, long group_slot)
    {
        keys().set_key_group(keycode, group_slot);
    }

};

/**
 * Global functions defined in perform.cpp.
 */

extern void * output_thread_func (void * p);
extern void * input_thread_func (void * p);

}           // namespace seq64

#endif      // SEQ64_PERFORM_HPP

/*
 * perform.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

