/*//NBL: ROS Compliance
#include "ros/ros.h"
#include "std_msgs/String.h"*/

#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.
#include <iostream>   // std::cout
#include <string>     // std::string, std::to_string
#include <chrono>
#include <ctime>

using namespace std;
using namespace std::chrono;

system_clock::time_point now;
time_t tt;
tm utc_tm;
tm local_tm;
tm last_tm;
int timestamp_modifier_i;
char timestamp_modifier_c;

/*
//NBL: ROS Compliance
std_msgs::String record;*/
string recordData = "1";
bool camerasInitialized;

unsigned int imageCnt;
/*
//NBL: ROS Compliance
void chatterCallback(const std_msgs::String::ConstPtr& msg)
{
    record = *msg;
 
    //ROS_INFO("I heard: [%s]", msg->data.c_str());
}
*/
/*
int AcquireImage()
{
}*/

int main()
{
    // Variables
    XCHANDLE handle = 0; // Handle to the camera
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.
    word *frameBuffer = 0; // 16-bit buffer to store the capture frame.
    dword frameSize = 0; // The size in bytes of the raw image.

    // Open a connection to the first detected camera by using connection string cam://0
    printf("Opening connection to cam://0\n");
    handle = XC_OpenCamera("cam://0");

    // When the connection is initialised, ...
    if(XC_IsInitialised(handle))
    {      
        // ... start capturing
        printf("Start capturing.\n");
        if ((errorCode = XC_StartCapture(handle)) != I_OK)
        {
            printf("Could not start capturing, errorCode: %lu\n", errorCode);
        }
        else if (XC_IsCapturing(handle)) // When the camera is capturing ...
        {
            // Determine native framesize.
            frameSize = XC_GetFrameSize(handle);

            // Initialize the 16-bit buffer.
            frameBuffer = new word[frameSize / 2];

            if(recordData == "1")
            {
                // ... grab a frame from the camera.
                printf("Grabbing a frame.\n");
                if ((errorCode = XC_GetFrame(handle, FT_NATIVE, XGF_Blocking, frameBuffer, frameSize)) != I_OK)
                {
                    printf("Problem while fetching frame, errorCode %lu", errorCode);
                }else{
                    //NBL: Grab time and date information for creating a timestamp for the image filenames.        
                    now = system_clock::now();
                    tt = system_clock::to_time_t(now);
                    utc_tm = *gmtime(&tt);
                    local_tm = *localtime(&tt);
                    string dateTime = to_string(utc_tm.tm_year + 1900) + '-' + to_string(utc_tm.tm_mon + 1) + '-' + to_string(utc_tm.tm_mday) + '_' 
                                    + to_string(utc_tm.tm_hour) + '-' + to_string(utc_tm.tm_min) + '-' + to_string(utc_tm.tm_sec);
                 
                    if((local_tm.tm_hour == last_tm.tm_hour) &&
                       (local_tm.tm_min == last_tm.tm_min) &&
                       (local_tm.tm_sec == last_tm.tm_sec))
                    {
                        timestamp_modifier_c = static_cast<char>(timestamp_modifier_i);
                        dateTime += timestamp_modifier_c;
                        timestamp_modifier_i ++;
                    }
                    else
                    {
                        timestamp_modifier_i = 97;
                    }
                    last_tm = local_tm;
                    
                    string imageDirectory = "/home/user1/Data/"; 
                    string imageFilename = imageDirectory + "Xenic-Gobi" + "_" + dateTime + ".png";
                    
                    printf("Attempting to save frame....\n");
                    if((errorCode = XC_SaveData(handle, imageFilename.c_str(), XSD_SaveThermalInfo | XSD_Force16)) != I_OK)
                    {
                        printf("Problem saving data, errorCode %lu\n", errorCode);
                    }else{
                        printf("Saved successfully!\n");
                    }
                }
            }
        }
    }
    else
    {
        printf("Initialization failed\n");
    }

    // Cleanup.

    // When the camera is still capturing, ...
    if(XC_IsCapturing(handle))
    {
        // ... stop capturing.
        printf("Stop capturing.\n");
        if ((errorCode = XC_StopCapture(handle)) != I_OK)
        {
            printf("Could not stop capturing, errorCode: %lu\n", errorCode);
        }
    }

    // When the handle to the camera is still initialised ...
    if (XC_IsInitialised(handle))
    {
        printf("Closing connection to camera.\n");
        XC_CloseCamera(handle);
    }

    printf("Clearing buffers.\n");
    if (frameBuffer != 0)
    {
        delete [] frameBuffer;
        frameBuffer = 0;
    }

    return 0;
}
