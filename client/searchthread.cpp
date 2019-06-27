#include"searchthread.h"
#include <qapplication.h>

SeaThread::SeaThread(){
}
SeaThread::~SeaThread(){
}
void SeaThread::run(){
    while(true){
	sleep(1);
	pause.lock();
	//printf("running\n");
	emit connectToServer();
	pause.unlock();
	
    }
}
