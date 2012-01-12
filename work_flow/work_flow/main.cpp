#include "workflow.h"
#include <iostream>
#include <ctime>

enum testflow_work_id
{
	testflow_work_1,
	testflow_work_2,
	testflow_work_3
};

class testflow_context : public context_base
{
public:
	testflow_context() {}
	virtual ~testflow_context() {}
};

class testwork1 : public work_base
{
public:
	testwork1( work_flow* flow, int sec ) : work_base(flow, sec) {}
	~testwork1() {}

	virtual unsigned int get_id() const { return testflow_work_1; }
	virtual bool begin() { std::cout << "begin testwork1!" << std::endl; return true; }
	virtual void end( bool success, void* data = NULL ) { std::cout << "work_base id = " << get_id() << " execute success!" << std::endl; }
};

class testwork2 : public work_base
{
public:
	testwork2( work_flow* flow, int sec ) : work_base(flow, sec) {}
	~testwork2() {}

	virtual unsigned int get_id() const { return testflow_work_2; }
	virtual bool begin() { std::cout << "begin testwork2!" << std::endl; return true; }
	virtual void end( bool success, void* data = NULL ) { std::cout << "work_base id = " << get_id() << " execute success!" << std::endl; }
};

class testwork3 : public work_base
{
public:
	testwork3( work_flow* flow, int sec ) : work_base(flow, sec) {}
	~testwork3() {}

	virtual unsigned int get_id() const { return testflow_work_3; }
	virtual bool begin() { std::cout << "begin testwork3!" << std::endl; return true; }
	virtual void end( bool success, void* data = NULL ) { std::cout << "work_base id = " << get_id() << " execute success!" << std::endl; }
};

int main()
{
	work_flow_manager work_flow_manager_;

	work_flow* work_flow_ = new work_flow(new testflow_context);
	
	testwork1* work1 = new testwork1(work_flow_, 5);
	work_flow_->link_work(work1);

	testwork2* work2 = new testwork2(work_flow_, 5);
	work_flow_->link_work(work2);

	testwork3* work3 = new testwork3(work_flow_, 5);
	work_flow_->link_work(work3);
	unsigned int flow_id = work_flow_->submit(&work_flow_manager_);

	time_t time_stamp = time(NULL);
	while (true)
	{
		time_t time_stamp_ = time(NULL);
		if (time_stamp_ - time_stamp >= 30)
			break;

		work_flow_manager_.update();
		work_flow_manager_.plugging(flow_id, testflow_work_1, true);
	};
	
	return 0;
}