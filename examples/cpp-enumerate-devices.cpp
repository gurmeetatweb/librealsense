// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#include <librealsense/rs.hpp>
#include <iostream>
#include <iomanip>

#include "tclap/CmdLine.h"

using namespace std;
using namespace TCLAP;

int main(int argc, char** argv) try
{
    CmdLine cmd("librealsense cpp-enumerate-devices example tool", ' ', RS_API_VERSION_STR);

    SwitchArg compact_view_arg("s", "short", "Provide short summary of the devices");
    SwitchArg show_options("o", "option", "Show all supported options per subdevice");
    SwitchArg show_modes("m", "modes", "Show all supported stream modes per subdevice");
    cmd.add(compact_view_arg);
    cmd.add(show_options);
    cmd.add(show_modes);

    cmd.parse(argc, argv);

    rs::log_to_console(RS_LOG_SEVERITY_WARN);

    // Obtain a list of devices currently present on the system
    rs::context ctx;
    auto devices = ctx.query_devices();
    int device_count = devices.size();
    if (!device_count)
    {
        printf("No device detected. Is it plugged in?\n");
        return EXIT_SUCCESS;
    }

    if (compact_view_arg.getValue())
    {
        cout << left << setw(30) << "Device Name"
                     << setw(20) << "Serial Number"
                     << setw(20) << "Firmware Version"
                     << endl;

        for (auto i = 0; i < device_count; ++i)
        {
            auto dev = devices[i];

            cout << left << setw(30) << dev.get_camera_info(RS_CAMERA_INFO_DEVICE_NAME) 
                         << setw(20) << dev.get_camera_info(RS_CAMERA_INFO_DEVICE_SERIAL_NUMBER)
                         << setw(20) << dev.get_camera_info(RS_CAMERA_INFO_CAMERA_FIRMWARE_VERSION)
                         << endl;
        }
        
        return EXIT_SUCCESS;
    }

    for (auto i = 0; i < device_count; ++i)
    {
        auto dev = devices[i];

        // Show which options are supported by this device
        cout << " Camera info: \n";
        for (auto j = 0; j < RS_CAMERA_INFO_COUNT; ++j)
        {
            auto param = static_cast<rs_camera_info>(j);
            if (dev.supports(param))
                cout << "    " << left << setw(20) << rs_camera_info_to_string(rs_camera_info(param)) 
                     << ": \t" << dev.get_camera_info(param) << endl;
        }

        cout << endl;

        for (auto s = 0; s < RS_SUBDEVICE_COUNT; s++)
        {
            auto subdevice = static_cast<rs_subdevice>(s);

            if (!dev.supports(subdevice)) continue;

            cout << " Subdevice " << rs_subdevice_to_string(subdevice) << endl;
            // Show which options are supported by this device

            if (show_options.getValue())
            {
                cout << setw(55) << " Supported options:" << setw(10) << "min" << setw(10)
                    << " max" << setw(6) << " step" << setw(10) << " default" << endl;
                for (auto j = 0; j < RS_OPTION_COUNT; ++j)
                {
                    auto opt = static_cast<rs_option>(j);
                    if (dev.get_subdevice(subdevice).supports(opt))
                    {
                        auto range = dev.get_subdevice(subdevice).get_option_range(opt);
                        cout << "    " << left << setw(50) << opt << " : "
                            << setw(5) << range.min << "... " << setw(12) << range.max
                            << setw(6) << range.step << setw(10) << range.def << "\n";
                    }
                }

                cout << endl;
            }

            if (show_modes.getValue())
            {
                cout << setw(55) << " Supported modes:" << setw(10) << "stream" << setw(10)
                    << " resolution" << setw(6) << " fps" << setw(10) << " format" << endl;
                // Show which streams are supported by this device
                for (auto&& profile : dev.get_subdevice(subdevice).get_stream_profiles())
                {
                    cout << "    " << profile.stream << "\t  " << profile.width << "\tx "
                        << profile.height << "\t@ " << profile.fps << "Hz\t" << profile.format << endl;

                    // Show horizontal and vertical field of view, in degrees
                    //std::cout << "\t" << std::setprecision(3) << intrin.hfov() << " x " << intrin.vfov() << " degrees\n";
                }

                cout << endl;
            }
        }

        cout << endl;
    }

    return EXIT_SUCCESS;
}
catch (const rs::error & e)
{
    cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << endl;
    return EXIT_FAILURE;
}