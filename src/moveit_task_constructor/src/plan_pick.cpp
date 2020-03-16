#include <moveit_task_constructor/task.h>
#include <moveit_task_constructor/subtask.h>

#include <moveit_task_constructor/subtasks/current_state.h>

using namespace moveit::task_constructor;

int main(){
// /*
	Task t;

	std::shared_ptr<int> foo = std::make_shared<int> (10);

	t.addStart( std::make_shared<subtasks::CurrentState>("current state") );
/*
	{
		auto move= std::make_shared<subtasks::Gripper>("open gripper");
		move->setGroup("gripper");
		move->setTo("open");
		t.addAfter(move);
	}

	{
		auto move= std::make_shared<subtasks::Move>("move to pre-grasp");
		move->setGroup("arm");
		move->setPlannerId("RRTConnectkConfigDefault");
		move->setPlanningTime(5.0);
		t.addAfter(move);
	}

	{
		auto move= std::make_shared<subtasks::CartesianPositionMotion>("proceed to grasp pose");
		move->minMaxDistance(3.0, 10.0);
		move->towards("object", 0.0, 0.0, 0.0);
		t.addAfter(move);
	}

	{
		auto grasps= std::make_shared<GenerateGraspPose>("generate grasp pose");
	}

	{
		auto move= std::make_shared<subtasks::Gripper>("grasp");
		move->setGroup("gripper");
		move->setTo("closed");
		move->graspObject("object");
		t.addAfter(move);
	}

	{
		auto move= std::make_shared<subtasks::CartesianPositionMotion>("Lift Object");
		move->setGroup("arm");
		move->minMaxDistance(3.0, 5.0);
		move->along("world", 0.0, 0.0, 1.0);
		t.addAfter(move);
	}
*/
	t.plan();

	return 0;
}