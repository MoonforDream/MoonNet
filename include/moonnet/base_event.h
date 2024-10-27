#ifndef _BASE_EVENT_H
#define _BASE_EVENT_H

namespace moon {

class eventloop;
class base_event {
public:
    virtual ~base_event(){}
    virtual eventloop* getloop() const=0;
    virtual void close()=0;
    virtual void disable_cb()=0;
};

}

#endif
