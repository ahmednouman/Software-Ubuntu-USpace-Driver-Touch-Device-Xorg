
#ifndef DDC_API
#define DDC_API
#include "video_api.h"


int findBusNumber(string id);
int getVCP(string id, int bus, int code);
bool setVCP(string id, int bus, int code, int value);

#endif 
