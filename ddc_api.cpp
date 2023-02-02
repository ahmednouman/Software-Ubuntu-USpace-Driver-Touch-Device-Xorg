#include "video_api.h"
#include "ddc_api.h"

int findBusNumber(string id){
   string cmd, string_sub;
   string display_id = id;
   string path = "/sys/devices/pci0000:00/0000:00:02.0/drm/card0/card0-";
   string  call = "ls ";
   string filter = " | grep i2c-*";
   cmd = call + path + display_id + filter;
   string result = runUnixCommandAndCaptureOutput(cmd);
   size_t pos = result.find("-");
   string_sub = result.substr(pos+1);
   int bus = stoi(string_sub);
   return bus;
}

// int getVCP(string id, int bus, int code){
//    string cmd;
//    string  call = "sudo python3 ddc_call.py";
//    string s_bus = std::to_string(bus);
//    string s_code = std::to_string(code);
//    string sp = " ";

//    cmd = call + sp + s_bus + sp + s_code;
//    string result = runUnixCommandAndCaptureOutput(cmd);
//    cout << result << endl;
//    return 0;

// }


// int main() {

//    std::vector<std::string> id_list = getDisplaysIDs();

//    int x = findBusNumber(id_list[0]);

//    int v = getVCP(id_list[0], x, 0x10);

//    // printf("v= %d \n", v);

//    return 0;
// }