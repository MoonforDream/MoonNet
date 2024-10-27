// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//


#include "buffer.h"

using namespace moon;


void buffer::append(const char *data,size_t len){
    able_wirte(len);
    memcpy(buffer_.data() + writer_, data, len);
    writer_+=len;
}


size_t buffer::remove(char *data,size_t len){
    size_t rbytes=std::min(len,readbytes());
    memcpy(data,buffer_.data()+reader_,rbytes);
    reader_+=rbytes;
    if(reader_==writer_) reset();
    return rbytes;
}


std::string buffer::remove(size_t len){
    size_t rbytes = std::min(len, readbytes());
    std::string data(buffer_.data()+reader_,rbytes);
    reader_ += rbytes;
    if(reader_ == writer_) reset();
    return data;
}


void buffer::retrieve(size_t len){
    size_t rbytes = std::min(len, readbytes());
    reader_ += rbytes;
    if(reader_ == writer_) reset();
}


size_t buffer::readbytes() const{
    return writer_- reader_;
}


size_t buffer::writebytes() const{
    return buffer_.size()-writer_;
}


const char* buffer::peek() const{
    return buffer_.data()+reader_;
}


void buffer::reset(){
    reader_=writer_=0;
}



ssize_t buffer::readiov(int fd,int& errnum){
    char buf[IOBUF];
    struct iovec vec[2];
    const size_t wbytes=writebytes();
    const size_t buflen=sizeof(buf);

    vec[0].iov_base=&*buffer_.begin()+writer_;
    vec[0].iov_len=wbytes;
    vec[1].iov_base=buf;
    vec[1].iov_len=buflen;

    const int iovlen=(wbytes<buflen)?2:1;
    const ssize_t n=readv(fd,vec,iovlen);
    if(n<0) errnum=errno;
    else if(static_cast<size_t>(n)<=wbytes){
        writer_+=n;
    }else{
        writer_=buffer_.size();
        append(buf,n-wbytes);
    }
    return n;
}



