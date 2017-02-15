//
//  buffer_yi_util.hpp
//  heibaiyu
//
//  Created by jiwei.wang on 2/10/17.
//  Copyright © 2017 yijian. All rights reserved.
//

#ifndef buffer_yi_util_hpp
#define buffer_yi_util_hpp

#include "buffer_yi.h"
#include "typemapre.h"
#include <google/protobuf/util/json_util.h>

#include <stdio.h>
// c++ protobuf 
template <typename Proto>
std::string pro2string(Proto & any) {
  std::string value;
  google::protobuf::util::MessageToJsonString(any, &value);
  return value;
}

template <typename Proto>
std::vector<std::shared_ptr<yijian::buffer> > yijianBuffer(Proto && any) {

  //std::string value;
  //google::protobuf::util::MessageToJsonString(any, &value);
  YILOG_TRACE("func: {}, any: {}", __func__, pro2string(any));
  static std::size_t maxlength = static_cast<int32_t>(Message_Type::message)
      - PADDING_LENGTH;
  std::vector<std::shared_ptr<yijian::buffer> > vec;
  std::string data_string = any.SerializeAsString();
  int i = 0;
  std::size_t pos = 0;
  std::size_t length = 0;
  do {
    if (maxlength * (i + 1) > data_string.size()) {
      length = data_string.size();
    }else {
      length = maxlength;
    }
    auto subContent = data_string.substr(pos, length);
    vec.push_back(yijian::buffer::Buffer(dispatchType(any), subContent));
    ++i;
    pos += length;
  }while(pos < data_string.size());
  return vec;
}

#endif /* buffer_yi_util_hpp */
