#ifndef SEQ64_RTMIDI_HPP
#define SEQ64_RTMIDI_HPP

/**
 * \file          rtmidi.hpp
 *
 *  An abstract base class for realtime MIDI input/output.
 *
 * \library       sequencer64 application
 * \author        Gary P. Scavone; refactoring by Chris Ahlstrom
 * \date          2016-11-14
 * \updates       2017-03-21
 * \license       See the rtexmidi.lic file.  Too big for a header file.
 *
 *  The big difference between this class (seq64::rtmidi) and
 *  seq64::rtmidi_info is that it gets information via midi_api-derived
 *  functions, while the latter gets if via midi_api_info-derived functions.
 */

#include <string>

#include "seq64_rtmidi_features.h"          /* defines what's implemented   */
#include "midi_api.hpp"                     /* seq64::midi[_in][_out]_api   */
#include "easy_macros.h"                    /* platform macros for compiler */
#include "rterror.hpp"                      /* seq64::rterror               */
#include "rtmidi_types.hpp"                 /* seq64::rtmidi_api etc.       */
#include "rtmidi_info.hpp"                  /* seq64::rtmidi_info           */

/*
 * Do not document the namespace; it breaks Doxygen.
 */

namespace seq64
{

/**
 *  The main class of the rtmidi API.  We moved the enum Api definition into
 *  the new rtmidi_types.hpp module to make refactoring the code easier.
 */

class rtmidi : public midi_api
{
    friend class midibus;

private:

    /**
     *  Holds a reference to the "global" midi_info wrapper object.
     *  Unlike the original RtMidi library, this library separates the
     *  port-enumeration code ("info") from the port-usage code ("api").
     *
     *  We might make it a static object at some point.
     */

    rtmidi_info & m_midi_info;

    /**
     *  Points to the API I/O object (e.g. midi_alsa or midi_jack) for which
     *  this class is a wrapper.
     */

    midi_api * m_midi_api;

protected:

    rtmidi (midibus & parentbus, rtmidi_info & info);
    virtual ~rtmidi ();

public:

    virtual bool api_connect ()
    {
        return get_api()->api_connect();
    }

    virtual void api_play (event * e24, midibyte channel)
    {
        get_api()->api_play(e24, channel);
    }

    virtual void api_continue_from (midipulse tick, midipulse beats)
    {
        get_api()->api_continue_from(tick, beats);
    }

    virtual void api_start ()
    {
        get_api()->api_start();
    }

    virtual void api_stop ()
    {
        get_api()->api_stop();
    }

    virtual void api_clock (midipulse tick)
    {
        get_api()->api_clock(tick);
    }

    virtual void api_set_ppqn (int ppqn)
    {
        get_api()->api_set_ppqn(ppqn);
    }

    virtual void api_set_beats_per_minute (midibpm bpm)
    {
        get_api()->api_set_beats_per_minute(bpm);
    }

    virtual bool api_init_out ()
    {
        return get_api()->api_init_out();
    }

    virtual bool api_init_out_sub ()
    {
        return get_api()->api_init_out_sub();
    }

    virtual bool api_init_in ()
    {
        return get_api()->api_init_in();
    }

    virtual bool api_init_in_sub ()
    {
        return get_api()->api_init_in_sub();
    }

    virtual bool api_deinit_in ()
    {
        return get_api()->api_deinit_in();
    }

    virtual bool api_get_midi_event (event * inev)
    {
        return get_api()->api_get_midi_event(inev);
    }

    virtual int api_poll_for_midi ()
    {
        return get_api()->api_poll_for_midi();
    }

    virtual void api_sysex (event * e24)
    {
        get_api()->api_sysex(e24);
    }

    virtual void api_flush ()
    {
        get_api()->api_flush();
    }

public:

    /**
     *  Returns true if a port is open and false if not.
     */

    virtual bool is_port_open () const
    {
       return get_api()->is_port_open();
    }

    /**
     *  Gets the buss/client ID for a MIDI interfaces.  This is the left-hand
     *  side of a X:Y pair (such as 128:0).
     *
     *  This function is a new part of the RtMidi interface.
     *
     * \return
     *      Returns the buss/client value as provided by the selected API.
     */

    virtual int get_bus_id ()
    {
        return get_api()->get_bus_id();
    }

    /**
     * \return
     *      Returns the buss name from the selected API subsystem.
     */

    virtual std::string get_bus_name ()
    {
        return get_api()->bus_name();
    }

    /**
     * \return
     *      Returns the port ID number from the selected API subsystem.
     */

    virtual int get_port_id ()
    {
        return get_api()->get_port_id();
    }

    /**
     * \return
     *      Returns the port name from the selected API subsystem.
     */

    virtual std::string get_port_name ()
    {
        return get_api()->port_name();
    }

    /**
     *  \return
     *      This value depends on the MIDI mode setting (input versus output).
     */

    int get_port_count ()
    {
        return m_midi_info.get_port_count();
    }

    /**
     *  \return
     *      This value is the sum of the number of input and output ports.
     */

    int full_port_count ()
    {
        return m_midi_info.full_port_count();
    }

    /**
     * \getter m_midi_api const version
     */

    const midi_api * get_api () const
    {
        return m_midi_api;
    }

    /**
     * \getter m_midi_api non-const version
     */

    midi_api * get_api ()
    {
        return m_midi_api;
    }

protected:

    /**
     * \setter m_midi_api
     */

    void set_api (midi_api * ma)
    {
        if (not_nullptr(ma))
            m_midi_api = ma;
    }

    /**
     * \setter m_midi_api
     */

    void delete_api ()
    {
        if (not_nullptr(m_midi_api))
        {
            delete m_midi_api;
            m_midi_api = nullptr;
        }
    }

protected:

};          // class rtmidi

/**
 *  A realtime MIDI input class.
 *
 *  This class provides a common, platform-independent API for realtime MIDI
 *  input.  It allows access to a single MIDI input port.  Incoming MIDI
 *  messages are either saved to a queue for retrieval using the get_message()
 *  function or immediately passed to a user-specified callback function.
 *  Create multiple instances of this class to connect to more than one MIDI
 *  device at the same time.  With the OS-X, Linux ALSA, and JACK MIDI APIs,
 *  it is also possible to open a virtual input port to which other MIDI
 *  software clients can connect.
 */

class rtmidi_in : public rtmidi
{

public:

    rtmidi_in (midibus & parentbus, rtmidi_info & info);
    virtual ~rtmidi_in ();

    /**
     *  Set a callback function to be invoked for incoming MIDI messages.
     *
     *  The callback function will be called whenever an incoming MIDI
     *  message is received.  While not absolutely necessary, it is best
     *  to set the callback function before opening a MIDI port to avoid
     *  leaving some messages in the queue.
     *
     * \param callback
     *      A callback function must be given.
     *
     * \param userdata
     *      Optionally, a pointer to additional data can be passed to the
     *      callback function whenever it is called.
     */

    void user_callback (rtmidi_callback_t callback, void * userdata = nullptr)
    {
       dynamic_cast<midi_api *>(get_api())->user_callback(callback, userdata);
    }

    /**
     *  Cancel use of the current callback function (if one exists).
     *
     *  Subsequent incoming MIDI messages will be written to the queue
     *  and can be retrieved with the \e get_message function.
     */

    void cancel_callback ()
    {
       dynamic_cast<midi_api *>(get_api())->cancel_callback();
    }

protected:

    void openmidi_api
    (
        rtmidi_api api, rtmidi_info & info //, int index = SEQ64_NO_INDEX
    );

};

/**
 *  A realtime MIDI output class.
 *
 *  This class provides a common, platform-independent API for MIDI output.
 *  It allows one to probe available MIDI output ports, to connect to one such
 *  port, and to send MIDI bytes immediately over the connection.  Create
 *  multiple instances of this class to connect to more than one MIDI device
 *  at the same time.  With the OS-X, Linux ALSA and JACK MIDI APIs, it is
 *  also possible to open a virtual port to which other MIDI software clients
 *  can connect.
 */

class rtmidi_out : public rtmidi
{

public:

    rtmidi_out (midibus & parentbus, rtmidi_info & info);

    /**
     *  The destructor closes any open MIDI connections.
     */

    virtual ~rtmidi_out ();

protected:

    void openmidi_api
    (
        rtmidi_api api, rtmidi_info & info // , int index = SEQ64_NO_INDEX
    );

};          // class rtmidi_out

}           // namespace seq64

#endif      // SEQ64_RTMIDI_HPP

/*
 * rtmidi.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

