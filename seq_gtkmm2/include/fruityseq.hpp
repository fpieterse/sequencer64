#ifndef SEQ64_FRUITYSEQ_HPP
#define SEQ64_FRUITYSEQ_HPP

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
 * \file          fruityseq.hpp
 *
 *  This module declares/defines the mouse interactions for the "fruity"
 *  mode.
 *
 * \library       sequencer64 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-08-02
 * \updates       2017-09-17
 * \license       GNU GPLv2 or above
 *
 */

#include "seqevent.hpp"                 /* seq64::seqevent class        */
/*
 * Do not document the namespace; it breaks Doxygen.
 */

namespace seq64
{

/**
 *  This class implements the interaction methods for the "fruity"
 *  mode of operation in the event panel of the seqroll.
 */

class FruitySeqEventInput : public seqevent
{

private:

    /**
     *  Indicates that the left mouse button was click to start a selection.
     */

    bool m_justselected_one;

    /**
     *  Set to true when the mouse button is pressed and we're starting to
     *  drag some notes to move them and paste them to a different location.
     */

    bool m_is_drag_pasting_start;

    /**
     *  Set to true when the left mouse button is pressed for dragging and
     *  pasting, set to false when the mouse button is released to drop the
     *  pasted items.
     */

    bool m_is_drag_pasting;

public:

    FruitySeqEventInput
    (
        perform & p,
        sequence & seq,
        int zoom,
        int snap,
        seqdata & seqdata_wid,
        Gtk::Adjustment & hadjust
    );

private:

    virtual void update_mouse_pointer ();

private:

    virtual bool on_button_press_event (GdkEventButton * ev);
    virtual bool on_button_release_event (GdkEventButton * ev);
    virtual bool on_motion_notify_event (GdkEventMotion * ev);

};          // class FruitySeqEventInput

}           // namespace seq64

#endif      // SEQ64_FRUITYSEQ_HPP

/*
 * fruityseq.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
