/*
work flow
author tinyzhang
date 2012/01/09
*/
#ifndef __workflow_h__
#define __workflow_h__

#include "workflowcommon.h"
#include <list>
#include <map>

enum work_state
{
	work_state_none = 0,
	work_state_pending,
};

class work_base
{
	friend class work_flow;
	friend class work_flow_manager;
	enum { default_timeout_seconds = 0x20 };
public:
	/// need define id
	work_base( work_flow* flow_, int sec = default_timeout_seconds );
	virtual ~work_base();

public:
	virtual unsigned int get_id() const = 0;
	virtual bool begin() = 0;
	virtual void end( bool success, void* data = NULL ) = 0;

private:
	void end_( bool success );
	void update();

private:
	int seconds;
	time_t start_time;

	work_base* prev;
	work_base* next;
	work_state state_;

	work_flow* flow;

private:
	work_base();
	DISALLOW_EVIL_CONSTRUCTORS( work_base );
};

/// the context contains entrys that work flow wanna know
/// several kind of entrys:
/// 1. associated with user's context entry, not care the lifetime
/// 2. stack entry, auto
/// 3. heap entry, lifetime controlled by destructor
class context_base
{
public:
	virtual ~context_base() {}
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
	explicit work_flow( context_base* ctx );
	~work_flow();

public:
	void link_work( work_base* work );
	unsigned int submit( work_flow_manager* mgr );

private:
	void update();

private:
	work_base* head;
	work_base* tail;
	work_base* cursor;
	work_flow_manager* mgr_;
	work_flow_execute_result execute_res_;
	bool gc_status;

private:
	context_base* context;

private:
	work_flow();
	DISALLOW_EVIL_CONSTRUCTORS( work_flow );
};

/// a instance per thread
/// todo precise control
class work_flow_manager
{
public:
	work_flow_manager();
	~work_flow_manager();

public:
	void plugging( unsigned int flow_id, unsigned int work_id, bool success, void* data = NULL );
	void update();
	unsigned int add_work_flow( work_flow* flow );

private:
	void update_timeout( work_flow* flow, time_t curr_time );

private:
	std::map<unsigned int, work_flow*> work_flows;

	unsigned int curr_id;
	std::list<unsigned int> free_ids;

private:
	DISALLOW_EVIL_CONSTRUCTORS( work_flow_manager );
};

#endif