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
 * \file          qseqkeys.cpp
 *
 *  This module declares/defines the base class for the left-side piano of
 *  the pattern/sequence panel.
 *
 * \library       sequencer64 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2018-01-01
 * \updates       2018-07-22
 * \license       GNU GPLv2 or above
 *
 *      We've added the feature of a right-click toggling between showing the
 *      main octave values (e.g. "C1" or "C#1") versus the numerical MIDI
 *      values of the keys.
 */

#include "qseqkeys.hpp"
#include "sequence.hpp"

/*
 *  Do not document a namespace; it breaks Doxygen.
 */

namespace seq64
{
    class perform;

qseqkeys::qseqkeys
(
    sequence & seq, QWidget * parent, int keyheight, int keyareaheight
) :
    QWidget                 (parent),
    m_seq                   (seq),
    m_timer                 (nullptr),
    m_font                  (),
    m_show_octave_letters   (true),
    m_key                   (0),
    m_key_y                 (keyheight),
    m_key_area_y            (keyareaheight),
    m_Previewing            (false),
    m_PreviewKey            (-1)
{
    // setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMouseTracking(true);
}

/**
 *
 */

void
qseqkeys::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QPen pen(Qt::black);
    QBrush brush (Qt::SolidPattern);
    pen.setStyle(Qt::SolidLine);
    brush.setColor(Qt::lightGray);
    m_font.setPointSize(6);
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.setFont(m_font);

    // draw keyboard border

    painter.drawRect(0, 0, c_keyarea_x, m_key_area_y);
    for (int i = 0; i < c_num_keys; ++i)
    {
        pen.setColor(Qt::black);                // draw keys
        pen.setStyle(Qt::SolidLine);
        brush.setColor(Qt::white);
        brush.setStyle(Qt::SolidPattern);
        painter.setPen(pen);
        painter.setBrush(brush);
        painter.drawRect(c_keyoffset_x+1, m_key_y*i + 1, c_key_x-2, m_key_y-1);

        int keyvalue = c_num_keys - i - 1;
        int key = keyvalue % SEQ64_OCTAVE_SIZE;
        if (is_black_key(key))
        {
            pen.setStyle(Qt::SolidLine); // draw black keys
            pen.setColor(Qt::black);
            brush.setColor(Qt::black);
            painter.setPen(pen);
            painter.setBrush(brush);
            painter.drawRect
            (
                c_keyoffset_x+1, m_key_y*i + 3, c_key_x-4, m_key_y-5
            );
        }

        if (keyvalue == m_PreviewKey) // highlight note preview
        {
            brush.setColor(Qt::red);
            pen.setStyle(Qt::NoPen);
            painter.setPen(pen);
            painter.setBrush(brush);
            painter.drawRect
            (
                c_keyoffset_x+3, m_key_y*i + 3, c_key_x-5, m_key_y-4
            );
        }

        char note[20];
        if (m_show_octave_letters)
        {
            if (key == m_key)
            {
                int octave = (keyvalue / 12) - 1;      /* notes */
                if (octave < 0)
                    octave *= -1;

                /*
                 * TODO:  use the scales header file to get this
                 */

                snprintf
                (
                    note, sizeof note, "%2s%1d", c_key_text[key], octave
                );
                pen.setColor(Qt::black);            // draw "Cx" octave labels
                pen.setStyle(Qt::SolidLine);
                painter.setPen(pen);
                painter.drawText(2, m_key_y * i + 11, note);
            }
        }
        else
        {
            if ((keyvalue % 2) == 0)
            {
                snprintf(note, sizeof note, "%3d", keyvalue);
                pen.setColor(Qt::black);
                pen.setStyle(Qt::SolidLine);
                painter.setPen(pen);
                painter.drawText(1, m_key_y * i + 9, note);
            }
        }
    }
}

/**
 *
 */

void
qseqkeys::mousePressEvent (QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        int note;
        int y = event->y();
        m_Previewing = true;
        convert_y(y, note);
        m_PreviewKey = note;
        m_seq.play_note_on(note);
    }
    else if (event->button() == Qt::RightButton)
    {
        m_show_octave_letters = ! m_show_octave_letters;
    }
    update();
}

/**
 *
 */

void
qseqkeys::mouseReleaseEvent (QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton && m_Previewing)
    {
        m_seq.play_note_off(m_PreviewKey);
        m_Previewing = false;
        m_PreviewKey = -1;
    }
    update();
}

/**
 *
 */

void
qseqkeys::mouseMoveEvent (QMouseEvent * event)
{
    int note;
    int y = event->y();
    convert_y(y, note);
    if (m_Previewing)
    {
        if (note != m_PreviewKey)
        {
            m_seq.play_note_off(m_PreviewKey);
            m_seq.play_note_on(note);
            m_PreviewKey = note;
        }
    }
    update();
}

/**
 *
 */

QSize
qseqkeys::sizeHint () const
{
    return QSize(c_keyarea_x, m_key_area_y);
}

/**
 *
 */

void
qseqkeys::convert_y (int y, int & note)
{
    note = (m_key_area_y - y - 2) / m_key_y;
}

}           // namespace seq64

/*
 * qseqkeys.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

