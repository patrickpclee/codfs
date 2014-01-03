/** Switch Database **/
db = connect("localhost:27017/ncvfs");

/** Configuration indexes **/
db.getCollection("Configuration").ensureIndex({
  "_id": NumberInt(1)
},[
  
]);

/** system.users indexes **/
db.getCollection("system.users").ensureIndex({
  "_id": NumberInt(1)
},[
  
]);

/** system.users indexes **/
db.getCollection("system.users").ensureIndex({
  "user": NumberInt(1),
  "userSource": NumberInt(1)
},{
  "unique": true
});

/** Configuration records **/
db.getCollection("Configuration").insert({
  "_id": ObjectId("5024cb666803fa6077000000"),
  "fileId": NumberLong(1),
  "id": "config",
  "objectId": NumberLong(1),
  "version": NumberLong(8589934594)
});

/** system.users records **/
db.getCollection("system.users").insert({
  "_id": ObjectId("5024afc43ecbcc2346b08fad"),
  "user": "ncvfs",
  "readOnly": false,
  "pwd": "cf8b7fa0b1bcd277da8217f04f498bd0"
});
