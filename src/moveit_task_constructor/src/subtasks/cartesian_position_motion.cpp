#include <moveit_task_constructor/subtasks/cartesian_position_motion.h>

#include <rclcpp/rclcpp.hpp>
#include <moveit_msgs/msg/display_trajectory.hpp>

#include <moveit/planning_scene/planning_scene.h>
#include <moveit/robot_state/conversions.h>
#include <moveit/robot_state/robot_state.h>
#include <moveit/move_group_interface/move_group_interface.h>

// #include <eigen_conversions/eigen_msg.h>

#include <Eigen/Geometry>

moveit::task_constructor::subtasks::CartesianPositionMotion::CartesianPositionMotion(std::string name)
: moveit::task_constructor::SubTask::SubTask(name)
{
	std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared(""); // ros::NodeHandle nh;
	pub=node->create_publisher<moveit_msgs::msg::DisplayTrajectory>("move_group/display_planned_path", 50); // pub= nh.advertise<moveit_msgs::DisplayTrajectory>("move_group/display_planned_path", 50);
	rclcpp::WallRate(1.0).sleep(); // ros::Duration(1.0).sleep();
}

void
moveit::task_constructor::subtasks::CartesianPositionMotion::setGroup(std::string group){
	group_= group;
}

void
moveit::task_constructor::subtasks::CartesianPositionMotion::setLink(std::string link){
	link_= link;
}

void
moveit::task_constructor::subtasks::CartesianPositionMotion::setMinDistance(double distance){
	min_distance_= distance;
}

void
moveit::task_constructor::subtasks::CartesianPositionMotion::setMaxDistance(double distance){
	max_distance_= distance;
}

void
moveit::task_constructor::subtasks::CartesianPositionMotion::setMinMaxDistance(double min_distance, double max_distance){
	setMinDistance(min_distance);
	setMaxDistance(max_distance);
}

void
moveit::task_constructor::subtasks::CartesianPositionMotion::towards(geometry_msgs::msg::PointStamped towards){
	mode_= moveit::task_constructor::subtasks::CartesianPositionMotion::MODE_TOWARDS;
	towards_= towards;
}

void
moveit::task_constructor::subtasks::CartesianPositionMotion::along(geometry_msgs::msg::Vector3Stamped along){
	mode_= moveit::task_constructor::subtasks::CartesianPositionMotion::MODE_ALONG;
	along_= along;
}

bool
moveit::task_constructor::subtasks::CartesianPositionMotion::canCompute(){
	return hasEnding(); //hasBeginning() || hasEnding();
}

namespace {
	bool isValid(planning_scene::PlanningSceneConstPtr scene,
	        robot_state::RobotState* state,
	        const robot_model::JointModelGroup* jmg,
	        const double* joint_positions){
		state->setJointGroupPositions(jmg, joint_positions);
		state->update();
		return !scene->isStateColliding(*state, jmg->getName());
	}
}

bool
moveit::task_constructor::subtasks::CartesianPositionMotion::compute(){
	if( hasEnding() )
		return _computeFromEnding();
//	else if( hasBeginning() )
//		return _computeFromBeginning();
}

bool
moveit::task_constructor::subtasks::CartesianPositionMotion::_computeFromEnding(){
	assert( hasEnding() );

	moveit::task_constructor::InterfaceState& ending= fetchStateEnding();
	planning_scene::PlanningScenePtr result_scene = ending.state->diff();
	robot_state::RobotState &robot_state = result_scene->getCurrentStateNonConst();

	const moveit::core::JointModelGroup* jmg= robot_state.getJointModelGroup(group_);
	const moveit::core::LinkModel* link_model= robot_state.getRobotModel()->getLinkModel(link_);

	const moveit::core::GroupStateValidityCallbackFn is_valid=
		std::bind(
			&isValid,
			result_scene,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3);
	
	Eigen::Vector3d direction;

	switch(mode_){
	case(moveit::task_constructor::subtasks::CartesianPositionMotion::MODE_TOWARDS):
		{
		const Eigen::Affine3d& link_pose= robot_state.getGlobalLinkTransform(link_);
		direction= link_pose.linear()*Eigen::Vector3d(-1,0,0);
		}
		break;
	case(moveit::task_constructor::subtasks::CartesianPositionMotion::MODE_ALONG):
		break;
	default:
		throw std::runtime_error("position motion has neither a goal nor a direction");
	}

	auto traj= std::make_shared<robot_trajectory::RobotTrajectory>(robot_state.getRobotModel(), jmg);
	std::vector<moveit::core::RobotStatePtr> trajectory_steps;

	double achieved_distance= robot_state.computeCartesianPath(
		jmg,
		trajectory_steps,
		link_model,
		direction,
		true, /* global frame */
		max_distance_, /* distance */
		.005, /* cartesian step size */
		1.5);//, /* jump threshold */
		//is_valid);

	std::cout << "achieved " << achieved_distance << " of cartesian motion" << std::endl;

	bool succeeded= achieved_distance >= min_distance_;

	if(succeeded){
		for( auto& tp : trajectory_steps )
			traj->addPrefixWayPoint(tp, 0.0);

		moveit::core::RobotStatePtr result_state= trajectory_steps.back();
		robot_state= *result_state;

		moveit::task_constructor::SubTrajectory &trajectory = addTrajectory(traj);
		sendBackward(trajectory, result_scene);

		_publishTrajectory(*traj, *result_state);

		return true;
	}

	return false;
}


void
moveit::task_constructor::subtasks::CartesianPositionMotion::_publishTrajectory(robot_trajectory::RobotTrajectory& trajectory, const moveit::core::RobotState& start){ //changed removed const
			moveit_msgs::msg::DisplayTrajectory dt;
			robot_state::robotStateToRobotStateMsg(start, dt.trajectory_start);
			dt.trajectory.resize(1);
			trajectory.getRobotTrajectoryMsg(dt.trajectory[0]);
			dt.model_id= start.getRobotModel()->getName();
			pub->publish(dt);
}