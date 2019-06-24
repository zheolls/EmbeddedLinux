#include"searchthread.h"
#include <qapplication.h>
 
SeaThread::SeaThread(){
}
SeaThread::~SeaThread(){
}
void SeaThread::run(){
	while(true){
	   emit connectToServer();
	    sleep(1);
	}
 }
