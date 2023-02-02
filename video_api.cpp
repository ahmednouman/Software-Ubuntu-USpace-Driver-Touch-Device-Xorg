#include "video_api.h"

vector<string> splitString(const string& str)
{
    vector<std::string> tokens;
 
    string::size_type pos = 0;
    string::size_type prev = 0;
    while ((pos = str.find('\n', prev)) != string::npos) {
        tokens.push_back(str.substr(prev, pos - prev));
        prev = pos + 1;
    }
    tokens.push_back(str.substr(prev));
 
    return tokens;
}

string runUnixCommandAndCaptureOutput(string cmd) {
   char buffer[128];
   string result = "";
   FILE* pipe = popen(cmd.c_str(), "r");
   if (!pipe) throw std::runtime_error("popen() failed!");
   try {
      while (!feof(pipe)) {
         if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
      }
   } catch (...) {
      pclose(pipe);
      throw;
   }
   pclose(pipe);
   return result;
}


   vector<string> getDisplaysInfo(){
   string cmd = "xrandr | grep -w connected";
   string result = runUnixCommandAndCaptureOutput(cmd);
   vector<string> tokens = splitString(result);
   vector <int> index;
   for (int i=0; i < tokens.size(); i++) {
      if((tokens[i].empty())){
         index.push_back(i);
         tokens.erase(next(tokens.begin(), i));
      }      
   }
   return tokens;
 }

int getNumberOfConnectedDisplays(){
   string cmd = "xrandr | grep -w connected";
   string result = runUnixCommandAndCaptureOutput(cmd);
   vector<string> tokens = splitString(result);
   vector <int> index;
   for (int i=0; i < tokens.size(); i++) {
      if((tokens[i].empty())){
         index.push_back(i);
         tokens.erase(next(tokens.begin(), i));
      }      
   }

   // for (auto const &token: tokens) {
   //    std::cout << token << std::endl;        
   //  }
   return tokens.size();
}

vector<string> getDisplaysIDs(){
   string cmd = "xrandr | grep -w connected | awk -F'[ '\\+']' '{print $1}'";
   string result = runUnixCommandAndCaptureOutput(cmd);
   vector<std::string> tokens = splitString(result);
   vector <int> index;
   for (int i=0; i < tokens.size(); i++) {
      if((tokens[i].empty())){
         index.push_back(i);
         tokens.erase(next(tokens.begin(), i));
      }      
   }

   return tokens;
}

string getDisplayName(string id){
   string cmd;
   string display_id = id;
   string path = "/sys/devices/pci0000:00/0000:00:02.0/drm/card0/card0-";
   string  call = "parse-edid  < ";
   string filter = "/edid | grep 'ModelName' | awk -F'[ '\\+']' '{gsub(/\"/, \"\", $2);print $2}'";
   cmd = call + path + display_id + filter;
   string result = runUnixCommandAndCaptureOutput(cmd);
   vector<string> tokens = splitString(result);
   for (int i=0; i < tokens.size(); i++) {
      if((tokens[i].empty()) || (tokens[i].find("Checksum") != string::npos)){

         tokens.erase(next(tokens.begin(), i));
      }      
   }

   if (tokens.size() == 1){
      return tokens[0];
   }else{
      return "ERROR";
   }
}

bool getDisplayPrimary(string id){
   string cmd;
   string display_id = id;   
   vector<std::string> displays_list = getDisplaysInfo();
   for (int i=0; i < displays_list.size(); i++) {
      if((displays_list[i].find(display_id) != string::npos) && (displays_list[i].find("primary") != string::npos)){
         return true;
      }      
   }
   return false;
}

vector <int> getDisplayResolution(string id){
   vector <int>  value;
   bool check_primary = getDisplayPrimary(id);
   string cmd, string_sub, filter;
   string call = "xrandr | grep -w ";
   string display_id = id;    
   if(check_primary){
      filter = " | awk -F'[ \\+]' '{print $4}'";
   }else{
      filter = " | awk -F'[ \\+]' '{print $3}'";
   }
   cmd = call + display_id + filter;
   string result = runUnixCommandAndCaptureOutput(cmd);

   size_t pos = result.find("x");
   string_sub = result.substr(0, pos);

   int width = stoi(string_sub);
   string_sub = result.substr(pos + 1);
   int height = stoi(string_sub);

   value.push_back(width);
   value.push_back(height);

   return value;
 }

 vector <int> getDisplayLocation(string id){
   vector <int>  value;
   bool check_primary = getDisplayPrimary(id);
   string cmd_x, cmd_y, filter_x, filter_y;
   string call = "xrandr | grep -w ";
   string display_id = id;  
   if(check_primary){
      filter_x = " | awk -F'[ \\+]' '{print $5}'";
      filter_y = " | awk -F'[ \\+]' '{print $6}'";
   }else{
      filter_x = " | awk -F'[ \\+]' '{print $4}'";
      filter_y = " | awk -F'[ \\+]' '{print $5}'";
   }
   cmd_x = call + display_id + filter_x;
   cmd_y = call + display_id + filter_y;
   string result_x = runUnixCommandAndCaptureOutput(cmd_x);
   string result_y = runUnixCommandAndCaptureOutput(cmd_y);
   int x = stoi(result_x);
   int y = stoi(result_y);
   value.push_back(x);
   value.push_back(y);
   return value;
 }

int getDisplayOrientation(string id){
   string  cmd, filter;
   int angle;
   string call = "xrandr --query --verbose | grep ";
   string display_id = id;
   bool check_primary = getDisplayPrimary(id);
   if(check_primary){
      filter = " | awk -F'[ \\+]' '{print $8}'";
   }else{
      filter = " | awk -F'[ \\+]' '{print $7}'";
   }
   cmd = call + display_id + filter;
   string result = runUnixCommandAndCaptureOutput(cmd);
   vector<string> tokens = splitString(result);     

   if(tokens[0] == "normal"){
         angle = 0;
   }
   else if(tokens[0] == "left"){
         angle = 90;
   }
   else if(tokens[0] == "inverted"){
         angle = 180;
   }
   else if(tokens[0] == "right"){
         angle = 270;
   }
   return angle;
 }

 bool getMirrorState(string id){
   int display_width, display_height, display_x, display_y, width, height, x, y;
   vector <int> display_resolution = getDisplayResolution(id);
   vector <int> display_location = getDisplayLocation(id);
   display_width = display_resolution[0];
   display_height = display_resolution[1];
   display_x = display_location[0];
   display_y = display_location[1];
   vector<string> id_list = getDisplaysIDs();
   string other_id;
   for(int i=0; i < id_list.size(); i++){
      other_id = id_list[i];
      if (other_id != id){
            vector <int> resolution = getDisplayResolution(other_id);
            vector <int> location = getDisplayLocation(other_id);
            width = resolution[0];
            height = resolution[1];
            x = location[0];
            y = location[1];

            if( (display_width == width ) && (display_height == height) && (display_x == x) && (display_y == y) ){
               return true;
            }
      }
   }
   return false;

 }

 bool setDisplayOrientation(string id, int orientation){
   string angle, cmd;
   switch (orientation){
      case 0:
         angle = "normal";
         break;
      case 90:
         angle = "left";
         break;
      case 180:
         angle = "inverted";
         break;
      case 270:
         angle = "right";
         break;
   }

   string call = "xrandr --output ";
   string display_id = id;
   string filter = " --rotate ";
   cmd = call + display_id + filter + angle;
   string result = runUnixCommandAndCaptureOutput(cmd);
   return true;
 }

bool setDisplayLocation(std::vector<std::string> displayIds, int32_t* xs, int32_t* ys, uint32_t numDisplays){
   string cmd, filter, x, y;
   string call = "xrandr"; 
   string out = " --output ";
   string param = "--pos";
   string sep = "x";
   string space = " ";
   cmd = call;
   for (int i=0; i < displayIds.size(); i++){
      x = to_string(xs[i]);
      y = to_string(ys[i]);
      filter = out + displayIds[i] + space + param + space + x + sep + y;
      cmd = cmd + filter;
   }
   string result = runUnixCommandAndCaptureOutput(cmd);
   return true;
}

// int main() {
//    int displays_num = getNumberOfConnectedDisplays();
//    // cout << "num of displays = " << displays_num << endl;
//    std::vector<std::string> id_list = getDisplaysIDs();

//    bool x = getMirrorState(id_list[0]);

//    cout << x << endl;

//    // std::vector <int> one = getDisplayLocation(id_list[0]);
//    // std::vector <int> two = getDisplayLocation(id_list[1]);

//    // std::vector <int> res_one = getDisplayResolution(id_list[0]);
//    // std::vector <int> res_two = getDisplayResolution(id_list[1]);

//    // cout << id_list[0] << " // " << one[0] << "," << one[1] << " // " << res_one[0] << "," << res_one[1] << endl;
//    // cout << id_list[1] << " // " << two[0] << "," << two[1] << " // " << res_two[0] << "," << res_two[1] << endl;

 
//    return 0;
// }