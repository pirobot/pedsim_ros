/**
* Copyright 2014-2016 Social Robotics Lab, University of Freiburg
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*    # Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*    # Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*    # Neither the name of the University of Freiburg nor the names of its
*       contributors may be used to endorse or promote products derived from
*       this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* \author Billy Okal <okal@cs.uni-freiburg.de>
*/

#ifndef SIMULATOR_H
#define SIMULATOR_H

// ros and big guys
#include <ros/console.h>
#include <ros/ros.h>

#include <functional>
#include <memory>
#include <tf/transform_listener.h>

#include <pedsim_msgs/AgentState.h>
#include <pedsim_msgs/AllAgentsState.h>
#include <pedsim_msgs/SocialActivities.h>
#include <pedsim_msgs/SocialActivity.h>
#include <pedsim_msgs/TrackedGroup.h>
#include <pedsim_msgs/TrackedGroups.h>
#include <spencer_tracking_msgs/TrackedPerson.h>
#include <spencer_tracking_msgs/TrackedPersons.h>

// other ROS-sy messages
#include <animated_marker_msgs/AnimatedMarker.h>
#include <animated_marker_msgs/AnimatedMarkerArray.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovariance.h>
#include <geometry_msgs/TwistWithCovariance.h>
#include <nav_msgs/GridCells.h>
#include <nav_msgs/Odometry.h>
#include <std_msgs/ColorRGBA.h>
#include <std_msgs/Header.h>
#include <std_srvs/Empty.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>

#include <pedsim_simulator/agentstatemachine.h>
#include <pedsim_simulator/agentstatemachine.h>
#include <pedsim_simulator/config.h>
#include <pedsim_simulator/element/agent.h>
#include <pedsim_simulator/element/agentgroup.h>
#include <pedsim_simulator/element/attractionarea.h>
#include <pedsim_simulator/element/waitingqueue.h>
#include <pedsim_simulator/element/waypoint.h>
#include <pedsim_simulator/orientationhandler.h>
#include <pedsim_simulator/scenarioreader.h>
#include <pedsim_simulator/scene.h>

#include <dynamic_reconfigure/server.h>
#include <pedsim_simulator/PedsimSimulatorConfig.h>

using SimConfig = pedsim_simulator::PedsimSimulatorConfig;

/// -----------------------------------------------------------------
/// \class Simulator
/// \brief Simulation wrapper
/// \details ROS interface to the scene object provided by pedsim
/// -----------------------------------------------------------------
class Simulator {
public:
    explicit Simulator(const ros::NodeHandle& node);
    virtual ~Simulator();

    bool initializeSimulation();
    void loadConfigParameters();
    void runSimulation();
    void updateAgentActivities();

    /// publishers
    void publishAgents();
    void publishData();
    void publishSocialActivities();
    void publishGroupVisuals();
    void publishObstacles();
    void publishWalls();
    void publishAttractions();
    void publishRobotPosition();

    // callbacks
    bool onPauseSimulation(std_srvs::Empty::Request& request,
        std_srvs::Empty::Response& response);
    bool onUnpauseSimulation(std_srvs::Empty::Request& request,
        std_srvs::Empty::Response& response);

    // update robot position based upon data from TF
    void updateRobotPositionFromTF();

protected:
    void reconfigureCB(SimConfig& config, uint32_t level);
    void robotPositionCallback(const nav_msgs::Odometry& odom);
    dynamic_reconfigure::Server<SimConfig> server_;

private:
    ros::NodeHandle nh_;
    bool paused_; // simulation state

    /// publishers
    // - data messages
    ros::Publisher pub_obstacles_; // grid cells
    ros::Publisher pub_all_agents_; // positions and velocities (old msg)
    ros::Publisher pub_tracked_persons_; // in spencer format
    ros::Publisher pub_tracked_groups_;
    ros::Publisher pub_social_activities_;
    // - visualization related messages (e.g. markers)
    ros::Publisher pub_attractions_;
    ros::Publisher pub_agent_visuals_;
    ros::Publisher pub_group_lines_;
    ros::Publisher pub_walls_;
    ros::Publisher pub_queues_;
    ros::Publisher pub_waypoints_;
    ros::Publisher pub_agent_arrows_;
    ros::Publisher pub_robot_position_;

    // Subscribers
    ros::Subscriber sub_robot_position_;
    nav_msgs::Odometry gazebo_robot_odom_;

    // provided services
    ros::ServiceServer srv_pause_simulation_;
    ros::ServiceServer srv_unpause_simulation_;

    // agent id <-> activity map
    std::map<int, std::string> agent_activities_;

    // pointers and additional data
    std::unique_ptr<tf::TransformListener> transform_listener_;
    std::unique_ptr<OrientationHandler> orientation_handler_;
    Agent* robot_; // robot agent
    tf::StampedTransform last_robot_pose_; // pose of robot in previous timestep
    geometry_msgs::Quaternion last_robot_orientation_;

    inline Eigen::Quaternionf computePose(Agent* a);
    inline std::string agentStateToActivity(AgentStateMachine::AgentState state);
    inline std_msgs::ColorRGBA getColor(int agent_id);
};

#endif
