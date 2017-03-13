mongo chatdb
use chatdb

// user 
db.user.createIndex({"phoneNo": 1, "countryCode": 1}, {"unique": true})
db.user.createIndex({"nickname": 1})
// user unread
db.userUnread.createIndex({"userID": 1, "toNodeID": 1}, {"unique": true})
// add firend track
db.addFriend.createIndex({"littleIDBigID": 1}, {"unique": true})
db.addFriend.createIndex({"timestamp": 1})
// login track
db.userLogin.createIndex({"userID": 1})
// connect track
db.userConnect.createIndex({"userID": 1})
// message
db.nodeMessage.createIndex({"toNodeID": 1, "incrementID": 1}, {"unique": true})
// message  track id
db.messageNodeCount.createIndex({"nodeID": 1}, {"unique": true})
// connectInfo
db.connectInfo.createIndex({"UUID": 1}, {"unique": true})
//db.connectInfo.createIndex({"toNodeIDs": 1, "serverName": 1})
db.connectInfo.createIndex({"userID": 1, "serverName": 1, "toNodeIDs": 1})
// media
db.media.createIndex({"md5": 1}, {"unique": true})

