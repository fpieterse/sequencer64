#ifndef SEQ64_QSEQTIME_HPP
#define SEQ64_QSEQTIME_HPP

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
 * \file          qseqtime.hpp
 *
 *  This module declares/defines the base class for drawing the
 *  time/measures bar at the top of the patterns/sequence editor.
 *
 * \library       sequencer64 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2018-01-01
 * \updates       2018-07-14
 * \license       GNU GPLv2 or above
 *
 */

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPen>

#include "app_limits.h"                 /* SEQ64_DEFAULT_ZOOM               */
#include "qseqbase.hpp"                 /* seq64::qseqbase mixin class      */

/*
 *  Do not document a namespace; it breaks Doxygen.
 */

namespace seq64
{
    class perform;
    class sequence;

/**
 * The timebar for the sequence editor
 */

class qseqtime : public QWidget, public qseqbase
{
    Q_OBJECT

public:

    explicit qseqtime
    (
        perform & p,
        sequence & seq,
        int zoom            = SEQ64_DEFAULT_ZOOM,
        int ppqn            = SEQ64_USE_DEFAULT_PPQN,
        QWidget * parent    = nullptr
    );

    virtual ~qseqtime ()
    {
        // no code needed
    }

protected:

    void paintEvent (QPaintEvent *); // painting event to draw on the frame
    void mousePressEvent (QMouseEvent * event);
    void mouseReleaseEvent (QMouseEvent * event);
    void mouseMoveEvent (QMouseEvent * event);
    QSize sizeHint() const;

signals:

private slots:

    void conditional_update ();

private:

    QTimer * m_timer;
    QFont m_font;

};          // class qseqtime

}           // namespace seq64

#endif      // SEQ64_QSEQTIME_HPP

/*
 * qseqtime.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

