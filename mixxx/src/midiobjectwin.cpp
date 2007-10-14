/***************************************************************************
                          midiobjectcoremidi.cpp  -  description
                             -------------------
    begin                : Thu Jul 4 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "midiobjectwin.h"
#include "midiledhandler.h"
#include <atlconv.h>
#include <QtDebug>

#ifdef __WIN__
#ifdef __MSVS2005__
#pragma comment (lib, "atls.lib")
#endif
#endif

MidiObjectWin::MidiObjectWin(QString device) : MidiObject(device)
{
	updateDeviceList();

    /*
       // Don't open the device yet, it gets opened via dlgprefmidi soon
       // This is how the ALSA one does it anyway... -Adam
       // Open device
       if (device_valid)
       devOpen(device);
       else
       if (devices.count()==0)
        qDebug("MIDI: No MIDI devices available.");
       else
        devOpen(devices.first());*/
}

MidiObjectWin::~MidiObjectWin()
{
    // Close device and delete buffer
    devClose();
}

void MidiObjectWin::updateDeviceList() {

    USES_CONVERSION; // enable WCHAR to char macro conversion
    // Fill in list of available devices
	devices.clear();

    MIDIINCAPS info;
    for (unsigned int i=0; i<midiInGetNumDevs(); i++)
    {
        MMRESULT res = midiInGetDevCaps(i, &info, sizeof(MIDIINCAPS));

        qDebug("Midi Device '%s' found.", W2A(info.szPname));

        if (strlen(W2A(info.szPname))>0)
            devices.append(W2A(info.szPname));
        else
            devices.append(QString("Device %1").arg(i));
    }
}

void MidiObjectWin::devOpen(QString device)
{
    USES_CONVERSION; // enable WCHAR to char macro conversion
    // Select device. If not found, select default (first in list).
    unsigned int i;
    MIDIINCAPS info;
    for (i=0; i<midiInGetNumDevs(); i++)
    {
        MMRESULT res = midiInGetDevCaps(i, &info, sizeof(MIDIINCAPS));
        if (strlen(W2A(info.szPname))>0 && ((QString(W2A(info.szPname)) == device) || (QString("Device %1").arg(i) == device)))
        {
            qDebug("Using Midi Device #%i: %s", i, W2A(info.szPname));
            break;
        }
    }
    if (i==midiInGetNumDevs())
        i = 0;

//  handle = new HMIDIIN;
    MMRESULT res = midiInOpen(&handle, i, (DWORD)MidiInProc, (DWORD) this, CALLBACK_FUNCTION);
    if (res == MMSYSERR_NOERROR) {
        // Should follow selected device !!!!
        openDevice = device;
    } else {
        qDebug("Error opening midi device");
        return;
    }

    res = midiOutOpen(&outhandle, i, NULL, NULL, CALLBACK_NULL);
    if (res != MMSYSERR_NOERROR)
        qDebug("Error opening midi output for light control");        // Not so important

    res = midiInStart(handle);
    if (res != MMSYSERR_NOERROR)
        qDebug("Error starting midi.");
}

void MidiObjectWin::devClose()
{
    midiInReset(handle);
    midiInClose(handle);
    openDevice = QString("");
}

void MidiObjectWin::stop()
{
    MidiObject::stop();
}

void MidiObjectWin::run()
{
}

void MidiObjectWin::handleMidi(char channel, char midicontrol, char midivalue)
{
    qDebug("midi miditype: %X ch: %X, ctrl: %X, val: %X",(channel& 240),channel&15,midicontrol,midivalue);
    send((MidiCategory)(channel & 240), channel&15, midicontrol, midivalue);
}

// C/C++ wrapper function
void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    MidiObjectWin * midi = (MidiObjectWin *)dwInstance;
    switch (wMsg) {
    case MIM_DATA:
        qDebug("MIM_DATA");
        midi->handleMidi(dwParam1&0x000000ff, (dwParam1&0x0000ff00)>>8, (dwParam1&0x00ff0000)>>16);
        break;
    case MIM_LONGDATA:
        qDebug("MIM_LONGDATA");
        // for a MIM_LONGDATA implementation example refer to "void CALLBACK MidiInProc" @ http://www.csee.umbc.edu/help/sound/TiMidity++-2.13.2/interface/rtsyn_winmm.c
        break;
    }
}

void MidiObjectWin::sendShortMsg(unsigned int word) {
    // This checks your compiler isn't assigning some wierd type hopefully
    DWORD raw = word;
    midiOutShortMsg(outhandle, word);
}
