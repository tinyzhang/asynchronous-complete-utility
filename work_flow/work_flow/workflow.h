/*
work flow
author tinyzhang
date 2012/01/09
*/
#ifndef __workflow_h__
#define __workflow_h__

#include "workflowcommon.h"
#include <list>

enum work_state
{
	work_state_none = 0,
	work_state_pending,
};

class work_base
{
	friend class work_flow;
	friend class work_flow_manager;
public:
	work_base( work_flow* flow_, int sec );
	virtual ~work_base();

public:
	virtual bool request( void* ctx ) = 0;
	virtual void* clone_output_context( void* ctx ) = 0;
	
	/// ***called when this work response***
	void response_complete( bool success, void* data = NULL );
	/// ************************************

	void set_input_context( void* ctx ) { intput_context = ctx; }
	void* get_output_context() { return output_context; }

private:
	bool start();
	void update();

private:
	int seconds;
	time_t start_time;
	bool timeout_calc_status;

	work_base* prev;
	work_base* next;
	work_state state_;

	work_flow* flow;
	bool response_complete_;

private:
	void* intput_context;
	void* output_context;

private:
	DISALLOW_EVIL_CONSTRUCTORS( work_base );
};

class work_flow_manager;

enum work_flow_execute_result
{
	work_flow_execute_result_unknow = 0,
	work_flow_execute_result_success,
	work_flow_execute_result_fail
};

class work_flow
{
	friend class work_base;
	friend class work_flow_manager;
public:
	work_flow();
	~work_flow();

public:
	void link_work( work_base* work );
	void submit( work_flow_manager* mgr );
	void update();

private:
	work_base* head;
	work_base* tail;
	work_base* cursor;
	work_flow_manager* mgr_;
	work_flow_execute_result execute_res_;
	bool gc_status;

private:
	DISALLOW_EVIL_CONSTRUCTORS( work_flow );
};

/// a instance per thread
class work_flow_manager
{
public:
	work_flow_manager();
	~work_flow_manager();

public:
	void add_work_flow( work_flow* flow );
	void update();
	
	/// todo precise control

private:
	void update_timeout( work_flow* flow, time_t curr_time );

private:
	std::list<work_flow*> work_flows;

private:
	DISALLOW_EVIL_CONSTRUCTORS( work_flow_manager );
};

#endif