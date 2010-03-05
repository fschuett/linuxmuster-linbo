#include "linboRegisterBoxImpl.hh"
#include "linboProgressImpl.hh"
#include "linboGUIImpl.hh"
#include <q3progressbar.h>
#include <qapplication.h>
#include <QtGui>
#include "linboPushButton.hh"
#include "linboYesNoImpl.hh"

linboRegisterBoxImpl::linboRegisterBoxImpl(  QWidget* parent ) : linboDialog()
{
  Ui_linboRegisterBox::setupUi((QDialog*)this);

  process = new Q3Process( this );

  if( parent )
    myParent = parent;

  connect(registerButton,SIGNAL(clicked()),this,SLOT(postcmd()));
  connect(cancelButton,SIGNAL(clicked()),this,SLOT(close()));

  // connect stdout and stderr to linbo console
  connect( process, SIGNAL(readyReadStdout()),
           this, SLOT(readFromStdout()) );
  connect( process, SIGNAL(readyReadStderr()),
           this, SLOT(readFromStderr()) );

  Qt::WindowFlags flags;
  flags = Qt::Dialog | Qt::WindowStaysOnTopHint;
  setWindowFlags( flags );

  QRect qRect(QApplication::desktop()->screenGeometry());
  // open in the center of our screen
  int xpos=qRect.width()/2-this->width()/2;
  int ypos=qRect.height()/2-this->height()/2;
  this->move(xpos,ypos);
  this->setFixedSize( this->width(), this->height() );
}

linboRegisterBoxImpl::~linboRegisterBoxImpl()
{
} 

void linboRegisterBoxImpl::setTextBrowser( Q3TextBrowser* newBrowser )
{
  Console = newBrowser;
}

void linboRegisterBoxImpl::setMainApp( QWidget* newMainApp ) {
  myMainApp = newMainApp;
}


void linboRegisterBoxImpl::precmd() {
  // nothing to do
}


void linboRegisterBoxImpl::postcmd() {
  this->hide();
  // here, some further checks are needed
  if( !roomName->text().isEmpty() &&
      !ipAddress->text().isEmpty() &&
      !clientGroup->text().isEmpty() &&
      !clientName->text().isEmpty() ) {

    // update our command
    // room name
    myCommand[5] = roomName->text();
    // client name
    myCommand[6] = clientName->text();
    // IP
    myCommand[7] = ipAddress->text();
    // client group
    myCommand[8] = clientGroup->text();

    linboGUIImpl* app = static_cast<linboGUIImpl*>( myMainApp );

    if( app ) {
      // do something
      linboProgressImpl *progwindow = new linboProgressImpl(0); //,"Arbeite...",0, Qt::WStyle_Tool );
      progwindow->setProcess( process );
      connect( process, SIGNAL(processExited()), progwindow, SLOT(close()));
      progwindow->show();
      progwindow->raise();
      progwindow->progressBar->setTotalSteps( 100 );

      progwindow->setActiveWindow();
      progwindow->setUpdatesEnabled( true );
      progwindow->setEnabled( true );
      
      process->clearArguments();
      process->setArguments( myCommand );

      app->disableButtons();

      process->start();

      while( process->isRunning() ) {
        for( int i = 0; i <= 100; i++ ) {
          usleep(10000);
          progwindow->progressBar->setProgress(i,100);
          progwindow->update();
          
          qApp->processEvents();
        } 
        
        if( ! process->isRunning() ) {
          progwindow->close();
        }
      }
    }
    app->restoreButtonsState();
  }
  this->close();
}

void linboRegisterBoxImpl::setCommand(const QStringList& arglist)
{
  myCommand = QStringList(arglist); // Create local copy
}

QStringList linboRegisterBoxImpl::getCommand()
{
  return QStringList(myCommand); 
}


void linboRegisterBoxImpl::readFromStdout()
{
  while( process->canReadLineStdout() )
    {
      line = process->readLineStdout();
      Console->append( line );
    } 
}

void linboRegisterBoxImpl::readFromStderr()
{
  while( process->canReadLineStderr() )
    {
      line = process->readLineStderr();
      line.prepend( "<FONT COLOR=red>" );
      line.append( "</FONT>" );
      Console->append( line );
    } 
}