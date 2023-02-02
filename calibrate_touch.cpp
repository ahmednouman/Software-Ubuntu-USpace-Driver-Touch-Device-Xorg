#include "calibrate_touch.h"

static vector<string> touchID_arr;
static vector<string> nodes_arr;
static vector<string> usb_portID_arr;
static vector<string> displayID_arr;

static bool readConf = true; 

bool isConf(){
   FILE *file;
   if (file = fopen("./espressoSetup.conf", "r")) {
      fclose(file);
      cout << "Config file exists" << endl;
      return true;
   } else {
      cout << "Config file doesn't exist" << endl;
      return false;
   }
}

vector<string> parseConf(){
    vector<string> confInfo;
    ifstream confFile; 
    confFile.open("./espressoSetup.conf");
    string line;
    if ( confFile.is_open() ) {
        while ( confFile ) {
            getline (confFile, line);
            //std::cout << line << '\n';
            confInfo.push_back(line);
        }
    }
    return confInfo;
}

vector<string> findTouchDeviceIDs(){
   vector<string>  list;
   size_t pos = 0;
   string touch_id, cmd, call, filter_VID, filter_PID, VID, PID;
   cmd = "xinput | sed '/TouchScreen/s/.*id=\\([0-9]*\\).*/\\1/;t;d'";

   string result = runUnixCommandAndCaptureOutput(cmd); 
 
   vector<string> tokens = splitString(result);
   for (int i=0; i < tokens.size(); i++) {
      if((tokens[i].empty())){
         tokens.erase(next(tokens.begin(), i));
      }      
   }
    // for (auto const &token: tokens) {
    //   std::cout << "tok: "<< tokens.size() << ":" <<token << std::endl;        
    // }
    for (int i=0; i < tokens.size(); i++) {
        call = "xinput list-props ";
        touch_id = tokens[i];
        filter_VID = " | grep 'Device Product' |  awk '{gsub(\",\",\"\", $5); print $5}'";
        filter_PID = " | grep 'Device Product' |  awk '{gsub(\",\",\"\", $6); print $6}'";

        cmd = call + touch_id + filter_VID;
        VID = runUnixCommandAndCaptureOutput(cmd); 
        cmd = call + touch_id + filter_PID;
        PID = runUnixCommandAndCaptureOutput(cmd); 
        if((VID == "1386\n") && (PID == "33169\n")){
            list.push_back(touch_id);    
        }
 
    }
    touchID_arr = list;
    return list;
}

string findSingleDeviceNode(string TouchID){
    string node;
    string call, filter, cmd, device_node;

    call = "xinput list-props ";
    filter = " | grep 'Device Node' |  awk '{gsub(\",\",\"\", $4); print $4}'";
        cmd = call + TouchID + filter;
        device_node = runUnixCommandAndCaptureOutput(cmd);
        // cout << device_node << endl;
        vector<string> tokens = splitString(device_node);
        for (int i=0; i < tokens.size(); i++) {
            if((tokens[i].empty())){
                tokens.erase(next(tokens.begin(), i));
            }      
        }
        //for (auto const &token: tokens) {
            //cout << "tok: "<< tokens.size() << ":" <<token << endl;       
            node = tokens[0]; 
        //}


    return node;
}

vector<string> findDeviceNodes(vector<string> TouchIDs){
    vector<string> nodes;
    string call, filter, cmd, device_node;

    call = "xinput list-props ";
    filter = " | grep 'Device Node' |  awk '{gsub(\",\",\"\", $4); print $4}'";
    for (int i=0; i < TouchIDs.size(); i++){
        cmd = call + TouchIDs[i] + filter;
        device_node = runUnixCommandAndCaptureOutput(cmd);
        // cout << device_node << endl;
        vector<string> tokens = splitString(device_node);
        for (int i=0; i < tokens.size(); i++) {
            if((tokens[i].empty())){
                tokens.erase(next(tokens.begin(), i));
            }      
        }
        for (auto const &token: tokens) {
            //cout << "tok: "<< tokens.size() << ":" <<token << endl;       
            nodes.push_back(token); 
        }
    }
    nodes_arr = nodes;
    return nodes;
}

string findSingleDeviceUSBPort(string node){
    string USBPort;
    string call, filter, cmd, device_port;

    call = "udevadm info -a ";
    filter = "  | grep 'KERNELS' | awk '/KERNELS/{i++}i==3'";
    cmd = call + node + filter;
    device_port = runUnixCommandAndCaptureOutput(cmd);
    vector<string> tokens = splitString(device_port);
    for (int i=0; i < tokens.size(); i++) {
        if((tokens[i].empty())){
            tokens.erase(next(tokens.begin(), i));
        }      
    }

    string m = tokens[0];
    m.erase(remove(m.begin(), m.end(), ' '), m.end()); 
    m.erase(remove(m.begin(), m.end(), '\n'), m.end());      

    USBPort = m;
    return USBPort;
}

vector<string> findDeviceUSBPort(vector<string> nodes){
    vector<string> USBPorts;
    string call, filter, cmd, device_port;

    call = "udevadm info -a ";
    filter = "  | grep 'KERNELS' | awk '/KERNELS/{i++}i==3'";
    for (int i=0; i < nodes.size(); i++){
        cmd = call + nodes[i] + filter;
        device_port = runUnixCommandAndCaptureOutput(cmd);
        vector<string> tokens = splitString(device_port);
        for (int i=0; i < tokens.size(); i++) {
            if((tokens[i].empty())){
                tokens.erase(next(tokens.begin(), i));
            }      
        }
        for (auto const &token: tokens) {
            string m = token;
            m.erase(remove(m.begin(), m.end(), ' '), m.end());     
            cout << m << endl; 
            USBPorts.push_back(m); 
        }
    }
    usb_portID_arr = USBPorts;
    return USBPorts;
}

vector<string> numberOfUniqueUSB(vector<string> ports){
    vector<string>  unique_usb_ports;
    string uniquePort;
    for(int i=0; i < ports.size(); i++){
       uniquePort = ports[i];
       if(i == 0){
        unique_usb_ports.push_back(uniquePort);
        continue;
       }
        if (find(unique_usb_ports.begin(), unique_usb_ports.end(), uniquePort) != unique_usb_ports.end())
        {
        // Element in vector.
        }else{
            unique_usb_ports.push_back(uniquePort);
        }
    }
    return unique_usb_ports;
}

bool mapTouchToOutput(vector<string>  id){
    int num;    
    vector<string> id_copy = id;

    vector<string> touch_ids = findTouchDeviceIDs();
    vector<string> device_nodes = findDeviceNodes(touch_ids);
    vector<string> device_usb_ports = findDeviceUSBPort(device_nodes);
    vector<string> phy_usb = numberOfUniqueUSB(device_usb_ports);
    int num_of_py_usb = phy_usb.size();

    int displays_num = touch_ids.size() / 3;
    // int id_array[touch_ids.size()] =  {};
    // for(int i=0; i < touch_ids.size(); i++){
    //     num = stoi(touch_ids[i]);
    //     id_array[i] = num;
    // }
    // sort(id_array, id_array + touch_ids.size());
    string cmd;
    string  call = "xinput map-to-output ";
    string sp = " ";
    if(touch_ids.size() == 3 && id.size() == displays_num){
        cout << "Single espresso Display Detected" << endl;
        string display_id = id[0];
        for (int i=0; i < touch_ids.size(); i++) {
            cout << "Calibrated touch id = " << touch_ids[i] << endl;
            cmd = call + touch_ids[i] + sp + display_id;
            cout << cmd << endl;
            string result = runUnixCommandAndCaptureOutput(cmd);
        }       

    }
    else{
        bool ConfFileExist = isConf();
        if(ConfFileExist && readConf){
            string display_id;
            cout << "Multi espresso Displays Detected - Reading Config File Only" << endl;

            vector<string> configData = parseConf();
            string line_0 = configData[0];
            string display_num_par = line_0.substr(line_0.find("=")+1, line_0.size());
            int d_num_p = stoi(display_num_par);
            if(displays_num == id.size() && d_num_p == displays_num){
                string touch_id, port_id; 
                string display_id;
                cout << displays_num << " Displays Detected" << endl;
                
                vector<string> unique_lines;
                vector<string> displaysID;
                vector<string> physicalPorts;
                for (int i=0; i < configData.size(); i++) {
                    if ( i != 0){
                        if(i == 1){
                            unique_lines.push_back(configData[i]);
                            display_id = configData[i].substr(0, configData[i].find(";"));
                            port_id = configData[i].substr(configData[i].find(";")+1, configData[i].size());
                            displaysID.push_back(display_id);
                            physicalPorts.push_back(port_id);
                        }
                        if(find(unique_lines.begin(), unique_lines.end(), configData[i]) != unique_lines.end()){

                        }else{
                            unique_lines.push_back(configData[i]);
                            display_id = configData[i].substr(0, configData[i].find(";"));
                            port_id = configData[i].substr(configData[i].find(";")+1, configData[i].size());  
                            displaysID.push_back(display_id);
                            physicalPorts.push_back(port_id);                          
                        }

                    }
 
                 }
                 
                 string display_port, device_event, usb_port; 
                 
                 for(int i=0; i < touch_ids.size(); i++){

                    device_event = findSingleDeviceNode(touch_ids[i]);
                    usb_port = findSingleDeviceUSBPort(device_event);
                    for(int j=0; j < physicalPorts.size(); j++){
                       // cout << usb_port << endl;
                        if(usb_port == physicalPorts[j]){
                            cout << usb_port << endl;
                            cout << "j=" << j << " Calibrated touch id = " << touch_ids[i] << " Display " << displaysID[j] << endl;
                            cmd = call + touch_ids[i] + sp + displaysID[j];
                            string result = runUnixCommandAndCaptureOutput(cmd);  
                        }
                    }
                 }
            }

        }else{
            cout << "Multi espresso Displays Detected - Calibrating & Writing to Config File" << endl;
            cout << displays_num << " Displays Detected" << endl;
            string display_id;
            if(displays_num == id.size()){
            
            string endCalibration;
            int iteration = 0, fact=1;
            for(int i=1;i<=num_of_py_usb;i++){    
                fact=fact*i;    
            }
            cout << "Number of Different Possible Setups = " << fact << endl;
            vector<int> setupArr;

            for(int iteration=0; iteration < fact; iteration++){
                ofstream confHandler;
                confHandler.open("./espressoSetup.conf");
                confHandler << "displayNum=" << displays_num << endl;

                if(iteration == 1 && id.size() == 2){
                    id = id_copy;
                    swap(id[0], id[1]);
                }else if(iteration == 1 && id.size() == 3){
                    id = id_copy;
                    swap(id[1], id[2]);
                }else if(iteration == 2){
                    id = id_copy;
                    swap(id[1], id[2]);
                    swap(id[0], id[1]);
                }else if(iteration == 3){
                    id = id_copy;
                    swap(id[0], id[1]);
                }else if(iteration == 4){
                    id = id_copy;
                    swap(id[0], id[1]);
                    swap(id[1], id[2]);
                }else if(iteration == 5){
                    id = id_copy;
                    swap(id[0], id[2]);
                }

                    for (int i=0; i < phy_usb.size(); i++) {
                        string usb_iter = phy_usb[i];
                        display_id = id[i];
                        
                        for(int j=0; j < device_usb_ports.size(); j++){
                            if(usb_iter == device_usb_ports[j]){
                                confHandler << display_id << ";" << phy_usb[i] << endl;
                                cout << "j="<< j << " Calibrated touch id = " << touch_ids[j] << " Display " << display_id << endl;
                                cmd = call + touch_ids[j] + sp + display_id;   
                                string result = runUnixCommandAndCaptureOutput(cmd);                                 
                            }
                        }
                        
                    }
                    confHandler.close();
                    cout << "Touch the espresso displays to verify calibration" << endl;
                    cout << "finished calibration? press (y) then enter for yes, or (n) for no to try another setup" << endl;
                    cin >> endCalibration;
                    transform(endCalibration.begin(), endCalibration.end(), endCalibration.begin(), ::tolower);      
                    if(endCalibration == "y"){
                        break;
                    }             
                }
            }            
        }
    }
    return true;
}


int main(int argc, char** argv) {
    string arguments;
    if(argc > 2){
        cout << "Invalid Arguments!" << endl;
        return 0;
    }
    for (int i = 0; i < argc; ++i){
        arguments = argv[i];
        if(arguments == "-calibrate"){
            cout << "Calibration Mode" << endl;
            readConf = false;
        }
    }
    vector<string> id_list = getDisplaysIDs();
    vector<string>  espresso_diplays;

    for(int i=0; i < id_list.size(); i++){
     string s = getDisplayName(id_list[i]);
//     cout << id_list[i] << " ---- " << s << endl;
     if ((s == "eD13T(2022)") || (s == "eD15T(2022)")){
        espresso_diplays.push_back(id_list[i]);
         cout << " espresso id = " << id_list[i] << endl;
     }
    }
    displayID_arr = espresso_diplays;
    mapTouchToOutput(espresso_diplays);
   return 0;
}