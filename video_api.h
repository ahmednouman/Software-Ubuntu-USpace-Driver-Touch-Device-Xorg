
#ifndef VIDEO_API
#define VIDEO_API

#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
vector<string> splitString(const string& str);
string runUnixCommandAndCaptureOutput(string cmd);

vector<string> getDisplaysInfo();
int getNumberOfConnectedDisplays();
vector<string> getDisplaysIDs();
string getDisplayName(string id);
bool getDisplayPrimary(string id);
vector <int> getDisplayResolution(string id);
vector <int> getDisplayLocation(string id);
int getDisplayOrientation(string id);
bool setDisplayOrientation(string id, int orientation);
bool setDisplayLocation(vector<string> displayIds, int32_t* xs, int32_t* ys, uint32_t numDisplays);
#endif 

