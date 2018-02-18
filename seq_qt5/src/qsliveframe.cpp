#include "qsliveframe.hpp"
#include "globals.h"
#include "forms/qsliveframe.ui.h"

qsliveframe::qsliveframe(QWidget *parent, perform *perf) :
    QFrame(parent),
    ui(new Ui::qsliveframe),
    mPerf(perf),
    m_bank_id(0),
    mAddingNew(false),
    mCanPaste(false)
{
    setSizePolicy(QSizePolicy::Expanding,
                  QSizePolicy::Expanding);

    setFocusPolicy(Qt::StrongFocus);

    ui->setupUi(this);

    mMsgBoxNewSeqCheck = new QMessageBox(this);
    mMsgBoxNewSeqCheck->setText(tr("Sequence already present"));
    mMsgBoxNewSeqCheck->setInformativeText(tr("There is already a sequence stored in this slot. Overwrite it and create a new blank sequence?"));
    mMsgBoxNewSeqCheck->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    mMsgBoxNewSeqCheck->setDefaultButton(QMessageBox::No);

    setBank(0);

    QString bankName = (*mPerf->get_bank_name(m_bank_id)).c_str();
    ui->txtBankName->setPlainText(bankName);

    connect(ui->spinBank,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(updateBank(int)));

    connect(ui->txtBankName,
            SIGNAL(textChanged()),
            this,
            SLOT(updateBankName()));

    //start refresh timer to queue regular redraws
    mRedrawTimer = new QTimer(this);
    mRedrawTimer->setInterval(50);
    connect(mRedrawTimer,
            SIGNAL(timeout()),
            this,
            SLOT(update()));
    mRedrawTimer->start();

}

void qsliveframe::paintEvent(QPaintEvent *)
{
    drawAllSequences();
}

qsliveframe::~qsliveframe()
{
    delete ui;
    delete mMsgBoxNewSeqCheck;
    delete mRedrawTimer;
}

void qsliveframe::drawSequence(int a_seq)
{
    mPainter = new QPainter(this);
    mPen = new QPen(Qt::black);
    mBrush = new QBrush(Qt::darkGray);
    mFont.setPointSize(6);
    mFont.setLetterSpacing(QFont::AbsoluteSpacing,
                           1);
    mPainter->setPen(*mPen);
    mPainter->setBrush(*mBrush);
    mPainter->setFont(mFont);

    //timing info for timed draw elements
    long tick = mPerf->get_tick();
    int metro = (tick / c_ppqn) % 2;

    //grab frame dimensions for scaled drawing
    thumbW = (ui->frame->width() - 1 - c_mainwid_spacing * 8)
             / c_mainwnd_cols;
    thumbH = (ui->frame->height() - 1 - c_mainwid_spacing * 5)
             / c_mainwnd_rows;

    previewW = thumbW - mFont.pointSize() * 2;
    previewH = thumbH - mFont.pointSize() * 5;

    if (a_seq >= (m_bank_id  * c_mainwnd_rows * c_mainwnd_cols) &&
            a_seq < ((m_bank_id + 1)  * c_mainwnd_rows * c_mainwnd_cols))
    {
        int i = (a_seq / c_mainwnd_rows) % c_mainwnd_cols;
        int j =  a_seq % c_mainwnd_rows;

        int base_x = (ui->frame->x() + 1 +
                      (thumbW + c_mainwid_spacing) * i);
        int base_y = (ui->frame->y() + 1 +
                      (thumbH + c_mainwid_spacing) * j);


        if (mPerf->is_active(a_seq))
        {
            sequence *seq = mPerf->get_sequence(a_seq);

            //get seq's assigned colour
            QColor backColour =
            QColor(colourMap.value(mPerf->get_sequence_color(a_seq)));

            mPen->setColor(Qt::black);
            mPen->setStyle(Qt::SolidLine);

            if (seq->get_playing() &&
                    (seq->get_queued() || seq->getOffFromSnap()))
                //playing but queued to mute, or
                //turning off after snapping
            {
                mPen->setWidth(2);
                mPen->setColor(Qt::black);
                mPen->setStyle(Qt::DashLine);
                mPainter->setPen(*mPen);
                backColour.setAlpha(210);
                mBrush->setColor(backColour);
                mPainter->setBrush(*mBrush);
                mPainter->drawRect(base_x,
                                   base_y,
                                   thumbW + 1,
                                   thumbH + 1);
            }
            else if (seq->get_playing())
                //playing, no queueing
            {
                mPen->setWidth(2);
                mPainter->setPen(*mPen);
                backColour.setAlpha(210);
                mBrush->setColor(backColour);
                mPainter->setBrush(*mBrush);
                mPainter->drawRect(base_x,
                                   base_y,
                                   thumbW + 1,
                                   thumbH + 1);
            }
            else if (seq->get_queued())
                //not playing but queued
            {
                mPen->setWidth(2);
                mPen->setColor(Qt::darkGray);
                mPen->setStyle(Qt::DashLine);
                backColour.setAlpha(180);
                mBrush->setColor(backColour);
                mPainter->setPen(*mPen);
                mPainter->setBrush(*mBrush);
                mPainter->drawRect(base_x,
                                   base_y,
                                   thumbW,
                                   thumbH);
            }
            else if (seq->getOneshot())
                //queued for one-shot
            {
                mPen->setWidth(2);
                mPen->setColor(Qt::darkGray);
                mPen->setStyle(Qt::DotLine);
                backColour.setAlpha(180);
                mBrush->setColor(backColour);
                mPainter->setPen(*mPen);
                mPainter->setBrush(*mBrush);
                mPainter->drawRect(base_x,
                                   base_y,
                                   thumbW,
                                   thumbH);
            }
            else
                //just not playing
            {
                mPen->setStyle(Qt::NoPen);
                backColour.setAlpha(180);
                mBrush->setColor(backColour);
                mPainter->setPen(*mPen);
                mPainter->setBrush(*mBrush);
                mPainter->drawRect(base_x,
                                   base_y,
                                   thumbW,
                                   thumbH);
            }

            //write seq data strings
            ///name
            mPen->setColor(Qt::black);
            mPen->setWidth(1);
            mPen->setStyle(Qt::SolidLine);
            mPainter->setPen(*mPen);
            char name[20];
            snprintf(name, sizeof name, "%.13s", seq->name());
            mPainter->drawText(base_x + c_text_x,
                               base_y + 4,
                               80,
                               80,
                               1,
                               name);

            /* midi channel + key + timesig */
            if (mPerf->show_ui_sequence_key())
            {
                QString key; //when looking up key, ignore bank offset (print keys on every bank)
                key[0] = (char)mPerf->lookup_keyevent_key(a_seq - mPerf->getBank() * c_seqs_in_set);
                mPainter->drawText(base_x + thumbW - 10,
                                   base_y + thumbH - 5,
                                   key);
            }

            QString seqInfo = QString::number(seq->get_midi_bus() + 1);
            seqInfo.append("-");
            seqInfo.append(QString::number(seq->get_midi_channel() + 1));

            mPainter->drawText(base_x + 5,
                               base_y + thumbH - 5,
                               seqInfo);

            int rectangle_x = base_x + 7;
            int rectangle_y = base_y + 15;

            mPen->setColor(Qt::gray);
            mBrush->setStyle(Qt::NoBrush);
            mPainter->setBrush(*mBrush);
            mPainter->setPen(*mPen);
            //draw inner box for notes
            mPainter->drawRect(rectangle_x - 2,
                               rectangle_y - 1,
                               previewW,
                               previewH);

            int lowest_note = seq->get_lowest_note_event();
            int highest_note = seq->get_highest_note_event();

            int height = highest_note - lowest_note;
            height += 2;

            int length = seq->getLength();

            long tick_s;
            long tick_f;
            int note;

            bool selected;

            int velocity;
            draw_type dt;

            seq->reset_draw_marker();

            //add padding to box measurements
            previewH -= 6;
            previewW -= 6;
            rectangle_x += 2;
            rectangle_y += 2;

            while ((dt = seq->get_next_note_event(&tick_s, &tick_f, &note,
                                                  &selected, &velocity)) != DRAW_FIN)
            {

                int note_y = previewH -
                             (previewH  * (note + 1 - lowest_note)) / height ;

                int tick_s_x = (tick_s * previewW)  / length;
                int tick_f_x = (tick_f * previewH)  / length;

                if (dt == DRAW_NOTE_ON || dt == DRAW_NOTE_OFF)
                    tick_f_x = tick_s_x + 1;
                if (tick_f_x <= tick_s_x)
                    tick_f_x = tick_s_x + 1;

                //draw line representing this note
                mPen->setColor(Qt::black);
                mPen->setWidth(2);
                mPainter->setPen(*mPen);
                mPainter->drawLine(rectangle_x + tick_s_x,
                                   rectangle_y + note_y,
                                   rectangle_x + tick_f_x,
                                   rectangle_y + note_y);
            }

            //draw playhead
            int a_tick = mPerf->get_tick();
            a_tick += (length - seq->get_trigger_offset());
            a_tick %= length;

            long tick_x = a_tick * previewW / length;

            if (seq->get_playing())
                mPen->setColor(Qt::red);
            else
                mPen->setColor(Qt::black);

            if (seq->get_queued() ||
                    (seq->getOffFromSnap() && seq->get_playing()))
            {
                mPen->setColor(Qt::green);
            }
            else if (seq->getOneshot())
            {
                mPen->setColor(Qt::blue);
            }

            mPen->setWidth(1);
            mPainter->setPen(*mPen);
            mPainter->drawLine(rectangle_x + tick_x - 1,
                               rectangle_y - 1,
                               rectangle_x + tick_x - 1,
                               rectangle_y + previewH + 1);
        }
        else
        {
            mPen->setColor(Qt::black);
            mPen->setStyle(Qt::NoPen);
            mFont.setPointSize(15);
            mPainter->setPen(*mPen);
            mPainter->setFont(mFont);

            //draw outline of this seq thumbnail
            mPainter->drawRect(base_x,
                               base_y,
                               thumbW,
                               thumbH);

            //no sequence present. Insert placeholder
            mPen->setStyle(Qt::SolidLine);
            mPainter->setPen(*mPen);
            mPainter->drawText(base_x + 2,
                               base_y + 17,
                               "+");
        }
    }
    //lessen alpha on each redraw to have smooth fading
    //done as a factor of the bpm to get useful fades
    alpha *= 0.7 - mPerf->get_bpm() / 300;

    lastMetro = metro;

    delete mPainter;
    delete mPen;
    delete mBrush;
}
void qsliveframe::drawAllSequences()
{
    for (int i = 0; i < (c_mainwnd_rows * c_mainwnd_cols); i++)
    {
        drawSequence(i + (m_bank_id * c_mainwnd_rows * c_mainwnd_cols));

        m_last_tick_x[i + (m_bank_id * c_mainwnd_rows * c_mainwnd_cols)] = 0;
    }
}

void qsliveframe::setBank(int newBank)
{
    m_bank_id = newBank;

    if (m_bank_id < 0)
        m_bank_id = c_max_num_banks - 1;

    if (m_bank_id >= c_max_num_banks)
        m_bank_id = 0;

    mPerf->set_offset(m_bank_id);

    QString bankName = (*mPerf->get_bank_name(m_bank_id)).c_str();
    ui->txtBankName->setPlainText(bankName);

    ui->spinBank->setValue(m_bank_id);

    update();

    qDebug() << "Newly selected bank" << endl
             << "Name - " << bankName << endl
             << "ID - " << m_bank_id << endl;

}

void qsliveframe::redraw()
{
    drawAllSequences();
}

void qsliveframe::updateBank(int newBank)
{
    mPerf->setBank(newBank);
    setBank(newBank);
    mPerf->setModified(true);
}

void qsliveframe::updateBankName()
{
    updateInternalBankName();
    mPerf->setModified(true);
}

void qsliveframe::updateInternalBankName()
{
    string newName =
        ui->txtBankName->document()->toPlainText().toStdString();

    qDebug() << "qsliveframe.cpp, New bank name is - "
             << QString(newName.c_str()) << endl;

    mPerf->setBankName(m_bank_id, &newName);
}

int qsliveframe::seqIDFromClickXY(int click_x, int click_y)
{
    /* adjust for border */
    int x = click_x - c_mainwid_border;
    int y = click_y - c_mainwid_border;

    /* is it in the box ? */
    if (x < 0
            || x >= ((thumbW + c_mainwid_spacing) * c_mainwnd_cols)
            || y < 0
            || y >= ((thumbH + c_mainwid_spacing) * c_mainwnd_rows))
    {

        return -1;
    }

    /* gives us in box corrdinates */
    int box_test_x = x % (thumbW + c_mainwid_spacing);
    int box_test_y = y % (thumbH + c_mainwid_spacing);

    /* right inactive side of area */
    if (box_test_x > thumbW
            || box_test_y > thumbH)
    {

        return -1;
    }

    x /= (thumbW + c_mainwid_spacing);
    y /= (thumbH + c_mainwid_spacing);

    int seqId = ((x * c_mainwnd_rows + y)
                 + (m_bank_id * c_mainwnd_rows * c_mainwnd_cols));

    return seqId;

}

void qsliveframe::mousePressEvent(QMouseEvent *event)
{
    mCurrentSeq = seqIDFromClickXY(
                      event->x(), event->y());

    if (mCurrentSeq != -1
            && event->button() == Qt::LeftButton)
    {
        mButtonDown = true;
    }
}

void qsliveframe::mouseReleaseEvent(QMouseEvent *event)
{
    /* get the sequence number we clicked on */
    mCurrentSeq = seqIDFromClickXY(
                      event->x(), event->y());

    mButtonDown = false;

    /* if we're on a valid sequence, hit the left mouse button,
          * and are not dragging a sequence - toggle playing*/
    if (mCurrentSeq != -1
            && event->button() == Qt::LeftButton
            && !mMoving)
    {
        if (mPerf->is_active(mCurrentSeq))
        {
            if (!mAddingNew)
            {
                mPerf->sequence_playing_toggle(mCurrentSeq);
            }
            mAddingNew = false;
            update();
        }
        else
        {
            mAddingNew = true;
        }
    }

    /* if left mouse button & we're moving a seq between slots */
    if (event->button() == Qt::LeftButton
            && mMoving)
    {
        mMoving = false;

        if (!mPerf->is_active(mCurrentSeq)
                && mCurrentSeq != -1
                && !mPerf->is_sequence_in_edit(mCurrentSeq))
        {
            mPerf->new_sequence(mCurrentSeq);
            *(mPerf->get_sequence(mCurrentSeq)) = m_moving_seq;

            update();

        }
        else
        {
            mPerf->new_sequence(mOldSeq);
            *(mPerf->get_sequence(mOldSeq)) = m_moving_seq;

            update();
        }
    }

    /* check for right mouse click - this launches the popup menu */
    if (mCurrentSeq != -1
            && event->button() == Qt::RightButton)
    {
        mPopup = new QMenu(this);

        //new sequence option
        QAction *actionNew = new QAction(tr("New sequence"), mPopup);
        mPopup->addAction(actionNew);
        QObject::connect(actionNew,
                         SIGNAL(triggered(bool)),
                         this,
                         SLOT(newSeq()));

        if (mPerf->is_active(mCurrentSeq))
        {
            //edit sequence
            QAction *actionEdit = new QAction(tr("Edit sequence"), mPopup);
            mPopup->addAction(actionEdit);
            connect(actionEdit,
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(editSeq()));

            //set the colour from the scheme
            QMenu *menuColour = new QMenu(tr("Set colour..."));

            QAction *actionColours[8];

            actionColours[0] = new QAction(tr("White"), menuColour);
            actionColours[1] = new QAction(tr("Red"), menuColour);
            actionColours[2] = new QAction(tr("Green"), menuColour);
            actionColours[3] = new QAction(tr("Blue"), menuColour);
            actionColours[4] = new QAction(tr("Yellow"), menuColour);
            actionColours[5] = new QAction(tr("Purple"), menuColour);
            actionColours[6] = new QAction(tr("Pink"), menuColour);
            actionColours[7] = new QAction(tr("Orange"), menuColour);

            connect(actionColours[0],
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(setColourWhite()));

            connect(actionColours[1],
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(setColourRed()));

            connect(actionColours[2],
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(setColourGreen()));

            connect(actionColours[3],
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(setColourBlue()));

            connect(actionColours[4],
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(setColourYellow()));

            connect(actionColours[5],
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(setColourPurple()));

            connect(actionColours[6],
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(setColourPink()));

            connect(actionColours[7],
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(setColourOrange()));

            for (int i = 0; i < 8; i++)
            {
                menuColour->addAction(actionColours[i]);
            }

            mPopup->addMenu(menuColour);

            //copy sequence
            QAction *actionCopy = new QAction(tr("Copy sequence"), mPopup);
            mPopup->addAction(actionCopy);
            connect(actionCopy,
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(copySeq()));

            //cut sequence
            QAction *actionCut = new QAction(tr("Cut sequence"), mPopup);
            mPopup->addAction(actionCut);
            connect(actionCut,
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(cutSeq()));

            //delete sequence
            QAction *actionDelete = new QAction(tr("Delete sequence"), mPopup);
            mPopup->addAction(actionDelete);
            connect(actionDelete,
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(deleteSeq()));

        }
        else if (mCanPaste)
        {
            //paste sequence
            QAction *actionPaste = new QAction(tr("Paste sequence"), mPopup);
            mPopup->addAction(actionPaste);
            connect(actionPaste,
                    SIGNAL(triggered(bool)),
                    this,
                    SLOT(pasteSeq()));
        }

        mPopup->exec(QCursor::pos());
    }

    //middle button launches seq editor
    if (mCurrentSeq != -1
            && event->button() == Qt::MiddleButton
            && mPerf->is_active(mCurrentSeq))
    {
        callEditor(mCurrentSeq);
    }
}

void qsliveframe::mouseMoveEvent(QMouseEvent *event)
{
    int seqId = seqIDFromClickXY(event->x(), event->y());

    if (mButtonDown)
    {
        if (seqId != mCurrentSeq
                && !mMoving
                && !mPerf->is_sequence_in_edit(mCurrentSeq))
        {
            /* lets drag a sequence between slots */
            if (mPerf->is_active(mCurrentSeq))
            {
                mOldSeq = mCurrentSeq;
                mMoving = true;

                /* save the sequence and clear the old slot */
                m_moving_seq = *(mPerf->get_sequence(mCurrentSeq));
                mPerf->delete_sequence(mCurrentSeq);

                update();
            }
        }
    }
}

void qsliveframe::mouseDoubleClickEvent(QMouseEvent *)
{
    if (mAddingNew)
    {
        newSeq();
    }
}

void qsliveframe::newSeq()
{
    if (mPerf->is_active(mCurrentSeq))
    {
        int choice = mMsgBoxNewSeqCheck->exec();
        if (choice == QMessageBox::No)
            return;
    }
    mPerf->new_sequence(mCurrentSeq);
    mPerf->get_sequence(mCurrentSeq)->set_dirty();
    //TODO reenable - disabled opening the editor for each new seq
    //    callEditor(m_main_perf->get_sequence(m_current_seq));

}

void qsliveframe::editSeq()
{
    callEditor(mCurrentSeq);
}

void qsliveframe::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_BracketLeft:
        setBank(m_bank_id - 1);
        break;
    case Qt::Key_BracketRight:
        setBank(m_bank_id + 1);
        break;
    case Qt::Key_Semicolon: //replace
        mPerf->set_sequence_control_status(c_status_replace);
        break;
    case Qt::Key_Slash: //queue
        mPerf->set_sequence_control_status(c_status_queue);
        break;
    case Qt::Key_Apostrophe || Qt::Key_NumberSign: //snapshot
        mPerf->set_sequence_control_status(c_status_snapshot);
        break;
    case Qt::Key_Period: //one-shot
        mPerf->set_sequence_control_status(c_status_oneshot);
        break;
    default: //sequence mute toggling
        quint32 keycode =  event->key();
        qDebug() << "Live frame key press - " << keycode << endl;
        if (mPerf->get_key_events()->count(event->key()) != 0)
            sequence_key(mPerf->lookup_keyevent_seq(event->key()));
        else
            event->ignore();
        break;
    }
}

void qsliveframe::keyReleaseEvent(QKeyEvent *event)
{
    //unset the relevant control modifiers
    switch (event->key())
    {
    case Qt::Key_Semicolon: //replace
        mPerf->unset_sequence_control_status(c_status_replace);
        break;
    case Qt::Key_Slash: //queue
        mPerf->unset_sequence_control_status(c_status_queue);
        break;
    case Qt::Key_Apostrophe || Qt::Key_NumberSign: //snapshot
        mPerf->unset_sequence_control_status(c_status_snapshot);
        break;
    case Qt::Key_Period: //one-shot
        mPerf->unset_sequence_control_status(c_status_oneshot);
        break;
    }
}

void qsliveframe::sequence_key(int a_seq)
{
    /* add bank offset */
    a_seq += mPerf->getBank() * c_mainwnd_rows * c_mainwnd_cols;

    if (mPerf->is_active(a_seq))
    {

        mPerf->sequence_playing_toggle(a_seq);
    }
}

void qsliveframe::setColourWhite()
{
    mPerf->setSequenceColour(mCurrentSeq, White);
}
void qsliveframe::setColourRed()
{
    mPerf->setSequenceColour(mCurrentSeq, Red);
}
void qsliveframe::setColourGreen()
{
    mPerf->setSequenceColour(mCurrentSeq, Green);
}
void qsliveframe::setColourBlue()
{
    mPerf->setSequenceColour(mCurrentSeq, Blue);
}
void qsliveframe::setColourYellow()
{
    mPerf->setSequenceColour(mCurrentSeq, Yellow);
}
void qsliveframe::setColourPurple()
{
    mPerf->setSequenceColour(mCurrentSeq, Purple);
}
void qsliveframe::setColourPink()
{
    mPerf->setSequenceColour(mCurrentSeq, Pink);
}
void qsliveframe::setColourOrange()
{
    mPerf->setSequenceColour(mCurrentSeq, Orange);
}

void qsliveframe::copySeq()
{
    if (mPerf->is_active(mCurrentSeq))
    {
        mSeqClipboard = *(mPerf->get_sequence(mCurrentSeq));
        mCanPaste = true;
    }
}

void qsliveframe::cutSeq()
{
    if (mPerf->is_active(mCurrentSeq) &&
            !mPerf->is_sequence_in_edit(mCurrentSeq))
        //TODO dialog warning that the editor is the reason
        //this seq cant be cut
    {
        mSeqClipboard = *(mPerf->get_sequence(mCurrentSeq));
        mCanPaste = true;
        mPerf->delete_sequence(mCurrentSeq);
    }
}

void qsliveframe::deleteSeq()
{
    if (mPerf->is_active(mCurrentSeq) &&
            !mPerf->is_sequence_in_edit(mCurrentSeq))
        //TODO dialog warning that the editor is the reason
        //this seq cant be deleted
        mPerf->delete_sequence(mCurrentSeq);
}

void qsliveframe::pasteSeq()
{
    if (! mPerf->is_active(mCurrentSeq))
    {
        mPerf->new_sequence(mCurrentSeq);
        *(mPerf->get_sequence(mCurrentSeq)) = mSeqClipboard;
        mPerf->get_sequence(mCurrentSeq)->set_dirty();
    }
}
