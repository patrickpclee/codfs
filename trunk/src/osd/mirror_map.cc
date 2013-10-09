#include "storagemodule.hh"
#include <boost/lexical_cast.hpp>

void StorageModule::initializeDb() {
    /* Create SQL statement */
    int rc = 0;
    char *zErrMsg = 0;
    string sql;
    sql = "CREATE TABLE IF NOT EXISTS deltaIdMap ("  \
          "key TEXT NOT NULL, "\
          "deltaId INT NOT NULL);";
    rc += sqlite3_exec(_db, sql.c_str(), 0, 0, &zErrMsg);
    sql = "CREATE TABLE IF NOT EXISTS deltaOffsetLength ("  \
            "key TEXT NOT NULL, "\
            "offset INT NOT NULL, "\
            "length INT NOT NULL);";
    rc += sqlite3_exec(_db, sql.c_str(), 0, 0, &zErrMsg);
    sql = "CREATE TABLE IF NOT EXISTS deltaLocationMap ("  \
             "key TEXT NOT NULL, "\
             "deltaId INT NOT NULL, "\
             "blockId INT NOT NULL, "\
             "isReserveSpace INT NOT NULL, "\
             "offset INT NOT NULL, "\
             "length INT NOT NULL);";
    rc += sqlite3_exec(_db, sql.c_str(), 0, 0, &zErrMsg);
    sql = "CREATE TABLE IF NOT EXISTS reserveSpaceMap ("  \
             "key TEXT NOT NULL, "\
             "remainingReserveSpace INT NOT NULL, "\
             "currentOffset INT NOT NULL, "\
             "blockSize INT NOT NULL);";
    rc += sqlite3_exec(_db, sql.c_str(), 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        debug_error("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(-1);
    }
}

static int restoreDeltaIdMap(void *ptr, int argc, char **argv, char **azColName) {
    ConcurrentMap<string, uint32_t>* deltaIdMap = static_cast<ConcurrentMap<string, uint32_t>*>(ptr);
    if (argc != 2) {
       debug_error("Restore Error argc = %d\n", argc);
       exit (-1);
    }
    deltaIdMap->set(argv[0], boost::lexical_cast<uint32_t>(argv[1]));
    return 0;
}
static int restoreDeltaOffsetLengthMap(void *ptr, int argc, char **argv, char **azColName) {
    ConcurrentMap<string, vector<offset_length_t>>* deltaOffsetLengthMap = static_cast<ConcurrentMap<string, vector<offset_length_t>>*>(ptr);
    if (argc != 3) {
       debug_error("Restore Error argc = %d\n", argc);
       exit (-1);
    }
    string key (argv[0]);
    uint32_t offset = boost::lexical_cast<uint32_t>(argv[1]);
    uint32_t length = boost::lexical_cast<uint32_t>(argv[2]);
    if (deltaOffsetLengthMap->count(key)) {
        deltaOffsetLengthMap->get(key).push_back(make_pair(offset,length));
    } else {
        std::vector<offset_length_t> v = {make_pair(offset,length)};
        deltaOffsetLengthMap->set(key,v);
    }
    return 0;
}
static int restoreDeltaLocationMap(void *ptr, int argc, char **argv, char **azColName) {
    ConcurrentMap<string, vector<DeltaLocation>>* deltaLocationMap = static_cast<ConcurrentMap<string, vector<DeltaLocation>>*>(ptr);
    if (argc != 6) {
       debug_error("Restore Error argc = %d\n", argc);
       exit (-1);
    }
    string key (argv[0]);
    DeltaLocation deltaLocation;
    deltaLocation.blockId = boost::lexical_cast<uint32_t>(argv[1]);
    deltaLocation.deltaId = boost::lexical_cast<uint32_t>(argv[2]);
    deltaLocation.isReserveSpace = boost::lexical_cast<bool>(argv[3]);
    deltaLocation.offsetLength = make_pair(boost::lexical_cast<uint32_t>(argv[4]),
                                            boost::lexical_cast<uint32_t>(argv[5]));
    if (deltaLocationMap->count(key)) {
        deltaLocationMap->get(key).push_back(deltaLocation);
    } else {
        std::vector<DeltaLocation> v = { deltaLocation };
        deltaLocationMap->set(key, v);
    }
    return 0;
}
static int restoreReserveSpaceMap(void *ptr, int argc, char **argv, char **azColName) {
    ConcurrentMap<string, ReserveSpaceInfo>* reserveSpaceInfoMap = static_cast<ConcurrentMap<string, ReserveSpaceInfo>*>(ptr);
    if (argc != 4) {
       debug_error("Restore Error argc = %d\n", argc);
       exit (-1);
    }
    string key (argv[0]);
    ReserveSpaceInfo reserveSpaceInfo;
    reserveSpaceInfo.blockSize = boost::lexical_cast<uint32_t>(argv[1]);
    reserveSpaceInfo.currentOffset = boost::lexical_cast<uint32_t>(argv[2]);
    reserveSpaceInfo.remainingReserveSpace = boost::lexical_cast<uint32_t>(argv[3]);
    reserveSpaceInfoMap->set(key, reserveSpaceInfo);
    return 0;
}

void StorageModule::restoreFromDb() {
    int rc = 0;
    char *zErrMsg = 0;
    string sql;
    sql = "SELECT key, deltaId FROM deltaIdMap;";
    rc += sqlite3_exec(_db, sql.c_str(), restoreDeltaIdMap, &_deltaIdMap, &zErrMsg);
    sql = "SELECT key, offset, length FROM deltaOffsetLength;";
    rc += sqlite3_exec(_db, sql.c_str(), restoreDeltaOffsetLengthMap, &_deltaOffsetLength, &zErrMsg);
    sql = "SELECT key, deltaId, blockId, isReserveSpace, offset, length FROM deltaLocationMap;";
    rc += sqlite3_exec(_db, sql.c_str(), restoreDeltaLocationMap, &_deltaLocationMap, &zErrMsg);
    sql = "SELECT key, remainingReserveSpace, currentOffset, blockSize FROM reserveSpaceMap;";
    rc += sqlite3_exec(_db, sql.c_str(), restoreReserveSpaceMap, &_reserveSpaceMap, &zErrMsg);
    if (rc != SQLITE_OK) {
        debug_error("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(-1);
    }
}
