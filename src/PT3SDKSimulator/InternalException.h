#pragma once

#include <stdexcept>
#include <Prefix_Integer.h>

namespace EARTH {
namespace PT {


/**
* 内部エラー
*/
class InternalException : public std::runtime_error {

public:

  explicit InternalException(EARTH::status status, const std::string& what_arg) : std::runtime_error(what_arg), mStatus(status){}

  explicit InternalException(EARTH::status status, const char* what_arg) : std::runtime_error(what_arg), mStatus(status) {}

  EARTH::status status() const { return mStatus; }

private:

  EARTH::status mStatus;
};


/**
* 1台（奇数台）接続時に発生するエラー（基本的に正常扱い）
*/
class SingleDeviceException : public std::runtime_error {

public:

  explicit SingleDeviceException(const std::string& what_arg) : std::runtime_error(what_arg) {}

  explicit SingleDeviceException(const char* what_arg) : std::runtime_error(what_arg) {}
};


}
}