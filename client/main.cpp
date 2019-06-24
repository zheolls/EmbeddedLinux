#include <qapplication.h>
#include "control.h"
int main( int argc, char ** argv )
{    
    QApplication a( argc, argv );
    Form w;
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );        
    return a.exec();
}
