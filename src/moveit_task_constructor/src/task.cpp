#include <moveit_task_constructor/task.h>
#include <moveit_task_constructor/subtask.h>
#include <moveit/robot_model_loader/robot_model_loader.h> 
#include <rclcpp/rclcpp.hpp>

#include <moveit_msgs/srv/get_planning_scene.hpp>

moveit::task_constructor::Task::Task()
{
	// rml_.reset(new robot_model_loader::RobotModelLoader); //error
	if( !rml_->getModel() )
	throw Exception("Task failed to construct RobotModel");

	std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared("get_planning_scene_client");
	rclcpp::Client<moveit_msgs::srv::GetPlanningScene>::SharedPtr client = node->create_client<moveit_msgs::srv::GetPlanningScene>("get_planning_scene");
	client->wait_for_service(); //client.waitForExistence();

	moveit_msgs::srv::GetPlanningScene::Request req;
	moveit_msgs::srv::GetPlanningScene::Response res;
	
	req.components.components =
		  moveit_msgs::msg::PlanningSceneComponents::SCENE_SETTINGS
		| moveit_msgs::msg::PlanningSceneComponents::ROBOT_STATE
		| moveit_msgs::msg::PlanningSceneComponents::ROBOT_STATE_ATTACHED_OBJECTS
		| moveit_msgs::msg::PlanningSceneComponents::WORLD_OBJECT_NAMES
		| moveit_msgs::msg::PlanningSceneComponents::WORLD_OBJECT_GEOMETRY
		| moveit_msgs::msg::PlanningSceneComponents::OCTOMAP
		| moveit_msgs::msg::PlanningSceneComponents::TRANSFORMS
		| moveit_msgs::msg::PlanningSceneComponents::ALLOWED_COLLISION_MATRIX
		| moveit_msgs::msg::PlanningSceneComponents::LINK_PADDING_AND_SCALING
		| moveit_msgs::msg::PlanningSceneComponents::OBJECT_COLORS;
	
	// if(!client.call(req, res)){
		// throw Exception("Task failed to aquire current PlanningScene");
	// }
	// client->async_send_request(req); //new
	// planning_scene::PlanningScenePtr ps(new planning_scene::PlanningScene(rml_->getModel()));
	// ps->setPlanningSceneMsg(res.scene);
	scene_.reset(new planning_scene::PlanningScene(rml_->getModel()));
	scene_->setPlanningSceneMsg(res.scene);
}

void moveit::task_constructor::Task::addStart( SubTaskPtr subtask ){
	subtasks_.clear();
	addSubTask( subtask );
}

void moveit::task_constructor::Task::addAfter( SubTaskPtr subtask ){
	subtask->addPredecessor( subtasks_.back() );
	subtasks_.back()->addSuccessor( subtask );
	addSubTask( subtask );
}

bool moveit::task_constructor::Task::plan(){
	for( SubTaskPtr& subtask : subtasks_ ){
		std::cout << "Computing subtask '" << subtask->getName() << "': ";
		bool success= subtask->compute();
		std::cout << (success ? "succeeded" : "failed") << std::endl;
	}
	return false;
}

void moveit::task_constructor::Task::addSubTask( SubTaskPtr subtask ){
	subtask->setPlanningScene( scene_ );
	subtasks_.push_back( subtask );
}