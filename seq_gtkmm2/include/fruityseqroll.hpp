#ifndef SEQ64_FRUITYSEQROLL_HPP
#define SEQ64_FRUITYSEQROLL_HPP

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
 * \file          fruityseqroll.hpp
 *
 *  This module declares/defines the base class for seqroll interactions
 *  using the "fruity" mouse paradigm.
 *
 * \library       sequencer64 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2018-01-20
 * \license       GNU GPLv2 or above
 *
 */

#include "seqroll.hpp"                  /* seq64::seqroll class         */

namespace Gtk
{
    class Adjustment;
}

/*
 *  Do not document a namespace; it breaks Doxygen.
 */

namespace seq64
{
    class perform;
    class seqkeys;
    class sequence;

/**
 *  Implements the fruity mouse interaction paradigm for the seqroll.
 */

class FruitySeqRollInput : public seqroll
{

private:

    /**
     *  Indicates adding status peculiar to the fruity mode.  Currently
     *  always true.
     */

    bool m_can_add;

    /**
     *  Set to tru if we hold the right mouse button down (in "fruity" mode)
     *  and start to drag the mouse around, erasing notes.
     */

    bool m_erase_painting;

    /**
     *  Holds the original position of the mouse when ctrl-left-click-drag is
     *  done, and is used to make sure that the action doesn't occur until a
     *  movement of at least 6 pixels has occurred, to avoid unintended
     *  actions caused by minimal jitter in the user's hands.
     */

    int m_drag_paste_start_pos[2];

public:

    /**
     * Default constructor.
     */

    FruitySeqRollInput
    (
        perform & perf,
        sequence & seq,
        int zoom,
        int snap,
        seqkeys & seqkeys_wid,
        int pos,
        Gtk::Adjustment & hadjust,
        Gtk::Adjustment & vadjust
    );

    virtual void update_mouse_pointer (bool isadding);

public:         // callbacks

    virtual bool on_button_press_event (GdkEventButton * ev);
    virtual bool on_button_release_event (GdkEventButton * ev);
    virtual bool on_motion_notify_event (GdkEventMotion * ev);

};          // FruitySeqRollInput

}           // namespace seq64

#endif      // SEQ64_FRUITYSEQROLL_HPP

/*
 * fruityseqroll.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

