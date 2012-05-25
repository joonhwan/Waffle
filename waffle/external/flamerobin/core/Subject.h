/*
  Copyright (c) 2004-2012 The FlameRobin Development Team

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


  $Id: Subject.h 2200 2012-01-20 08:31:01Z mghie $

*/

#ifndef FR_SUBJECT_H
#define FR_SUBJECT_H
//-----------------------------------------------------------------------------
#include <list>
#include <vector>

namespace fr {

class Observer;
//-----------------------------------------------------------------------------
class Subject
{
private:
    friend class SubjectLocker;

    unsigned int locksCountM;
    std::list<Observer*> observersM;
    bool needsNotifyObjectsM;
protected:
    // make these protected, as instances of this class are bogus...
    Subject();
    virtual ~Subject();

    virtual void lockedChanged(bool locked);
public:
    unsigned int getLockCount();
    virtual bool isLocked();
    virtual void lockSubject();
    virtual void unlockSubject();

    void attachObserver(Observer* observer, bool callUpdate);
    void detachObserver(Observer* observer);
    void detachAllObservers();
    bool isObservedBy(Observer* observer) const;
    void notifyObservers();
};
//-----------------------------------------------------------------------------
class SubjectLocker
{
private:
    Subject* subjectM;
protected:
    Subject* getSubject();
    void setSubject(Subject* subject);
public:
    SubjectLocker(Subject* subject);
    ~SubjectLocker();
};
//-----------------------------------------------------------------------------

} // namespace fr

#endif
