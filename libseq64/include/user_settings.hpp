#ifndef SEQ64_USER_SETTINGS_HPP
#define SEQ64_USER_SETTINGS_HPP

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
 * \file          user_settings.hpp
 *
 *  This module declares/defines just some of the global (gasp!) variables
 *  in this application.
 *
 * \library       sequencer64 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-09-22
 * \updates       2016-08-14
 * \license       GNU GPLv2 or above
 *
 *  This module defines the following categories of "global" variables that
 *  are good to collect in one place, especially for settings stored in the
 *  "user" configuration file (<code> sequencer64.usr </code>):
 *
 *      -   The [user-midi-bus] settings, collected in the user_midi_bus
 *          class.
 *      -   The [use-instrument] settings, collected in the user_instrument
 *          class.
 *      -   The [user-interface-settings] settings, a small collection of
 *          variables that describe some facets of the "Patterns Panel" or
 *          "Sequences Window", which is visually presented by the
 *          Gtk::Window-derived class called mainwnd.  These variables define
 *          the limits and resolution of various MIDI-to-GUI and application
 *          control parameters.
 *      -   The [user-midi-settings] settings, a collection of variables that
 *          will replaced hard-wired global MIDI parameters with modifiable
 *          parameters better suited to a range of MIDI files.
 *
 *  The Patterns Panel contains an 8-by-4 grid of "pattern boxes" or
 *  "sequence boxes".  All of the patterns in this grid comprise what is
 *  called a "set" (in the musical sense) or a "screen set".
 *
 *  We want to be able to change these defaults.  We will let you know when we
 *  are finished, and what you can do with these variables.
 */

#include <string>
#include <vector>

#include "easy_macros.h"                /* with platform_macros.h, too  */
#include "scales.h"                     /* SEQ64_KEY_OF_C etc.          */
#include "midi_container.hpp"           /* SEQ64_IS_LEGAL_SEQUENCE etc. */
#include "user_instrument.hpp"
#include "user_midi_bus.hpp"

/**
 *  This special value of zoom sets the zoom according to a power of two
 *  related to the PPQN value of the song.
 */

#define SEQ64_USE_ZOOM_POWER_OF_2       0

namespace seq64
{

/**
 *  Holds the current values of sequence settings and settings that can
 *  modify the number of sequences and the configuration of the
 *  user-interface.  These settings will eventually be made part of the
 *  "user" settings file.
 */

class user_settings
{
    friend class userfile;      /* allow protected access to file parser */

    /**
     *  Provides a setting to control the overall style of grid-drawing for
     *  the pattern slots in mainwid.  These values can be specified in the
     *  [user-interface-settings] section of the "user" configuration file.
     *
     * \var grid_style_normal
     *      The grid background color is the normal background color for the
     *      current GTK theme.  The box is drawn with brackets on either side.
     *
     * \var grid_style_white
     *      The grid background color is white.  This style better fits
     *      displaying the white-on-black sequence numbers.  The box is drawn
     *      with brackets on either side.
     *
     * \var grid_style_black
     *      The grid background color is black.
     *
     * \var grid_style_max
     *      Marks the end of the list, and is an illegal value.
     */

    enum mainwid_grid_style_t
    {
        grid_style_normal,
        grid_style_white,
        grid_style_black,
        grid_style_max
    };

    /**
     *  [user-midi-bus-definitions]
     *
     *  Internal type for the container of user_midi_bus objects.
     *  Sorry about the "confusion" about "bus" versus "buss".
     *  See Google for arguments about it.
     */

    typedef std::vector<user_midi_bus> Busses;
    typedef std::vector<user_midi_bus>::iterator BussIterator;
    typedef std::vector<user_midi_bus>::const_iterator BussConstIterator;

    /**
     *  Provides data about the MIDI busses, readable from the "user"
     *  configuration file.  Since this object is a vector, its size is
     *  adjustable.
     */

    Busses m_midi_buses;

    /**
     *  [user-instrument-definitions]
     *
     *  Internal type for the container of user_instrument objects.
     */

    typedef std::vector<user_instrument> Instruments;
    typedef std::vector<user_instrument>::iterator InstrumentIterator;
    typedef std::vector<user_instrument>::const_iterator InstrumentConstIterator;

    /**
     *  Provides data about the MIDI instruments, readable from the "user"
     *  configuration file.  The size is adjustable, and grows as objects
     *  are added.
     */

    Instruments m_instruments;

    /**
     *  [user-interface-settings]
     *
     *  These are not labelled, but are present in the "user" configuration
     *  file in the following order:
     *
     *      -#  grid-style
     *      -#  grid-brackets
     *      -#  mainwnd-rows
     *      -#  mainwnd-cols
     *      -#  max-set
     *      -#  mainwid-border
     *      -#  control-height
     *      -#  zoom
     *      -#  global-seq-feature
     *      -#  use-new-font
     *      -#  allow-two-perfedits
     *      -#  perf-h-page-increment
     *      -#  perf-v-page-increment
     *      -#  progress-bar-colored (new)
     *      -#  progress-bar-thick (new)
     *      -#  window-redraw-rate-ms (new)
     */

    /**
     *  Specifies the current grid style.
     */

    mainwid_grid_style_t m_grid_style;

    /**
     *  Specify drawing brackets (like the old Seq24) or a solid box.
     *  0 = no brackets, 1 and above is the thickness of the brakcets.
     *  1 is the normal thickness of the brackets, 2 is a two-pixel thickness,
     *  and so on.
     */

    int m_grid_brackets;

    /**
     *  Number of rows in the Patterns Panel.  The current value is 4, and
     *  if changed, many other values depend on it.  Together with
     *  m_mainwnd_cols, this value fixes the patterns grid into a 4 x 8
     *  set of patterns known as a "screen set".  We would like to be able to
     *  change this value from 4 to 8, and maybe allow the values of 5, 6, and 7
     *  as well.  But if we could just get 8 working, then well would
     *  Sequencer64 deserve the 64 in its name.
     */

    int m_mainwnd_rows;

    /**
     *  Number of columns in the Patterns Panel.  The current value is 4,
     *  and probably won't change, since other values depend on it.
     *  Together with m_mainwnd_rows, this value fixes the patterns grid
     *  into a 4 x 8 set of patterns known as a "screen set".
     */

    int m_mainwnd_cols;

    /**
     *  Maximum number of screen sets that can be supported.  Basically,
     *  that the number of times the Patterns Panel can be filled.  32
     *  sets can be created.  Although this value is part of the "user"
     *  configuration file, it is likely that it will never change.  Rather,
     *  the number of sequences per set would change.  We'll see.
     */

    int m_max_sets;

    /**
     *  These control sizes.  We'll try changing them and see what
     *  happens.  Increasing these value spreads out the pattern grids a
     *  little bit and makes the Patterns panel slightly bigger.  Seems
     *  like it would be useful to make these values user-configurable.
     */

    int m_mainwid_border;   /* c_mainwid_border = 0;  try 2 or 3 instead    */
    int m_mainwid_spacing;  /* c_mainwid_spacing = 2; try 4 or 6 instead    */

    /**
     *  This constants seems to be created for a future purpose, perhaps
     *  to reserve space for a new bar on the mainwid pane.  But it is
     *  used only in this header file, to define m_mainwid_y, but doesn't
     *  add anything to that value.
     */

    int m_control_height;   /* c_control_height = 0;                        */

    /**
     *  Provides the initial zoom value, in units of ticks per pixel.  The
     *  original default value was 32 ticks per pixel, but larger PPQN values
     *  need higher values, and we will have to adapt the default zoom to the
     *  PPQN value.  Also, the zoom can never be zero, as it can appear as the
     *  divisor in scaling equations.
     */

    int m_current_zoom;

    /**
     *  If true, this value provide a bit of backward-compatibility with the
     *  global key/scale/background-sequence persistence feature.  In this
     *  feature, applying one of these three changes to a sequence causes them
     *  to also be applied to sequences that are subsequently opened for
     *  editing.  However, we improve on this feature by allowing the changes
     *  to be saved in the global, proprietary part of the saved MIDI file.
     *
     *  If false, the user can still save the key/scale/background-sequence
     *  values with each individual sequence, so they can be different.
     *
     *  This value will be true by default, unless changed in the "user"
     *  configuration file.
     */

    bool m_global_seq_feature_save;

    /**
     *  Replaces seqedit::m_initial_scale as the repository for the scale to
     *  apply when a sequence is loaded into the sequence editor.  Its default
     *  value is c_scale_off.  Although this value is now stored in the
     *  user_settings class, it always comes from the currently loaded MIDI
     *  file, if present.  If m_global_seq_feature_save is true, this variable
     *  is stored in the "proprietary" track at the end of the file, under the
     *  control tag c_musicscale, and will be applied to any sequence that is
     *  edited.  If m_global_seq_feature_save is false, this variable is
     *  stored, if used, in the meta-data for the sequence to which it applies,
     *  and, again, is tagged with the control tag c_musicscale.
     */

    int m_seqedit_scale;

    /**
     *  Replaces seqedit::m_initial_key as the repository for the key to
     *  apply when a sequence is loaded into the sequence editor.  Its default
     *  value is SEQ64_KEY_OF_C.  Although this value is now stored in the
     *  user_settings class, it always comes from the currently loaded MIDI
     *  file, if present.  If m_global_seq_feature_save is true, this variable
     *  is stored in the "proprietary" track at the end of the file, under the
     *  control tag c_musickey, and will be applied to any sequence that is
     *  edited.  If m_global_seq_feature_save is false, this variable is
     *  stored, if used, in the meta-data for the sequence to which it applies,
     *  and, again, is tagged with the control tag c_musickey.
     */

    int m_seqedit_key;

    /**
     *  Replaces seqedit::m_initial_sequence as the repository for the
     *  background sequence to apply when a sequence is loaded into the
     *  sequence editor.  Its default value is SEQ64_SEQUENCE_LIMIT.  Although
     *  this value is now stored in the user_settings class, it always comes
     *  from the currently loaded MIDI file, if present.  If
     *  m_global_seq_feature_save is true, this variable is stored, if it has
     *  a valid (but not "legal") value, in the "proprietary" track at the end
     *  of the file, under the control tag c_backsequence, and will be applied
     *  to any sequence that is edited.  If m_global_seq_feature_save is
     *  false, this variable is stored, if used, in the meta-data for the
     *  sequence to which it applies, and, again, is tagged with the control
     *  tag c_backsequence.
     */

    int m_seqedit_bgsequence;

    /**
     *  Sets the usage of the font.  By default, in normal mode, the new font
     *  is used.  In legacy mode, the old font is used.
     */

    bool m_use_new_font;

    /**
     *  Enables the usage of two perfedit windows, for added convenience in
     *  editing multi-set songs.  Defaults to true.
     */

    bool m_allow_two_perfedits;

    /**
     *  Allows a changed to the page size for the horizontal scroll bar.
     *  The value used to be hardwired to 1 (in four-measure units), now it
     *  defaults to 4 (16 measures at a time).  The value of 1 is already
     *  covered by the scrollbar arrows.
     */

    int m_h_perf_page_increment;

    /**
     *  Allows a changed to the page size for the vertical scroll bar.
     *  The value used to be hardwired to 1 (in single-track units), now it
     *  defaults to 8.  The value of 1 is already covered by the scrollbar
     *  arrows.
     */

    int m_v_perf_page_increment;

    /**
     *  If set, makes progress bars have the "progress_color()", instead of
     *  black.  This value is hardwired in the gui_palette_gtk2 module, to
     *  red.  Really, that is the only color that stands out as well as black.
     */

    bool m_progress_bar_colored;

    /**
     *  If set, makes progress bars thicker than 1 pixel... 2 pixels.
     *  It isn't useful to support anything thicker.
     */

    bool m_progress_bar_thick;

    /**
     *  If set, use an alternate, neo-inverse color palette.  Not all colors
     *  are reversed, though.
     */

    bool m_inverse_colors;

    /**
     *  Provides the global setting for redraw rate of windows.  Not all
     *  windows use this yet.  The default is 40 ms (c_redraw_ms, which is 20
     *  ms in Windows builds)), but some windows originally used 25 ms, so
     *  beware of side-effects.
     */

    int m_window_redraw_rate_ms;

    /**
     *  Constants for the mainwid class.  The m_text_x and m_text_y
     *  constants help define the "seqarea" size.  It looks like these two
     *  values are the character width (x) and height (y) in pixels.
     *  Thus, these values would be dependent on the font chosen.  But
     *  that, currently, is hard-wired.  See the m_font_6_12[] array for
     *  the default font specification.
     *
     *  However, please not that font files are not used.  Instead, the
     *  fonts are provided by two pixmaps in the <code> src/pixmap </code>
     *  directory: <code> font_b.xpm </code> (black lettering on a white
     *  background) and <code> font_w.xpm </code> (white lettering on a black
     *  background).
     *
     *  We have added black-on-yellow and yellow-on-black versions of the
     *  fonts, to support the highlighting of pattern boxes if they are empty
     *  of actual MIDI events.
     *
     *  We have also added a set of four new font files that are roughly the
     *  same size, and are treated as the same size, but look smooth and less
     *  like a DOS-era font.
     *
     *  The font module does not use these values directly, but does define
     *  some similar variables that differ slightly between the two styles of
     *  font.  There are a lot of tricks and hard-wired places to fix before
     *  further work can be done with fonts in Sequencer64.
     */

    int m_text_x;       /* c_text_x =  6, does not include inner padding    */
    int m_text_y;       /* c_text_y = 12, does include inner padding        */

    /**
     *  Constants for the mainwid class.  The m_seqchars_x and
     *  m_seqchars_y constants help define the "seqarea" size.  These look
     *  like the number of characters per line and the number of lines of
     *  characters, in a pattern/sequence box.
     */

    int m_seqchars_x;   /* c_seqchars_x = 15    */
    int m_seqchars_y;   /* c_seqchars_y =  5    */

    /*
     *                  [user-midi-settings]
     */

    /**
     *  Provides the universal PPQN setting for the duration of this setting.
     *  This variable replaces the global ppqn.  The default value of this
     *  setting is 192 parts-per-quarter-note (PPQN).  There is still a lot of
     *  work to get a different PPQN to work properly in speed of playback,
     *  scaling of the user interface, and other issues.  Note that this value
     *  can be changed by the still-experimental --ppqn option.  There is one
     *  remaining trace of the global, though:  DEFAULT_PPQN.
     */

    int m_midi_ppqn;                     /* PPQN, parts per QN       */

    /**
     *  Provides the universal and unambiguous MIDI value for beats per
     *  measure, also called "beats per bar" (BPB).  This variable will
     *  replace the global beats per measure.  The default value of this
     *  variable is SEQ64_DEFAULT_BEATS_PER_MEASURE (4).  For external access,
     *  we will call this value "beats per bar", abbreviate it "BPB", and use
     *  "bpb" in any accessor function names.  Now, although it applies to the
     *  whole session, we should be able to continue seq24's tradition of
     *  allowing each sequence to have its own time signature.  Also, there
     *  are a number of places where the number 4 appears and looks like it
     *  might be a hardwired BPB value, either for MIDI purposes or for
     *  drawing the piano-roll grids.  So we might need a couple different
     *  versions of this variable.
     */

    int m_midi_beats_per_measure;        /* BPB, or beats per bar       */

    /**
     *  Provides the universal and unambiguous MIDI value for beats per minute
     *  (BPM).  This variable will replace the global beats per minute.  The
     *  default value of this variable is DEFAULT_BPM (120).  This variable
     *  should apply to the whole session; there's probably no way to support
     *  a diffent tempo for each sequence.  But we shall see.  For external
     *  access, we will call this value "beats per minute", abbreviate it
     *  "BPM", and use "bpm" in any accessor function names.
     */

    int m_midi_beats_per_minute;         /* BPM, or beats per minute    */

    /**
     *  Provides the universal MIDI value for beats width (BW).  This variable
     *  will replace the global beat_width.  The default value of this
     *  variable is DEFAULT_BEAT_WIDTH (4).  Now, although it applies to the
     *  whole session, we should be able to continue seq24's tradition of
     *  allowing each sequence to have its own time signature.  Also, there
     *  are a number of places where the number 4 appears and looks like it
     *  might be a hardwired BW value, either for MIDI purposes or for drawing
     *  the user-interface.  So we might need a couple different versions of
     *  this variable.  For external access, we will call this value "beat
     *  width", abbreviate it "BW", and use "bw" in any accessor function
     *  names.
     */

    int m_midi_beat_width;              /* BW, or beat width            */

    /**
     *  Provides a universal override of the buss number for all sequences, for
     *  the purpose of convenience of of testing.  This variable replaces the
     *  global buss-override variable, and is set via the command-line option
     *  --bus.
     */

    char m_midi_buss_override;          /* --bus n option               */

    /*
     *  Values calculated from other member values in the normalize() function.
     */

    /*
     *  The maximum number of patterns supported is given by the number of
     *  patterns supported in the panel (32) times the maximum number of
     *  sets (32), or 1024 patterns.  It is basically the same value as
     *  m_max_sequence by default.  It is a derived value, and not stored in
     *  the "user" file.  We might make it equal to the maximum number of
     *  sequences the currently-loaded MIDI file.
     *
     *      m_total_seqs = m_seqs_in_set * m_max_sets;
     */

    int m_total_seqs;                   /* not included in .usr file    */

    /**
     *  Number of patterns/sequences in the Patterns Panel, also known as
     *  a "set" or "screen set".  This value is 4 x 8 = 32 by default.
     *
     * \warning
     *      Currently implicit/explicit in a number of the "rc" file and
     *      rc_settings.  Would probably want the left 32 or the first 32
     *      items in the main window only to be subject to keystroke control.
     *      This value is calculated by the normalize() function, and is <i>
     *      not </i> part of the "user" configuration file.
     */

    int m_seqs_in_set;                  /* not include in .usr file     */

    /**
     *  Number of group-mute tracks that can be supported, which is
     *  m_seqs_in_set squared, or 1024.  This value is <i> not </i>
     *  part of the "user" configuration file; it is calculated by the
     *  normalize() function.
     */

    int m_gmute_tracks;                 /* not included in .usr file    */

    /**
     *  The maximum number of patterns supported is given by the number of
     *  patterns supported in the panel (32) times the maximum number of
     *  sets (32), or 1024 patterns.  It is a derived value, and not stored in
     *  the "user" file.
     *
     *      m_max_sequence = m_seqs_in_set * m_max_sets;
     */

    int m_max_sequence;

    /**
     *  The m_seqarea_x and m_seqarea_y constants are derived from the
     *  width and heights of the default character set, and the number of
     *  characters in width, and the number of lines, in a
     *  pattern/sequence box.
     *
     *  Compare these two constants to m_seqarea_seq_x(y), which was in
     *  mainwid.h, but is now in this file.
     */

    int m_seqarea_x;
    int m_seqarea_y;

    /**
     * Area of what?  Doesn't look at all like it is based on the size of
     * characters.  These are used only in the mainwid module.
     */

    int m_seqarea_seq_x;
    int m_seqarea_seq_y;

    /**
     * The width of the main pattern/sequence grid, in pixels.  Affected
     * by the m_mainwid_border and m_mainwid_spacing values.
     *
     *      c_mainwid_x =
     *      (
     *          (c_seqarea_x + c_mainwid_spacing) * c_mainwnd_cols -
     *              c_mainwid_spacing + c_mainwid_border * 2
     *      );
     */

    int m_mainwid_x;

    /*
     * The height of the main pattern/sequence grid, in pixels.  Affected by
     * the m_mainwid_border and m_control_height values.
     *
     *      c_mainwid_y =
     *      (
     *          (c_seqarea_y + c_mainwid_spacing) * c_mainwnd_rows +
     *               c_control_height + c_mainwid_border * 2
     *      );
     */

    int m_mainwid_y;

    /**
     *  Provides a temporary variable that can be set from the command line to
     *  cause the "user" state to be saved into the "user" configuration file.
     *
     *  Normally, this state is not saved.  It is not saved because there is
     *  currently no user-interface for editing it, and because it can pick up
     *  some command-line options, and it is not right to have them written
     *  to the "user" configuration file.
     *
     *  (The "rc" configuration file is a different case, having historically
     *  always been saved, and having a number of command-line options, such
     *  as JACK settings that should generally be permanent on a given
     *  system.)
     *
     *  Anyway, this flag can be set by the --user-save option.  This setting
     *  is never saved.  But note that, if no "user" configuration file is
     *  found, it is then saved anyway.
     */

    bool m_save_user_config;

    /*
     *  All constant (unchanging) values go here.  They are not saved or read.
     */

    /**
     *  Provides the minimum zoom value, currently a constant.  It's value is
     *  1.
     */

    const int mc_min_zoom;

    /**
     *  Provides the maximum zoom value, currently a constant.  It's value was
     *  32, but is now 512, to allow for better presentation of high PPQN
     *  valued sequences.
     */

    const int mc_max_zoom;

    /**
     *  Permanent storage for the baseline, default PPQN used by Seq24.
     *  This value is necessary in order to keep user-interface elements
     *  stable when different PPQNs are used.  It is set to DEFAULT_PPQN.
     */

    const int mc_baseline_ppqn;

public:

    user_settings ();
    user_settings (const user_settings & rhs);
    user_settings & operator = (const user_settings & rhs);

    void set_defaults ();
    void normalize ();

    bool add_bus (const std::string & alias);
    bool add_instrument (const std::string & instname);

    /**
     * \getter
     *      Unlike the non-const version this function is public.
     *      Cannot append the const specifier.
     */

    const user_midi_bus & bus (int index) // const
    {
        return private_bus(index);
    }

    /**
     * \getter
     *      Unlike the non-const version this function is public.
     *      Cannot append the const specifier.
     */

    const user_instrument & instrument (int index) // const
    {
        return private_instrument(index);
    }

    /**
     * \getter m_midi_buses.size()
     */

    int bus_count () const
    {
        return int(m_midi_buses.size());
    }

    void set_bus_instrument (int index, int channel, int instrum);

    /**
     * \getter m_midi_buses[buss].instrument[channel]
     */

     int bus_instrument (int buss, int channel)
     {
          return bus(buss).instrument(channel);
     }

    /**
     * \getter m_midi_buses[buss].name
     */

    const std::string & bus_name (int buss)
    {
        return bus(buss).name();
    }

    /**
     * \getter m_instruments.size()
     */

    int instrument_count () const
    {
        return int(m_instruments.size());
    }

    void set_instrument_controllers
    (
        int index, int cc, const std::string & ccname, bool isactive
    );

    /**
     * \getter m_instruments[instrument].instrument (name of instrument)
     */

    const std::string & instrument_name (int instrum)
    {
        return instrument(instrum).name();
    }

    /**
     *  Gets the correct instrument number from the buss and channel, and then
     *  looks up the name of the instrument.
     */

    const std::string & instrument_name (int buss, int channel)
    {
        int instrum = bus_instrument(buss, channel);
        return instrument(instrum).name();
    }

    /**
     * \getter m_instruments[instrument].controllers_active[controller]
     */

    bool instrument_controller_active (int instrum, int cc)
    {
        return instrument(instrum).controller_active(cc);
    }

    /**
     *      A convenience function so that the caller doesn't have to get the
     *      instrument number from the bus_instrument() member function.
     *      It also has a shorter name.
     */

    bool controller_active (int buss, int channel, int cc)
    {
        int instrum = bus_instrument(buss, channel);
        return instrument(instrum).controller_active(cc);
    }

    /**
     * \getter m_instruments[instrument].controllers_active[controller]
     */

    const std::string & instrument_controller_name (int instrum, int cc)
    {
        return instrument(instrum).controller_name(cc);
    }

    /**
     * \getter m_instruments[instrument].controllers_active[controller]
     *      A convenience function so that the caller doesn't have to get the
     *      instrument number from the bus_instrument() member function.
     *      It also has a shorter name.
     */

    const std::string & controller_name (int buss, int channel, int cc)
    {
        int instrum = bus_instrument(buss, channel);
        return instrument(instrum).controller_name(cc);
    }

public:

    /**
     * \getter m_grid_style
     *      Checks for normal style.
     */

    int grid_style () const
    {
        return int(m_grid_style);
    }

    /**
     * \getter m_grid_style
     *      Checks for normal style.
     */

    bool grid_is_normal () const
    {
        return m_grid_style == grid_style_normal;
    }

    /**
     * \getter m_grid_style
     *      Checks for the white style.
     */

    bool grid_is_white () const
    {
        return m_grid_style == grid_style_white;
    }

    /**
     * \getter m_grid_style
     *      Checks for the black style.
     */

    bool grid_is_black () const
    {
        return m_grid_style == grid_style_black;
    }

    /**
     * \getter m_grid_brackets
     */

    int grid_brackets () const
    {
        return m_grid_brackets;
    }

    /**
     * \getter m_mainwnd_rows
     */

    int mainwnd_rows () const
    {
        return m_mainwnd_rows;
    }

    /**
     * \getter m_mainwnd_cols
     */

    int mainwnd_cols () const
    {
        return m_mainwnd_cols;
    }

    /**
     * \getter m_seqs_in_set, dependent member
     */

    int seqs_in_set () const
    {
        return m_seqs_in_set;
    }

    /**
     * \getter m_gmute_tracks, dependent member
     */

    int gmute_tracks () const
    {
        return m_gmute_tracks;
    }

    /**
     * \getter m_max_sets
     */

    int max_sets () const
    {
        return m_max_sets;
    }

    /**
     * \getter m_max_sequence, dependent member
     */

    int max_sequence () const
    {
        return m_max_sequence;
    }

    /**
     * \getter m_text_x, not user modifiable, not saved
     */

    int text_x () const
    {
        return m_text_x;
    }

    /**
     * \getter m_text_y, not user modifiable, not saved
     */

    int text_y () const
    {
        return m_text_y;
    }

    /**
     * \getter m_seqchars_x, not user modifiable, not saved
     */

    int seqchars_x () const
    {
        return m_seqchars_x;
    }

    /**
     * \getter m_seqchars_y, not user modifiable, not saved
     */

    int seqchars_y () const
    {
        return m_seqchars_y;
    }

    /**
     * \getter m_seqarea_x, not user modifiable, not saved
     */

    int seqarea_x () const
    {
        return m_seqarea_x;
    }

    /**
     * \getter m_seqarea_y, not user modifiable, not saved
     */

    int seqarea_y () const
    {
        return m_seqarea_y;
    }

    /**
     * \getter m_seqarea_seq_x, not user modifiable, not saved
     */

    int seqarea_seq_x () const
    {
        return m_seqarea_seq_x;
    }

    /**
     * \getter m_seqarea_seq_y, not user modifiable, not saved
     */

    int seqarea_seq_y () const
    {
        return m_seqarea_seq_y;
    }

    /**
     * \getter m_mainwid_border
     */

    int mainwid_border () const
    {
        return m_mainwid_border;
    }

    /**
     * \getter m_mainwid_spacing
     */

    int mainwid_spacing () const
    {
        return m_mainwid_spacing;
    }

    /**
     * \getter m_mainwid_x, dependent member
     */

    int mainwid_x () const
    {
        return m_mainwid_x;
    }

    /**
     * \getter m_mainwid_y, dependent member
     */

    int mainwid_y () const
    {
        return m_mainwid_y;
    }

    /**
     * \getter m_control_height
     */

    int control_height () const
    {
        return m_control_height;
    }

    /**
     * \getter m_current_zoom
     */

    int zoom () const
    {
        return m_current_zoom;
    }

    void zoom (int value);      /* seqedit can change this one */

    /**
     * \getter m_global_seq_feature_save
     */

    bool global_seq_feature () const
    {
        return m_global_seq_feature_save;
    }

    /**
     * \setter m_global_seq_feature_save
     */

    void global_seq_feature (bool flag)
    {
        m_global_seq_feature_save = flag;
    }

    /**
     * \getter m_seqedit_scale
     */

    int seqedit_scale () const
    {
        return m_seqedit_scale;
    }

    /**
     * \setter m_seqedit_scale
     */

    void seqedit_scale (int scale)
    {
        if (scale >= int(c_scale_off) && scale < int(c_scale_size))
            m_seqedit_scale = scale;
    }

    /**
     * \getter m_seqedit_key
     */

    int seqedit_key () const
    {
        return m_seqedit_key;
    }

    /**
     * \setter m_seqedit_key
     */

    void seqedit_key (int key)
    {
        if (key >= SEQ64_KEY_OF_C && key < SEQ64_OCTAVE_SIZE)
            m_seqedit_key = key;
    }

    /**
     * \getter m_seqedit_bgsequence
     */

    int seqedit_bgsequence () const
    {
        return m_seqedit_bgsequence;
    }

    /**
     * \setter m_seqedit_bgsequence
     *      Note that SEQ64_IS_LEGAL_SEQUENCE() allows the
     *      SEQ64_SEQUENCE_LIMIT (0x800 = 2048) value, to turn off the use of
     *      a background sequence.
     */

    void seqedit_bgsequence (int seqnum)
    {
        if (SEQ64_IS_LEGAL_SEQUENCE(seqnum))
            m_seqedit_bgsequence = seqnum;
    }

    /**
     * \getter m_use_new_font
     */

    bool use_new_font () const
    {
        return m_use_new_font;
    }

    /**
     * \getter m_allow_two_perfedits
     */

    bool allow_two_perfedits () const
    {
        return m_allow_two_perfedits;
    }

    /**
     * \getter m_h_perf_page_increment
     */

    int perf_h_page_increment () const
    {
        return m_h_perf_page_increment;
    }

    /**
     * \getter m_v_perf_page_increment
     */

    int perf_v_page_increment () const
    {
        return m_v_perf_page_increment;
    }

    /**
     * \getter m_progress_bar_colored
     */

    bool progress_bar_colored () const
    {
        return m_progress_bar_colored;
    }

    /**
     * \getter m_progress_bar_thick
     */

    bool progress_bar_thick () const
    {
        return m_progress_bar_thick;
    }

    /**
     * \accessor m_inverse_colors
         */

    bool inverse_colors () const
    {
        return m_inverse_colors;
    }

    /**
     * \getter m_window_redraw_rate_ms
     */

    int window_redraw_rate () const
    {
        return m_window_redraw_rate_ms;
    }

    /**
     * \getter m_save_user_config
     */

    bool save_user_config () const
    {
        return m_save_user_config;
    }

    /**
     * \setter m_save_user_config
     */

    void save_user_config (bool flag)
    {
        m_save_user_config = flag;
    }

protected:

    /**
     * \getter m_grid_brackets
     */

    void grid_brackets (int thickness)
    {
        if (thickness >= (-30) && thickness <= 30)
            m_grid_brackets = thickness;
    }

    void grid_style (int gridstyle);
    void mainwnd_rows (int value);
    void mainwnd_cols (int value);
    void max_sets (int value);
    void text_x (int value);
    void text_y (int value);
    void seqchars_x (int value);
    void seqchars_y (int value);
    void seqarea_x (int value);
    void seqarea_y (int value);
    void seqarea_seq_x (int value);
    void seqarea_seq_y (int value);
    void mainwid_border (int value);
    void mainwid_spacing (int value);
    void control_height (int value);

    /*
     *  These values are calculated from other values in the normalize()
     *  function:
     *
     *  void seqs_in_set (int value);
     *  void gmute_tracks (int value);
     *  void max_sequence (int value);
     *  void mainwid_x (int value);
     *  void mainwid_y (int value);
     */

    void dump_summary();

public:

    /**
     * \getter m_midi_ppqn
     */

    int midi_ppqn () const
    {
        return m_midi_ppqn;
    }

    /**
     * \getter m_midi_beats_per_measure
     */

    int midi_beats_per_bar () const
    {
        return m_midi_beats_per_measure;
    }

    /**
     * \getter m_midi_beats_per_minute
     */

    int midi_beats_per_minute () const
    {
        return m_midi_beats_per_minute;
    }

    /**
     * \getter m_midi_beat_width
     */

    int midi_beat_width () const
    {
        return m_midi_beat_width;
    }

    /**
     * \getter m_midi_buss_override
     */

    char midi_buss_override () const
    {
        return m_midi_buss_override;
    }

    /**
     * \getter mc_min_zoom
     */

    int min_zoom () const
    {
        return mc_min_zoom;
    }

    /**
     * \getter mc_max_zoom
     */

    int max_zoom () const
    {
        return mc_max_zoom;
    }

    /**
     * \getter mc_baseline_ppqn
     */

    int baseline_ppqn () const
    {
        return mc_baseline_ppqn;
    }

public:         // used in main application module and the userfile class

    /**
     * \setter m_use_new_font
     */

    void use_new_font (bool flag)
    {
        m_use_new_font = flag;
    }

    /**
     *  Sets the value of allowing two perfedits to be created and shown to
     *  the user.
     */

    void allow_two_perfedits (bool flag)
    {
        m_allow_two_perfedits = flag;
    }

    void perf_h_page_increment (int inc);
    void perf_v_page_increment (int inc);

    /**
     * \setter m_progress_bar_colored
     */

    void progress_bar_colored (bool flag)
    {
        m_progress_bar_colored = flag;
    }

    /**
     * \setter m_progress_bar_thick
     */

    void progress_bar_thick (bool flag)
    {
        m_progress_bar_thick = flag;
    }

    /**
     * \setter m_inverse_colors
     */

    void inverse_colors (bool flag)
    {
        m_inverse_colors = flag;
    }

    /**
     * \setter m_window_redraw_rate_ms
     */

    void window_redraw_rate (int ms)
    {
        m_window_redraw_rate_ms = ms;
    }

    void midi_ppqn (int ppqn);
    void midi_buss_override (char buss);

protected:

    void midi_beats_per_bar (int beatsperbar);
    void midi_beats_per_minute (int beatsperminute);
    void midi_beat_width (int beatwidth);

private:

    user_midi_bus & private_bus (int buss);
    user_instrument & private_instrument (int instrum);

};

}           // namespace seq64

#endif      // SEQ64_USER_SETTINGS_HPP

/*
 * user_settings.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

