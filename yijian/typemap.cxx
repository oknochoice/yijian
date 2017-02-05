#include "typemap.h"
#include <unordered_map>

static std::unordered_map<int32_t, bool> 
check_map_ = {
  {ChatType::clientdisconnect, true},
  {ChatType::logout, true},
  {ChatType::queryuser, true},
  {ChatType::queryuserversion, true},
  {ChatType::querynode, true},
  {ChatType::addfriend, true},
  {ChatType::addfriendauthorize, true},
  {ChatType::creategroup, true},
  {ChatType::groupaddmember, true},
  {ChatType::nodemessage, true},
  {ChatType::querymessage, true},
  {ChatType::media, true},
  {ChatType::querymedia, true},
  {ChatType::mediacheck, true},
};
bool missing_check(int32_t type) {
  return check_map_.find(type) != check_map_.end();
}

