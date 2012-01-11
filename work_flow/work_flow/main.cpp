#include "workflow.h"
#include <iostream>

class testwork1 : public work_base
{
public:
	testwork1( work_flow* flow, int sec ) : work_base(flow, sec) {}
	~testwork1() {}

	virtual bool request( void* ctx ) { std::cout << "request work1!" << std::endl; return true; }
	virtual void* clone_output_context( void* ctx ) { return NULL; }
};

class testwork2 : public work_base
{
public:
	testwork2( work_flow* flow, int sec ) : work_base(flow, sec) {}
	~testwork2() {}

	virtual bool request( void* ctx ) { std::cout << "request work2!" << std::endl; return true; }
	virtual void* clone_output_context( void* ctx ) { return NULL; }
};

class testwork3 : public work_base
{
public:
	testwork3( work_flow* flow, int sec ) : work_base(flow, sec) {}
	~testwork3() {}

	virtual bool request( void* ctx ) { std::cout << "request work3!" << std::endl; return true; }
	virtual void* clone_output_context( void* ctx ) { return NULL; }
};

int main()
{
	work_flow_manager work_flow_manager_;
	work_flow* work_flow_ = new work_flow;
	
	testwork1* work1 = new testwork1(work_flow_, 10);
	work1->set_input_context(NULL);
	work_flow_->link_work(work1);

	testwork2* work2 = new testwork2(work_flow_, 10);
	work_flow_->link_work(work2);

	testwork3* work3 = new testwork3(work_flow_, 20);
	work_flow_->link_work(work3);
	work_flow_->submit(&work_flow_manager_);

	while (true)
	{
		work_flow_manager_.update();
		work1->response_complete(true, NULL);
		//work2->response_complete(true, NULL);
	};
	
	return 0;
}