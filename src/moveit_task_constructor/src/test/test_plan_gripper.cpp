#include <rclcpp/rclcpp.hpp>

#include <moveit_task_constructor/task.h>

#include <moveit_task_constructor/subtasks/current_state.h>
#include <moveit_task_constructor/subtasks/gripper.h>

using namespace moveit::task_constructor;

int main(int argc, char** argv){
    rclcpp::init(argc, argv); // rclcpp::init(argc, argv, "test_plan_gripper");
    std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared("test_plan_gripper");
    
	// rclcpp::AsyncSpinner spinner(1); //error
	// spinner.start();

	Task t;
    
	t.addStart( std::make_shared<subtasks::CurrentState>("current state") );

	{
		auto gripper= std::make_shared<subtasks::Gripper>("close gripper");
		gripper->setGroup("gripper");
		gripper->setTo("closed");
		t.addAfter( gripper );
	}
	
	t.plan();
	
	{
		auto gripper= std::make_shared<subtasks::Gripper>("close gripper");
		gripper->setGroup("gripper");
		gripper->setTo("closed");
		t.addStart( gripper );
	}

	t.addAfter( std::make_shared<subtasks::CurrentState>("current state") );

	// t.printState();

	rclcpp::spin(node);
	rclcpp::shutdown();
	return 0;
}