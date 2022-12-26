/*
 * Copyright (C) 2017 Hellfire <https://hellfire-core.github.io/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include <ace/Log_Msg.h>

#include "DelayExecutor.h"


DelayExecutor* DelayExecutor::instance()
{
  return ACE_Singleton<DelayExecutor, ACE_Thread_Mutex>::instance();
}

DelayExecutor::DelayExecutor () :
activated_ (false),
pre_svc_hook_ (NULL),
post_svc_hook_ (NULL) { }

DelayExecutor::~DelayExecutor()
{
  if (pre_svc_hook_)
    delete pre_svc_hook_;

  if (post_svc_hook_)
    delete post_svc_hook_;

  this->deactivate ();

  //todo probably free the queue ??
}

int DelayExecutor::deactivate()
{
  if (!this->activated())
    return -1;

  this->activated(false);

  this->queue_.queue()->deactivate();

  this->wait ();

  return 0;
}

int DelayExecutor::svc(void)
{
  if (pre_svc_hook_)
    pre_svc_hook_->call();

 for (;;)
    {
      ACE_Method_Request* rq = this->queue_.dequeue();

      if (!rq)
        break;

      rq->call();
      delete rq;
    }

  if (post_svc_hook_)
    post_svc_hook_->call();

  return 0;
}

int DelayExecutor::activate (int num_threads,
                         ACE_Method_Request* pre_svc_hook,
                         ACE_Method_Request* post_svc_hook)
{
  if (this->activated())
    return -1;

  if (num_threads < 1)
    return -1;

  if (this->pre_svc_hook_)
    delete this->pre_svc_hook_;

  if (this->post_svc_hook_)
    delete this->post_svc_hook_;

  this->pre_svc_hook_ = pre_svc_hook;
  this->post_svc_hook_ = post_svc_hook;

  this->queue_.queue()->activate();

  if( ACE_Task_Base::activate (THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
                                  num_threads) == -1)
    return -1;

  this->activated(true);

  return true;
}

int DelayExecutor::execute (ACE_Method_Request* new_req)
{
  if(new_req == NULL)
    return -1;

  if(this->queue_.enqueue(new_req,
                           (ACE_Time_Value*)&ACE_Time_Value::zero) == -1)
    {
      delete new_req;
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("(%t) %p\n"),
                         ACE_TEXT ("DelayExecutor::execute enqueue")),
                        -1);
    }
  return 0;
}

bool DelayExecutor::activated()
{
  return this->activated_;
}

void DelayExecutor::activated(bool s)
{
  this->activated_ = s;
}