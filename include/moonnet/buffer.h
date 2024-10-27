// Modified from muduo project http://github.com/chenshuo/muduo
// @see https://github.com/chenshuo/muduo/blob/master/muduo/net/Buffer.h 
// and https://github.com/chenshuo/muduo/blob/master/muduo/net/Buffer.cc

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <cstddef>
#include <vector>
#include <stdint.h>
#include <string>
#include <string.h>
#include <sys/uio.h>
#include <bits/types/struct_iovec.h>


#define BUFSIZE 1024
#define IOBUF 65536

namespace moon {

class buffer{
public:
    buffer():reader_(0),writer_(0),buffer_(BUFSIZE){}
    ~buffer(){
        reset();
    }
    //向writer_后添加数据
    void append(const char *data,size_t len);
    //读取reader_和writer_之间的len长度可读数据
    size_t remove(char *data,size_t len);
    std::string remove(size_t len);
    // 仅移动读指针，不复制数据
    void retrieve(size_t len);
    size_t readbytes() const;   //获取可读数据大小
    size_t writebytes() const;  //获取可写数据大小
    const char* peek() const;   //获取缓冲区内容
    void reset();   //重置缓冲区
    ssize_t readiov(int fd,int& errnum);
private:
    //确保有足够的科可写空间
    void able_wirte(size_t len){
        //如果reader_前和writer_后还有空间，将中间可读数据移动回前面,为写缓冲提供空间
        if(writebytes()<len&&reader_>0){
            size_t rbytes=readbytes();
            memmove(buffer_.data(),buffer_.data()+reader_,rbytes);
            // std::copy(buffer_.begin()+reader_,buffer_.begin()+writer_,buffer_.begin());
            reader_=0;
            writer_=rbytes;
        }
        if(writebytes()<len){
            buffer_.resize(writer_+len);
        }
    }

private:
    std::vector<char> buffer_;
    uint64_t reader_;
    uint64_t writer_;
};

}

#endif
