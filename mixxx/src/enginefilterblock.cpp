/***************************************************************************
                          enginefilterblock.cpp  -  description
                             -------------------
    begin                : Thu Apr 4 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "enginefilterblock.h"

EngineFilterBlock::EngineFilterBlock(QKnob *DialFilterLow, int midiLow,
                                     QKnob *DialFilterMid, int midiMid,
                                     QKnob *DialFilterHigh, int midiHigh, MidiObject *midi)
{
    gainLow = gainMid = gainHigh = 1.;

    //low = new EngineFilterRBJ(true,700.,1.);
    low = new EngineFilterIIR(bessel_lowpass);
    filterpotLow = new ControlLogpotmeter("filterpot", midiLow, midi, 5.);
    connect(filterpotLow,  SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdateLow(FLOAT_TYPE)));
    connect(DialFilterLow, SIGNAL(valueChanged(int)), filterpotLow, SLOT(slotSetPosition(int)));
    connect(filterpotLow, SIGNAL(recievedMidi(int)), DialFilterLow, SLOT(setValue(int)));

    filterpotMid = new ControlLogpotmeter("filterpot", midiMid, midi, 5.);
    connect(filterpotMid,  SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdateMid(FLOAT_TYPE)));
    connect(DialFilterMid, SIGNAL(valueChanged(int)), filterpotMid, SLOT(slotSetPosition(int)));
    connect(filterpotMid, SIGNAL(recievedMidi(int)), DialFilterMid, SLOT(setValue(int)));
  	
    //high = new EngineFilterRBJ(true,700.,1.);
    high = new EngineFilterIIR(bessel_highpass_5000);
    filterpotHigh = new ControlLogpotmeter("filterpot", midiHigh, midi, 5.);
    connect(filterpotHigh,  SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdateHigh(FLOAT_TYPE)));
    connect(DialFilterHigh, SIGNAL(valueChanged(int)), filterpotHigh, SLOT(slotSetPosition(int)));
    connect(filterpotHigh, SIGNAL(recievedMidi(int)), DialFilterHigh, SLOT(setValue(int)));

    buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineFilterBlock::~EngineFilterBlock()
{
	delete [] buffer;
	delete high;
	delete low;
}

CSAMPLE *EngineFilterBlock::process(const CSAMPLE *source, const int buf_size)
{
    CSAMPLE *p0 = low->process(source,buf_size);
    CSAMPLE *p1 = high->process(source,buf_size);
    
    for (int i=0; i<buf_size; i++)
	    buffer[i] = gainLow*p0[i] + gainHigh*p1[i] + gainMid*(source[i]-p0[i]-p1[i]);
    
    return buffer;
}

void EngineFilterBlock::slotUpdateLow(FLOAT_TYPE gain)
{
    gainLow = gain;
}

void EngineFilterBlock::slotUpdateMid(FLOAT_TYPE gain)
{
    gainMid = gain;
}

void EngineFilterBlock::slotUpdateHigh(FLOAT_TYPE gain)
{
    gainHigh = gain;
}
