// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "buffer.h"

using namespace moon;

/**
 * @brief Appends data to the buffer.
 *
 * This function appends `len` bytes of data from the `data` pointer to the
 * buffer. It ensures there is enough writable space by invoking
 * `able_write(len)`, then copies the data into the buffer and updates the write
 * position.
 *
 * @param data Pointer to the data to be appended.
 * @param len The number of bytes to append.
 */
void buffer::append(const char* data, size_t len) {
    able_wirte(len);
    memcpy(buffer_.data() + writer_, data, len);
    writer_ += len;
}

/**
 * @brief Removes data from the buffer and copies it into a provided buffer.
 *
 * This function removes up to `len` bytes of data from the buffer and copies it
 * into the `data` pointer. It calculates the actual number of bytes to remove
 * based on the available readable bytes, copies the data, updates the read
 * position, and resets the buffer if all data has been read.
 *
 * @param data Pointer to the buffer where removed data will be stored.
 * @param len The maximum number of bytes to remove.
 *
 * @return The actual number of bytes removed and copied into `data`.
 */
size_t buffer::remove(char* data, size_t len) {
    size_t rbytes = std::min(len, readbytes());
    memcpy(data, buffer_.data() + reader_, rbytes);
    reader_ += rbytes;
    if (reader_ == writer_) reset();
    return rbytes;
}

/**
 * @brief Removes data from the buffer and returns it as a `std::string`.
 *
 * This function removes up to `len` bytes of data from the buffer and returns
 * it as a `std::string`. It calculates the actual number of bytes to remove
 * based on the available readable bytes, constructs the string, updates the
 * read position, and resets the buffer if all data has been read.
 *
 * @param len The maximum number of bytes to remove.
 *
 * @return A `std::string` containing the removed data.
 */
std::string buffer::remove(size_t len) {
    size_t rbytes = std::min(len, readbytes());
    std::string data(buffer_.data() + reader_, rbytes);
    reader_ += rbytes;
    if (reader_ == writer_) reset();
    return data;
}

/**
 * @brief Removes a specified amount of data from the buffer.
 *
 * This function removes all available readable bytes from the buffer and
 * returns them as a `std::string`. It updates the read position and resets the
 * buffer if all data has been read.
 *
 * @return A `std::string` containing all removed data.
 */
std::string buffer::remove() { return remove(readbytes()); }

/**
 * @brief Retrieves and discards a specified amount of data from the buffer.
 *
 * This function removes up to `len` bytes of data from the buffer without
 * copying it to any external buffer. It updates the read position and resets
 * the buffer if all data has been read.
 *
 * @param len The number of bytes to retrieve and discard.
 */
void buffer::retrieve(size_t len) {
    size_t rbytes = std::min(len, readbytes());
    reader_ += rbytes;
    if (reader_ == writer_) reset();
}

/**
 * @brief Returns the number of readable bytes in the buffer.
 *
 * This function calculates and returns the number of bytes available for
 * reading.
 *
 * @return The number of readable bytes.
 */
size_t buffer::readbytes() const { return writer_ - reader_; }

/**
 * @brief Returns the number of writable bytes in the buffer.
 *
 * This function calculates and returns the remaining writable space in the
 * buffer.
 *
 * @return The number of writable bytes.
 */
size_t buffer::writebytes() const { return buffer_.size() - writer_; }

/**
 * @brief Provides a pointer to the beginning of the readable data in the
 * buffer.
 *
 * This function returns a constant pointer to the current read position in the
 * buffer.
 *
 * @return A constant pointer to the readable data.
 */
const char* buffer::peek() const { return buffer_.data() + reader_; }

void buffer::reset() { reader_ = writer_ = 0; }

/**
 * @brief Reads data from a file descriptor into the buffer using `readv`.
 *
 * This function attempts to read data from the specified file descriptor (`fd`)
 * into the buffer. It uses the `readv` system call to read data into two
 * buffers: the writable space in the buffer and a temporary buffer (`buf`). It
 * handles partial reads and appends any remaining data to the buffer.
 *
 * @param fd The file descriptor to read data from.
 * @param errnum Reference to an integer where the error number will be stored
 * in case of failure.
 *
 * @return The number of bytes read on success, or -1 on failure.
 */
ssize_t buffer::readiov(int fd, int& errnum) {
    char buf[IOBUF];
    struct iovec vec[2];
    const size_t wbytes = writebytes();
    const size_t buflen = sizeof(buf);

    vec[0].iov_base = &*buffer_.begin() + writer_;
    vec[0].iov_len = wbytes;
    vec[1].iov_base = buf;
    vec[1].iov_len = buflen;

    const int iovlen = (wbytes < buflen) ? 2 : 1;
    const ssize_t n = readv(fd, vec, iovlen);
    if (n < 0)
        errnum = errno;
    else if (static_cast<size_t>(n) <= wbytes) {
        writer_ += n;
    } else {
        writer_ = buffer_.size();
        append(buf, n - wbytes);
    }
    return n;
}

/**
 * @brief Ensures there is enough writable space in the buffer and resizes if
 * necessary.
 *
 * This function checks if there is sufficient writable space (`len`) in the
 * buffer. If not, and if there is space at the beginning of the buffer (i.e.,
 * `reader_ > 0`), it moves the readable data to the front to make space. If
 * there is still not enough space, it resizes the buffer to accommodate the
 * additional data.
 *
 * @param len The number of bytes that need to be writable.
 */
void buffer::able_wirte(size_t len) {
    // 如果reader_前和writer_后还有空间，将中间可读数据移动回前面,为写缓冲提供空间
    if (writebytes() < len && reader_ > 0) {
        size_t rbytes = readbytes();
        memmove(buffer_.data(), buffer_.data() + reader_, rbytes);
        // std::copy(buffer_.begin()+reader_,buffer_.begin()+writer_,buffer_.begin());
        reader_ = 0;
        writer_ = rbytes;
    }
    if (writebytes() < len) {
        buffer_.resize(writer_ + len);
    }
}
