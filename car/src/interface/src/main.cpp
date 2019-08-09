
#include <cstdlib>
#include <string>

#include "ros/ros.h"
#include "interface.h"
#include "uds.h"

using namespace std;
using namespace roscar::car::interface;
using namespace roscar::car::roscar_common;

const char *UDS_URI = "/tmp/.roscar.car.interface.soc";

int main(int argc, char **argv)
{
    ros::init(argc, argv, "interface");
    if (argc != 1)
    {
        ROS_INFO("usage: interface");
        return 1;
    }

    ros::NodeHandle nh;
    ROS_INFO("ROS Car Interface Ready.");

    // init interface
    ROS_DEBUG("Debug: init interface.");
    Interface::init(nh);

    // init uds
    ROS_DEBUG("Debug: init uds.");
    UDS uds(Interface::onSignaling);
    uds.start(UDS_URI);
    uds.join();

    return 0;
}
