//
// C++ Implementation: parserpls
//
// Description: module to parse pls formated playlists
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "parser.h"
#include "parserpls.h"

/**
@author Ingo Kossyk (kossyki@cs.tu-berlin.de)
**/

/**
ToDo:
    - parse ALL informations from the pls file if available ,
	  not only the filepath;
**/

ParserPls::ParserPls()
{
    m_psLocations = new QPtrList<QString>;
}

ParserPls::~ParserPls()
{

    delete m_psLocations;
}

QPtrList<QString> * ParserPls::parse(QString sFilename)
{
    //long numEntries =0;
    QFile * file = new QFile(sFilename);

    clearLocations();

    if (file->open(IO_ReadOnly) && !isBinary(sFilename) ) {

        QTextStream * textstream = new QTextStream( file );

        //numEntries = getNumEntries(textstream);

        //delete textstream;

        //textstream = new QTextStream( file );

        while(true){

            QString * psLine = new QString(getFilepath(textstream));
            
            const char * test = psLine->latin1();
            
            if(!(psLine->contains("NULL")))
                m_psLocations->append(psLine);
            else
                break;

            //--numEntries;
        }

        file->close();

        if(m_psLocations->count() != 0)
            return m_psLocations;
        else
            return 0;		// NULL pointer returned when no locations were found
    }

    file->close();
    return 0; //if we get here something went wrong :D
}

long ParserPls::getNumEntries(QTextStream * stream)
{

    QString textline;

    textline = stream->readLine();

    if(textline.contains("[playlist]")){

        while(!textline.contains("NumberOfEntries"))
            textline = stream->readLine();
        
        QString temp = textline.section("=",-1,-1);

        return temp.toLong();

    }else{
        qDebug( "ParserPls: pls file is not a playlist! \n" );
        return 0;
    }

}


QString ParserPls::getFilepath(QTextStream * stream)
{
    QString textline,filename = "";

    while(!(textline = stream->readLine()).isNull())

        if(textline.contains("File"))
        {

            int iPos = textline.find("=",0);
            ++iPos;

            filename = textline.right(textline.length()-iPos);
            break;
        }

    if(isFilepath(filename))
        return filename;
    else
        return QString("NULL");

}
