// $Id$
//
// CalFilter implementation
//

#include "kdebug.h"

#include "calfilter.h"

using namespace KCal;

CalFilter::CalFilter()
{
  mEnabled = true;
  mInclusion = 0;
  mExclusion = 0;
}

CalFilter::CalFilter(const QString &name)
{
  mName = name;
}

CalFilter::~CalFilter()
{
}

void CalFilter::apply(QList<Event> *eventlist)
{
  if (!mEnabled) return;

//  kdDebug() << "CalFilter::apply()" << endl;

  Event *event = eventlist->first();
  while(event) {
    if (!filterEvent(event)) {
      eventlist->remove();
      event = eventlist->current();
    } else {
      event = eventlist->next();
    }
  }

//  kdDebug() << "CalFilter::apply() done" << endl;
}

bool CalFilter::filterEvent(Event *event)
{
//  kdDebug() << "CalFilter::filterEvent(): " << event->getSummary() << endl;

  if (mInclusion) {
    bool passed = false;
    if (mInclusion & Recurring) {
      if (event->recurrence()->doesRecur()) passed = true;
    }
    if (mInclusion & Floating) {
      if (event->doesFloat()) passed = true;
    }
    if (!passed) return false;
  }
  
  
  if (mExclusion) {
    if (mExclusion & Recurring) {
      if (event->recurrence()->doesRecur()) return false;
    }
    if (mExclusion & Floating) {
      if (event->doesFloat()) return false;
    }
  }
  
//  kdDebug() << "CalFilter::filterEvent(): passed" << endl;
  
  return true;
}

void CalFilter::setEnabled(bool enabled)
{
  mEnabled = enabled;
}

bool CalFilter::isEnabled()
{
  return mEnabled;
}

void CalFilter::setInclusionCriteria(int criteria)
{
  mInclusion = criteria;
}

int CalFilter::inclusionCriteria()
{
  return mInclusion;
}

void CalFilter::setExclusionCriteria(int criteria)
{
  mExclusion = criteria;
}

int CalFilter::exclusionCriteria()
{
  return mExclusion;
}
