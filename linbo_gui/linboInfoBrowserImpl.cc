#include "linboInfoBrowserImpl.hh"
#include <qapplication.h>
#include <QtGui>
#include <q3textstream.h>

linboInfoBrowserImpl::linboInfoBrowserImpl(QWidget* parent ) : linboDialog()
{
   Ui_linboInfoBrowser::setupUi((QDialog*)this);

   process = new QProcess( this );
   
   if( parent)
     myParent = parent;

   connect( this->saveButton, SIGNAL(clicked()), this, SLOT(postcmd()));

   // connect SLOT for finished process
   connect( process, SIGNAL(finished(int, QProcess::ExitStatus) ),
	    this, SLOT(processFinished(int, QProcess::ExitStatus)) );

   // connect stdout and stderr to linbo console
   connect( process, SIGNAL(readyReadStandardOutput()),
	    this, SLOT(readFromStdout()) );
   
   connect( process, SIGNAL(readyReadStandardError()),
	    this, SLOT(readFromStderr()) );
   
   Qt::WindowFlags flags;
   flags = Qt::Dialog | Qt::WindowStaysOnTopHint ;
   setWindowFlags( flags );

   QRect qRect(QApplication::desktop()->screenGeometry());
   // open in the center of our screen
   int xpos=qRect.width()/2-this->width()/2;
   int ypos=qRect.height()/2-this->height()/2;
   this->move(xpos,ypos);
   this->setFixedSize( this->width(), this->height() );
}

linboInfoBrowserImpl::~linboInfoBrowserImpl()
{
  delete process;
} 

void linboInfoBrowserImpl::setTextBrowser( QTextEdit* newBrowser )
{
  Console = newBrowser;
}

void linboInfoBrowserImpl::setMainApp( QWidget* newMainApp ) {
  if ( newMainApp ) {
    myMainApp = newMainApp;
  }
}


void linboInfoBrowserImpl::precmd() {
  app = static_cast<linboGUIImpl*>( myMainApp );
  
  if( app ) {
    if ( app->isRoot() ) {
      saveButton->setText("Speichern");
      saveButton->setEnabled( true );
      editor->setReadOnly( false );
      // connect( this->saveButton, SIGNAL(clicked()), this, SLOT(postcmd()));
    } else {
      saveButton->setText("Schliessen");
      saveButton->setEnabled( true );
      editor->setReadOnly( true );
      // connect( this->saveButton, SIGNAL(clicked()), this, SLOT(close()));
    }

    arguments.clear();
    arguments = myLoadCommand;

#ifdef DEBUG
    Console->insert(QString("linboInfoBrowserImpl: myLoadCommand"));
    QStringList list = arguments;
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
      Console->insert( *it );
      ++it;
    }
    Console->insert(QString("*****"));
#endif

    QStringList processargs( arguments );
    QString command = processargs.takeFirst();

    process->start( command, processargs );

    while( process->state() == QProcess::Running ) {
      usleep( 1000 );
    }

    file = new QFile( filepath );
    // read content
    if( !file->open( QIODevice::ReadOnly ) ) {
      Console->setColor( QColor( QString("red") ) );
      Console->insert("Keine passende Beschreibung im Cache.");
      Console->insert(QString(QChar::LineSeparator));
      Console->moveCursor(QTextCursor::End);
      Console->ensureCursorVisible(); 
      Console->setColor( QColor( QString("white") ) );
    } 
    else {
      Q3TextStream ts( file );
      editor->setText( ts.read() );
      file->close();
    }
  }
  
}


void linboInfoBrowserImpl::postcmd() {
  
  if( app ) {
    if ( app->isRoot() ) {

      if ( !file->open( QIODevice::WriteOnly ) ) {
	Console->setColor( QColor( QString("red") ) );
        Console->insert("Fehler beim Speichern der Beschreibung.");
	Console->insert(QString(QChar::LineSeparator));
	Console->moveCursor(QTextCursor::End);
	Console->ensureCursorVisible(); 
	Console->setColor( QColor( QString("white") ) );
      } 
      else {
        Q3TextStream ts( file );
        ts << editor->text();
        file->flush();
        file->close();

	arguments.clear();
        arguments = mySaveCommand; 

#ifdef DEBUG
        Console->insert(QString("linboInfoBrowserImpl: mySaveCommand"));
        QStringList list = arguments;
        QStringList::Iterator it = list.begin();
      
        while( it != list.end() ) {
          Console->insert( *it );
          ++it;
        }
        Console->insert(QString("*****"));
#endif
	QStringList processargs( arguments );
	QString command = processargs.takeFirst();

	process->start( command, processargs );

	while( process->state() == QProcess::Running ) {
	  usleep( 1000 );
	}
      
	arguments.clear();
        arguments = myUploadCommand;

#ifdef DEBUG
        Console->insert(QString("linboInfoBrowserImpl: myUploadCommand"));
        list = arguments;
        it = list.begin();
      
        while( it != list.end() ) {
          Console->insert( *it );
          ++it;
        }
        Console->insert(QString("*****"));
#endif
	processargs.clear();
	processargs = arguments;
	command = processargs.takeFirst();

	process->start( command, processargs );

	while( process->state() == QProcess::Running ) {
	  usleep( 1000 );
	}


      }
    }
    this->close();
  }
  
}

void linboInfoBrowserImpl::setCommand( const QStringList& newArguments ) {
  myUploadCommand = newArguments;
}

void linboInfoBrowserImpl::setUploadCommand( const QStringList& newUploadCommand ) {
  myUploadCommand = newUploadCommand;
}

void linboInfoBrowserImpl::setLoadCommand( const QStringList& newLoadCommand ) {
  myLoadCommand = newLoadCommand;
}

void linboInfoBrowserImpl::setSaveCommand( const QStringList& newSaveCommand ) {
  mySaveCommand = newSaveCommand;
}

QStringList linboInfoBrowserImpl::getCommand() {
  // not needed here
  return arguments;
}

void linboInfoBrowserImpl::setFilePath( const QString& newFilepath ) {
  filepath = newFilepath;
}

void linboInfoBrowserImpl::readFromStdout()
{
  Console->setColor( QColor( QString("white") ) );
  Console->insert( process->readAllStandardOutput() );
  Console->moveCursor(QTextCursor::End);
  Console->ensureCursorVisible(); 
}

void linboInfoBrowserImpl::readFromStderr()
{
  Console->setColor( QColor( QString("red") ) );
  Console->insert( process->readAllStandardError() );
  Console->moveCursor(QTextCursor::End);
  Console->ensureCursorVisible(); 
  Console->setColor( QColor( QString("white") ) );
}

void linboInfoBrowserImpl::processFinished( int retval,
					     QProcess::ExitStatus status) {

  Console->setColor( QColor( QString("red") ) );
  Console->insert( QString("Command executed with exit value ") + QString::number( retval ) );

  if( status == 0)
    Console->insert( QString("Exit status: ") + QString("The process exited normally.") );
  else
    Console->insert( QString("Exit status: ") + QString("The process crashed.") );

  if( status == 1 ) {
    int errorstatus = process->error();
    switch ( errorstatus ) {
      case 0: Console->insert( QString("The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.") ); break;
      case 1: Console->insert( QString("The process crashed some time after starting successfully.") ); break;
      case 2: Console->insert( QString("The last waitFor...() function timed out.") ); break;
      case 3: Console->insert( QString("An error occurred when attempting to write to the process. For example, the process may not be running, or it may have closed its input channel.") ); break;
      case 4: Console->insert( QString("An error occurred when attempting to read from the process. For example, the process may not be running.") ); break;
      case 5: Console->insert( QString("An unknown error occurred.") ); break;
    }

  }
  Console->insert(QString(QChar::LineSeparator));
  Console->moveCursor(QTextCursor::End);
  Console->ensureCursorVisible(); 
  Console->setColor( QColor( QString("white") ) );
			   

  app->restoreButtonsState();
}
