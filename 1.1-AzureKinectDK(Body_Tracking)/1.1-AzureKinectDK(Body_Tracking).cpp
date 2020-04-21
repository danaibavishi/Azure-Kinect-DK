#include <stdio.h>
#include <stdlib.h>

#include <k4a/k4a.h>
#include <k4abt.h>

#include <osc\OscOutboundPacketStream.h>
#include <ip\UdpSocket.h>
#include "windows.h"

#define VERIFY(result, error)                                                                            \
    if(result != K4A_RESULT_SUCCEEDED)                                                                   \
    {                                                                                                    \
        printf("%s \n - (File: %s, Function: %s, Line: %d)\n", error, __FILE__, __FUNCTION__, __LINE__); \
        exit(1);                                                                                         \
    }              

#define ADDRESS "127.0.0.1"
#define PORT 6448
#define OUTPUT_BUFFER_SIZE 1024

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;
    k4a_device_t device = NULL;
    VERIFY(k4a_device_open(0, &device), "Open K4A Device failed!");

    // Start camera. Make sure depth camera is enabled.
    k4a_device_configuration_t deviceConfig = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    deviceConfig.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    deviceConfig.color_resolution = K4A_COLOR_RESOLUTION_OFF;
    VERIFY(k4a_device_start_cameras(device, &deviceConfig), "Start K4A cameras failed!");

    k4a_calibration_t sensor_calibration;
    VERIFY(k4a_device_get_calibration(device, deviceConfig.depth_mode, deviceConfig.color_resolution, &sensor_calibration),
        "Get depth camera calibration failed!");

    k4abt_tracker_t tracker = NULL;
    k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
    VERIFY(k4abt_tracker_create(&sensor_calibration, tracker_config, &tracker), "Body tracker initialization failed!");

    int frame_count = 0;

    while (true)
    {
        UdpTransmitSocket transmitSocket(IpEndpointName(ADDRESS, PORT));
        char buffer[OUTPUT_BUFFER_SIZE];
        osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

        k4a_capture_t sensor_capture;
        k4a_wait_result_t get_capture_result = k4a_device_get_capture(device, &sensor_capture, K4A_WAIT_INFINITE);
        if (get_capture_result == K4A_WAIT_RESULT_SUCCEEDED)
        {
            frame_count++;
            k4a_wait_result_t queue_capture_result = k4abt_tracker_enqueue_capture(tracker, sensor_capture, K4A_WAIT_INFINITE);
            k4a_capture_release(sensor_capture); // Remember to release the sensor capture once you finish using it
            if (queue_capture_result == K4A_WAIT_RESULT_TIMEOUT)
            {
                // It should never hit timeout when K4A_WAIT_INFINITE is set.
                printf("Error! Add capture to tracker process queue timeout!\n");
                break;
            }
            else if (queue_capture_result == K4A_WAIT_RESULT_FAILED)
            {
                printf("Error! Add capture to tracker process queue failed!\n");
                break;
            }

            k4abt_frame_t body_frame = NULL;
            k4a_wait_result_t pop_frame_result = k4abt_tracker_pop_result(tracker, &body_frame, K4A_WAIT_INFINITE);
            if (pop_frame_result == K4A_WAIT_RESULT_SUCCEEDED)
            {
                size_t num_bodies = k4abt_frame_get_num_bodies(body_frame);
                printf("%zu bodies are detected!\n", num_bodies);
                /*
                for (size_t i = 0; i < num_bodies; i++)
                {
                    k4abt_skeleton_t skeleton;
                    k4abt_frame_get_body_skeleton(body_frame, i, &skeleton);
                    uint32_t id = k4abt_frame_get_body_id(body_frame, i);
                    
                    //for (int j = 0; j < 2; j++)
                    //{
                    //    printf("Wrist Position = %f\n", skeleton.joints[K4ABT_JOINT_WRIST_RIGHT].position.xyz.x);
                    //}
                    
                    float rwristx = skeleton.joints[K4ABT_JOINT_WRIST_RIGHT].position.xyz.x;
                    float rwristy = skeleton.joints[K4ABT_JOINT_WRIST_RIGHT].position.xyz.y;

                    float lwristx = skeleton.joints[K4ABT_JOINT_WRIST_LEFT].position.xyz.x;
                    float lwristy = skeleton.joints[K4ABT_JOINT_WRIST_LEFT].position.xyz.y;

                    printf("[%u]Right Wrist X Coordinate = %f\n",i, rwristx);
                    printf("[%u]Right Wrist Y Coordinate = %f\n",i, rwristy);

                    printf("[%u]Left Wrist X Coordinate = %f\n",i, lwristx);
                    printf("[%u]Left Wrist Y Coordinate = %f\n",i, lwristy);

                    p << osc::BeginBundleImmediate
                        << osc::BeginMessage("/wek/inputs")
                        << rwristx << rwristy << lwristx << lwristy << osc::EndMessage
                        //<< osc::BeginMessage("/wek/inputs")
                        //<< (double)0.23 << osc::EndMessage
                        << osc::EndBundle;
                    transmitSocket.Send(p.Data(), p.Size());

                }
                */
                size_t i = 0;
                k4abt_skeleton_t skeleton;
                k4abt_frame_get_body_skeleton(body_frame, i, &skeleton);
                uint32_t id = k4abt_frame_get_body_id(body_frame, i);

                float rwristx = skeleton.joints[K4ABT_JOINT_WRIST_RIGHT].position.xyz.x;
                float rwristy = skeleton.joints[K4ABT_JOINT_WRIST_RIGHT].position.xyz.y;

                float lwristx = skeleton.joints[K4ABT_JOINT_WRIST_LEFT].position.xyz.x;
                float lwristy = skeleton.joints[K4ABT_JOINT_WRIST_LEFT].position.xyz.y;

                printf("[%u]Right Wrist X Coordinate = %f\n", i, rwristx);
                printf("[%u]Right Wrist Y Coordinate = %f\n", i, rwristy);

                printf("[%u]Left Wrist X Coordinate = %f\n", i, lwristx);
                printf("[%u]Left Wrist Y Coordinate = %f\n", i, lwristy);
                
                p << osc::BeginBundleImmediate
                    << osc::BeginMessage("/wek/inputs")
                    << rwristx << rwristy << lwristx << lwristy << osc::EndMessage
                    << osc::EndBundle;

                transmitSocket.Send(p.Data(), p.Size());

                k4abt_frame_release(body_frame); // Remember to release the body frame once you finish using it
            }
            else if (pop_frame_result == K4A_WAIT_RESULT_TIMEOUT)
            {
                //  It should never hit timeout when K4A_WAIT_INFINITE is set.
                printf("Error! Pop body frame result timeout!\n");
                break;
            }
            else
            {
                printf("Pop body frame result failed!\n");
                break;
            }
        }
        else if (get_capture_result == K4A_WAIT_RESULT_TIMEOUT)
        {
            // It should never hit time out when K4A_WAIT_INFINITE is set.
            printf("Error! Get depth frame time out!\n");
            break;
        }
        else
        {
            printf("Get depth capture returned error: %d\n", get_capture_result);
            break;
        }

    }

    printf("Finished body tracking processing!\n");

    k4abt_tracker_shutdown(tracker);
    k4abt_tracker_destroy(tracker);
    k4a_device_stop_cameras(device);
    k4a_device_close(device);

    return 0;
}