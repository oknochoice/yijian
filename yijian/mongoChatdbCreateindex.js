mongo chatdb
use chatdb
db.user.createIndex({"code_phone": 1}, {"unique": true})
db.messageNodeCount.createIndex({"nodeID": 1}, {"unique": true})
db.nodeMessage.createIndex({"toNodeID": 1, "incrementID": 1}, {"unique": true})
db.connectInfo.createIndex({"toNodeID": 1, "serverName": 1})
db.connectInfo.createIndex({"userID": 1, "serverName": 1, "toNodeID": 1})
db.connectInfo.createIndex({"UUID": 1}, {"unique": true})
