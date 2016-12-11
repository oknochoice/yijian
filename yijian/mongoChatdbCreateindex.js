mongo chatdb
use chatdb

db.user.createIndex({"code_phone": 1}, {"unique": true})

db.nodeMessage.createIndex({"toNodeID": 1, "incrementID": 1}, {"unique": true})

db.messageNodeCount.createIndex({"nodeID": 1}, {"unique": true})

db.connectInfo.createIndex({"UUID": 1}, {"unique": true})
db.connectInfo.createIndex({"toNodeIDs": 1, "serverName": 1})
db.connectInfo.createIndex({"userID": 1, "serverName": 1, "toNodeIDs": 1})

db.media.createIndex({"sha1": 1}, {"unique": true})

