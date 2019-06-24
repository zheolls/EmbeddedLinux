/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#define HOST_ADDR "192.168.43.216"
#define PORT 9413
#define STATE_OPEN 1000
#define SET_SPEED 1001
#define STATE_CHECK 1002 
#define STATE_CLOSE 1003

void Form::init(){
    socket = new QSocket( this);
    sthread=new SeaThread();
    buttonGroup1->setButton(0);
    connect(sthread,SIGNAL( connectToServer() ), this,
	    SLOT( connectToServer() ) );
    connect( socket, SIGNAL(connected()), this,
	     SLOT(socketConnected()) );
    connect( socket, SIGNAL(connectionClosed()), this,
	     SLOT(socketConnectionClosed()) );
    connect( socket, SIGNAL(readyRead()), this,
	     SLOT(socketReadyRead()) );
    connect( socket, SIGNAL(error(int)), this,
	     SLOT(socketError(int)) );
    sthread->start();
}


void Form::closeConnection()
{
    textLabel2->setText("unconnected");
    socket->close();
    if ( socket->state() == QSocket::Closing ) {
	// We have a delayed close.
	connect( socket, SIGNAL(delayedCloseFinished()),
		 SLOT(socketClosed()) );
    } else {
	// The socket is closed.
	socketClosed();
    }
}


void Form::sendToServer()
{
    // write to the server
    QString s=QString::number(slider2->value());
    if(buttonGroup1->selectedId()==1||buttonGroup1->selectedId()==-1){
	s=QString::number(SET_SPEED)+s+"\r\n";
	//textLabel2->setText(s);
	socket->writeBlock( s.latin1(), s.length() );
	socket->flush();
    }
    else  if(buttonGroup1->selectedId()==0){
	s=QString::number(STATE_CLOSE)+s+"\r\n";
	//textLabel2->setText(s);
	socket->writeBlock( s.latin1(), s.length() );
	socket->flush();	
    }
    else  textLabel2->setText("buttongGoup1 error");
}


void Form::socketReadyRead()
{
    // read from the server
    QString s=socket->readLine();
    QString sstate=s.mid(0,4);
    QString sspeed=s.mid(4);
    int nstate=sstate.toInt();
    int nspeed=sspeed.toInt();
    if(nstate==STATE_OPEN){
	textLabel2->setText("on");
    }
    else if(nstate==STATE_CLOSE)textLabel2->setText("off");
    else if(nstate==SET_SPEED){
	textLabel2->setText("on");
    }else textLabel2->setText("set error");
    textLabel4->setText(sspeed);
    slider1->setValue(nspeed);
}


void Form::socketConnected()
{
    textLabel2->setText("connected");
    QString s=QString::number(STATE_CHECK)+"0000";
    socket->writeBlock( s.latin1(), s.length() );
    socket->flush();    
}


void Form::socketConnectionClosed()
{
    textLabel2->setText("disconnected");
}


void Form::socketClosed()
{
    textLabel2->setText("socketclosed");
}

void Form::socketError( int e )
{
    QString s=QString::number(e);
    s="error:"+s;
    textLabel2->setText(s);
}

void Form::closeDialog(){
    sthread->exit();
    closeConnection();
    close();
}

void Form::connectToServer()
{
    if(socket->state()==QSocket::Idle){
	textLabel2->setText("connecting");
	QString host=HOST_ADDR;
	Q_UINT16 port=PORT;
	socket->connectToHost( host, port );
}
}
