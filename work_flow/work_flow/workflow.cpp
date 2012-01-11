#include "workflow.h"
#include <time.h>

work_base::work_base( work_flow* flow_, int sec )
: seconds(sec)
, timeout_calc_status(false)
, prev(NULL)
, next(NULL)
, state_(work_state_none)
, flow(flow_)
, intput_context(NULL)
, output_context(NULL)
, response_complete_(false)
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_base cstr!");
}

work_base::~work_base()
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_base dstr!");
	SAFE_DELETE(intput_context);
	SAFE_DELETE(output_context);
}

void work_base::response_complete( bool success, void* data /*= NULL */ )
{
	if (state_ != work_state_pending || response_complete_)
		return;

	output_context = data;

	if (!success)
	{
		flow->execute_res_ = work_flow_execute_result_fail;
	}
	else
	{
		if (!next)
			flow->execute_res_ = work_flow_execute_result_success;
		else
			flow->cursor = next;
	}

	if (flow->execute_res_ != work_flow_execute_result_unknow)
	{
		/// todo error handler
		
		flow->gc_status = true;
	}

	response_complete_ = true;
}

bool work_base::start()
{
	/// timeout
	timeout_calc_status = true;
	start_time = time(NULL);

	if (prev)
		intput_context = clone_output_context(prev->intput_context);
	
	return request(intput_context);
}

void work_base::update()
{
	if (state_ == work_state_none)
	{
		if (!start())
		{
			flow->execute_res_ = work_flow_execute_result_fail;
			return;
		}
		
		state_ = work_state_pending;
	}

	return;
}

work_flow::work_flow()
: head(NULL)
, tail(NULL)
, cursor(NULL)
, execute_res_(work_flow_execute_result_unknow)
, gc_status(false)
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_flow cstr!");
}

work_flow::~work_flow()
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_flow dstr!");
	work_base* curr = head;
	while(curr)
	{
		work_base* next = curr->next;
		SAFE_DELETE(curr);
		curr = next;
	}
}

void work_flow::link_work( work_base* work )
{
	ASYNC_UTILITY_ASSERT(work);
	
	if (!tail)
	{
		work->prev = work->next = NULL;
		head = tail = work;
	}
	else
	{
		tail->next = work;
		work->prev = tail;
		work->next = NULL;
		tail = work;
	}
}

void work_flow::submit( work_flow_manager* mgr )
{
	cursor = head;
	mgr->add_work_flow(this);
}

void work_flow::update()
{
	if (execute_res_ != work_flow_execute_result_unknow)
		return;

	if (cursor)
		cursor->update();
}

work_flow_manager::work_flow_manager()
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_flow_manager cstr!");
}

work_flow_manager::~work_flow_manager()
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_flow_manager dstr!");
}

void work_flow_manager::add_work_flow( work_flow* flow )
{
	work_flows.push_back(flow);
}

void work_flow_manager::update()
{
	time_t curr_time = time(NULL);

	std::list<work_flow*>::iterator flow_itr = work_flows.begin();
	for (; flow_itr != work_flows.end();)
	{
		work_flow* flow = *flow_itr;
		if (!flow || flow->gc_status)
		{
			SAFE_DELETE(*flow_itr);
			work_flows.erase(flow_itr++);
			continue;
		}

		/// timeout
		update_timeout(flow, curr_time);

		/// flow
		flow->update();

		++flow_itr;
	}
}

void work_flow_manager::update_timeout( work_flow* flow, time_t curr_time )
{
	ASYNC_UTILITY_ASSERT(flow);

	work_base* work = flow->head;
	while (work)
	{
		if (work->timeout_calc_status && !work->response_complete_)
		{
			if ((curr_time - work->start_time) >= work->seconds)
			{
				work->timeout_calc_status = false;
				/// timeout
				work->response_complete(false, NULL);
			}
		}
		work = work->next;
	}
}