#include "workflow.h"
#include <ctime>

work_base::work_base( work_flow* flow_, int sec )
: seconds(sec)
, prev(NULL)
, next(NULL)
, state_(work_state_none)
, flow(flow_)
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_base cstr!");
}

work_base::~work_base()
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_base dstr!");
}

void work_base::end_( bool success )
{
	if (state_ != work_state_pending)
		return;

	if (!success)
	{
		flow->execute_res_ = work_flow_execute_result_fail;
		ASYNC_UTILITY_DEBUG_OUTPUT("work_flow execute failed!");
	}
	else
	{
		if (!next)
		{
			flow->execute_res_ = work_flow_execute_result_success;
			ASYNC_UTILITY_DEBUG_OUTPUT("work_flow execute success!");
		}
		else
			flow->cursor = next;
	}

	if (flow->execute_res_ != work_flow_execute_result_unknow)
	{
		/// todo error handler
		flow->gc_status = true;
	}
}

void work_base::update()
{
	if (state_ == work_state_none)
	{
		/// start timeout
		start_time = time(NULL);

		if (!begin())
		{
			flow->execute_res_ = work_flow_execute_result_fail;
			return;
		}
		
		state_ = work_state_pending;
	}

	return;
}

work_flow::work_flow( context_base* ctx )
: head(NULL)
, tail(NULL)
, cursor(NULL)
, execute_res_(work_flow_execute_result_unknow)
, gc_status(false)
, context(ctx)
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_flow cstr!");
}

work_flow::~work_flow()
{
	work_base* curr = head;
	while(curr)
	{
		work_base* next = curr->next;
		SAFE_DELETE(curr);
		curr = next;
	}

	SAFE_DELETE(context);
	ASYNC_UTILITY_DEBUG_OUTPUT("work_flow dstr!");
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

unsigned int work_flow::submit( work_flow_manager* mgr )
{
	cursor = head;
	return mgr->add_work_flow(this);
}

void work_flow::update()
{
	if (execute_res_ != work_flow_execute_result_unknow)
		return;

	if (cursor)
		cursor->update();
}

work_flow_manager::work_flow_manager()
: curr_id(0)
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_flow_manager cstr!");
	work_flows.clear();
	free_ids.clear();
}

work_flow_manager::~work_flow_manager()
{
	ASYNC_UTILITY_DEBUG_OUTPUT("work_flow_manager dstr!");
}

unsigned int work_flow_manager::add_work_flow( work_flow* flow )
{
	ASYNC_UTILITY_ASSERT(flow);

	/// make id
	int new_id;
	if (!free_ids.empty())
		new_id = free_ids.front();
	else
		new_id = ++curr_id;

	std::map<unsigned int, work_flow*>::iterator itr = work_flows.find(new_id);
	ASYNC_UTILITY_ASSERT(itr == work_flows.end());

	/// not care insert failed
	work_flows.insert(std::make_pair(new_id, flow));
	return new_id;
}

void work_flow_manager::update()
{
	time_t curr_time = time(NULL);

	std::map<unsigned int, work_flow*>::iterator flow_itr = work_flows.begin();
	for (; flow_itr != work_flows.end();)
	{
		work_flow* flow = flow_itr->second;
		if (!flow || flow->gc_status)
		{
			free_ids.push_back(flow_itr->first);
			SAFE_DELETE(flow_itr->second);
			work_flows.erase(flow_itr++);
			continue;
		}

		ASYNC_UTILITY_ASSERT(flow->execute_res_ == work_flow_execute_result_unknow);

		/// timeout, just case the current state of flow
		work_base* work = flow->cursor;
		if (work && work->state_ == work_state_pending)
		{
			if ((curr_time - work->start_time) >= work->seconds)
			{
				/// timeout
#if ASYNC_UTILITY_DEBUG
				std::cout << "work_base id = " << work->get_id() << " execute timeout!" << std::endl;
#endif
				work->end_(false);
			}
		}

		/// flow
		flow->update();

		++flow_itr;
	}
}

void work_flow_manager::plugging( unsigned int flow_id, unsigned int work_id, bool success, void* data /*= NULL */ )
{
	std::map<unsigned int, work_flow*>::iterator itr = work_flows.find(flow_id);
	if (itr != work_flows.end())
	{
		work_flow* flow = itr->second;
		work_base* work = flow->cursor;
		if (work && work->get_id() == work_id)
		{
			work->end(success, data);
			work->end_(success);
		}
	}
}