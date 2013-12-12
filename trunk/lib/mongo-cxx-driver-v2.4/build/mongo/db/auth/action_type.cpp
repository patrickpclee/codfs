// AUTO-GENERATED FILE DO NOT EDIT
// See src/mongo/db/auth/generate_action_types.py
/*    Copyright 2012 10gen Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "mongo/pch.h"

#include "mongo/db/auth/action_type.h"

#include <iostream>
#include <string>

#include "mongo/base/status.h"
#include "mongo/platform/cstdint.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {

    const ActionType ActionType::addShard(addShardValue);
    const ActionType ActionType::captrunc(captruncValue);
    const ActionType ActionType::clean(cleanValue);
    const ActionType ActionType::clone(cloneValue);
    const ActionType ActionType::cloneCollectionLocalSource(cloneCollectionLocalSourceValue);
    const ActionType ActionType::cloneCollectionTarget(cloneCollectionTargetValue);
    const ActionType ActionType::closeAllDatabases(closeAllDatabasesValue);
    const ActionType ActionType::collMod(collModValue);
    const ActionType ActionType::collStats(collStatsValue);
    const ActionType ActionType::compact(compactValue);
    const ActionType ActionType::connPoolStats(connPoolStatsValue);
    const ActionType ActionType::connPoolSync(connPoolSyncValue);
    const ActionType ActionType::convertToCapped(convertToCappedValue);
    const ActionType ActionType::copyDBTarget(copyDBTargetValue);
    const ActionType ActionType::cpuProfiler(cpuProfilerValue);
    const ActionType ActionType::createCollection(createCollectionValue);
    const ActionType ActionType::cursorInfo(cursorInfoValue);
    const ActionType ActionType::dbHash(dbHashValue);
    const ActionType ActionType::dbStats(dbStatsValue);
    const ActionType ActionType::diagLogging(diagLoggingValue);
    const ActionType ActionType::dropCollection(dropCollectionValue);
    const ActionType ActionType::dropDatabase(dropDatabaseValue);
    const ActionType ActionType::dropIndexes(dropIndexesValue);
    const ActionType ActionType::emptycapped(emptycappedValue);
    const ActionType ActionType::enableSharding(enableShardingValue);
    const ActionType ActionType::ensureIndex(ensureIndexValue);
    const ActionType ActionType::find(findValue);
    const ActionType ActionType::flushRouterConfig(flushRouterConfigValue);
    const ActionType ActionType::fsync(fsyncValue);
    const ActionType ActionType::getCmdLineOpts(getCmdLineOptsValue);
    const ActionType ActionType::getLog(getLogValue);
    const ActionType ActionType::getParameter(getParameterValue);
    const ActionType ActionType::getShardMap(getShardMapValue);
    const ActionType ActionType::getShardVersion(getShardVersionValue);
    const ActionType ActionType::handshake(handshakeValue);
    const ActionType ActionType::hostInfo(hostInfoValue);
    const ActionType ActionType::indexRead(indexReadValue);
    const ActionType ActionType::indexStats(indexStatsValue);
    const ActionType ActionType::inprog(inprogValue);
    const ActionType ActionType::insert(insertValue);
    const ActionType ActionType::killCursors(killCursorsValue);
    const ActionType ActionType::killop(killopValue);
    const ActionType ActionType::listDatabases(listDatabasesValue);
    const ActionType ActionType::listShards(listShardsValue);
    const ActionType ActionType::logRotate(logRotateValue);
    const ActionType ActionType::mapReduceShardedFinish(mapReduceShardedFinishValue);
    const ActionType ActionType::moveChunk(moveChunkValue);
    const ActionType ActionType::movePrimary(movePrimaryValue);
    const ActionType ActionType::netstat(netstatValue);
    const ActionType ActionType::profileEnable(profileEnableValue);
    const ActionType ActionType::profileRead(profileReadValue);
    const ActionType ActionType::reIndex(reIndexValue);
    const ActionType ActionType::remove(removeValue);
    const ActionType ActionType::removeShard(removeShardValue);
    const ActionType ActionType::renameCollectionSameDB(renameCollectionSameDBValue);
    const ActionType ActionType::repairDatabase(repairDatabaseValue);
    const ActionType ActionType::replSetElect(replSetElectValue);
    const ActionType ActionType::replSetFreeze(replSetFreezeValue);
    const ActionType ActionType::replSetFresh(replSetFreshValue);
    const ActionType ActionType::replSetGetRBID(replSetGetRBIDValue);
    const ActionType ActionType::replSetGetStatus(replSetGetStatusValue);
    const ActionType ActionType::replSetHeartbeat(replSetHeartbeatValue);
    const ActionType ActionType::replSetInitiate(replSetInitiateValue);
    const ActionType ActionType::replSetMaintenance(replSetMaintenanceValue);
    const ActionType ActionType::replSetReconfig(replSetReconfigValue);
    const ActionType ActionType::replSetStepDown(replSetStepDownValue);
    const ActionType ActionType::replSetSyncFrom(replSetSyncFromValue);
    const ActionType ActionType::resync(resyncValue);
    const ActionType ActionType::serverStatus(serverStatusValue);
    const ActionType ActionType::setParameter(setParameterValue);
    const ActionType ActionType::setShardVersion(setShardVersionValue);
    const ActionType ActionType::shardCollection(shardCollectionValue);
    const ActionType ActionType::shardingState(shardingStateValue);
    const ActionType ActionType::shutdown(shutdownValue);
    const ActionType ActionType::split(splitValue);
    const ActionType ActionType::splitChunk(splitChunkValue);
    const ActionType ActionType::splitVector(splitVectorValue);
    const ActionType ActionType::storageDetails(storageDetailsValue);
    const ActionType ActionType::top(topValue);
    const ActionType ActionType::touch(touchValue);
    const ActionType ActionType::unlock(unlockValue);
    const ActionType ActionType::unsetSharding(unsetShardingValue);
    const ActionType ActionType::update(updateValue);
    const ActionType ActionType::userAdmin(userAdminValue);
    const ActionType ActionType::validate(validateValue);
    const ActionType ActionType::writebacklisten(writebacklistenValue);
    const ActionType ActionType::writeBacksQueued(writeBacksQueuedValue);
    const ActionType ActionType::_migrateClone(_migrateCloneValue);
    const ActionType ActionType::_recvChunkAbort(_recvChunkAbortValue);
    const ActionType ActionType::_recvChunkCommit(_recvChunkCommitValue);
    const ActionType ActionType::_recvChunkStart(_recvChunkStartValue);
    const ActionType ActionType::_recvChunkStatus(_recvChunkStatusValue);
    const ActionType ActionType::_transferMods(_transferModsValue);

    bool ActionType::operator==(const ActionType& rhs) const {
        return _identifier == rhs._identifier;
    }

    std::ostream& operator<<(std::ostream& os, const ActionType& at) {
        os << ActionType::actionToString(at);
        return os;
    }

    std::string ActionType::toString() const {
        return actionToString(*this);
    }

    Status ActionType::parseActionFromString(const std::string& action, ActionType* result) {
        if (action == "addShard") {
            *result = addShard;
            return Status::OK();
        }
        if (action == "captrunc") {
            *result = captrunc;
            return Status::OK();
        }
        if (action == "clean") {
            *result = clean;
            return Status::OK();
        }
        if (action == "clone") {
            *result = clone;
            return Status::OK();
        }
        if (action == "cloneCollectionLocalSource") {
            *result = cloneCollectionLocalSource;
            return Status::OK();
        }
        if (action == "cloneCollectionTarget") {
            *result = cloneCollectionTarget;
            return Status::OK();
        }
        if (action == "closeAllDatabases") {
            *result = closeAllDatabases;
            return Status::OK();
        }
        if (action == "collMod") {
            *result = collMod;
            return Status::OK();
        }
        if (action == "collStats") {
            *result = collStats;
            return Status::OK();
        }
        if (action == "compact") {
            *result = compact;
            return Status::OK();
        }
        if (action == "connPoolStats") {
            *result = connPoolStats;
            return Status::OK();
        }
        if (action == "connPoolSync") {
            *result = connPoolSync;
            return Status::OK();
        }
        if (action == "convertToCapped") {
            *result = convertToCapped;
            return Status::OK();
        }
        if (action == "copyDBTarget") {
            *result = copyDBTarget;
            return Status::OK();
        }
        if (action == "cpuProfiler") {
            *result = cpuProfiler;
            return Status::OK();
        }
        if (action == "createCollection") {
            *result = createCollection;
            return Status::OK();
        }
        if (action == "cursorInfo") {
            *result = cursorInfo;
            return Status::OK();
        }
        if (action == "dbHash") {
            *result = dbHash;
            return Status::OK();
        }
        if (action == "dbStats") {
            *result = dbStats;
            return Status::OK();
        }
        if (action == "diagLogging") {
            *result = diagLogging;
            return Status::OK();
        }
        if (action == "dropCollection") {
            *result = dropCollection;
            return Status::OK();
        }
        if (action == "dropDatabase") {
            *result = dropDatabase;
            return Status::OK();
        }
        if (action == "dropIndexes") {
            *result = dropIndexes;
            return Status::OK();
        }
        if (action == "emptycapped") {
            *result = emptycapped;
            return Status::OK();
        }
        if (action == "enableSharding") {
            *result = enableSharding;
            return Status::OK();
        }
        if (action == "ensureIndex") {
            *result = ensureIndex;
            return Status::OK();
        }
        if (action == "find") {
            *result = find;
            return Status::OK();
        }
        if (action == "flushRouterConfig") {
            *result = flushRouterConfig;
            return Status::OK();
        }
        if (action == "fsync") {
            *result = fsync;
            return Status::OK();
        }
        if (action == "getCmdLineOpts") {
            *result = getCmdLineOpts;
            return Status::OK();
        }
        if (action == "getLog") {
            *result = getLog;
            return Status::OK();
        }
        if (action == "getParameter") {
            *result = getParameter;
            return Status::OK();
        }
        if (action == "getShardMap") {
            *result = getShardMap;
            return Status::OK();
        }
        if (action == "getShardVersion") {
            *result = getShardVersion;
            return Status::OK();
        }
        if (action == "handshake") {
            *result = handshake;
            return Status::OK();
        }
        if (action == "hostInfo") {
            *result = hostInfo;
            return Status::OK();
        }
        if (action == "indexRead") {
            *result = indexRead;
            return Status::OK();
        }
        if (action == "indexStats") {
            *result = indexStats;
            return Status::OK();
        }
        if (action == "inprog") {
            *result = inprog;
            return Status::OK();
        }
        if (action == "insert") {
            *result = insert;
            return Status::OK();
        }
        if (action == "killCursors") {
            *result = killCursors;
            return Status::OK();
        }
        if (action == "killop") {
            *result = killop;
            return Status::OK();
        }
        if (action == "listDatabases") {
            *result = listDatabases;
            return Status::OK();
        }
        if (action == "listShards") {
            *result = listShards;
            return Status::OK();
        }
        if (action == "logRotate") {
            *result = logRotate;
            return Status::OK();
        }
        if (action == "mapReduceShardedFinish") {
            *result = mapReduceShardedFinish;
            return Status::OK();
        }
        if (action == "moveChunk") {
            *result = moveChunk;
            return Status::OK();
        }
        if (action == "movePrimary") {
            *result = movePrimary;
            return Status::OK();
        }
        if (action == "netstat") {
            *result = netstat;
            return Status::OK();
        }
        if (action == "profileEnable") {
            *result = profileEnable;
            return Status::OK();
        }
        if (action == "profileRead") {
            *result = profileRead;
            return Status::OK();
        }
        if (action == "reIndex") {
            *result = reIndex;
            return Status::OK();
        }
        if (action == "remove") {
            *result = remove;
            return Status::OK();
        }
        if (action == "removeShard") {
            *result = removeShard;
            return Status::OK();
        }
        if (action == "renameCollectionSameDB") {
            *result = renameCollectionSameDB;
            return Status::OK();
        }
        if (action == "repairDatabase") {
            *result = repairDatabase;
            return Status::OK();
        }
        if (action == "replSetElect") {
            *result = replSetElect;
            return Status::OK();
        }
        if (action == "replSetFreeze") {
            *result = replSetFreeze;
            return Status::OK();
        }
        if (action == "replSetFresh") {
            *result = replSetFresh;
            return Status::OK();
        }
        if (action == "replSetGetRBID") {
            *result = replSetGetRBID;
            return Status::OK();
        }
        if (action == "replSetGetStatus") {
            *result = replSetGetStatus;
            return Status::OK();
        }
        if (action == "replSetHeartbeat") {
            *result = replSetHeartbeat;
            return Status::OK();
        }
        if (action == "replSetInitiate") {
            *result = replSetInitiate;
            return Status::OK();
        }
        if (action == "replSetMaintenance") {
            *result = replSetMaintenance;
            return Status::OK();
        }
        if (action == "replSetReconfig") {
            *result = replSetReconfig;
            return Status::OK();
        }
        if (action == "replSetStepDown") {
            *result = replSetStepDown;
            return Status::OK();
        }
        if (action == "replSetSyncFrom") {
            *result = replSetSyncFrom;
            return Status::OK();
        }
        if (action == "resync") {
            *result = resync;
            return Status::OK();
        }
        if (action == "serverStatus") {
            *result = serverStatus;
            return Status::OK();
        }
        if (action == "setParameter") {
            *result = setParameter;
            return Status::OK();
        }
        if (action == "setShardVersion") {
            *result = setShardVersion;
            return Status::OK();
        }
        if (action == "shardCollection") {
            *result = shardCollection;
            return Status::OK();
        }
        if (action == "shardingState") {
            *result = shardingState;
            return Status::OK();
        }
        if (action == "shutdown") {
            *result = shutdown;
            return Status::OK();
        }
        if (action == "split") {
            *result = split;
            return Status::OK();
        }
        if (action == "splitChunk") {
            *result = splitChunk;
            return Status::OK();
        }
        if (action == "splitVector") {
            *result = splitVector;
            return Status::OK();
        }
        if (action == "storageDetails") {
            *result = storageDetails;
            return Status::OK();
        }
        if (action == "top") {
            *result = top;
            return Status::OK();
        }
        if (action == "touch") {
            *result = touch;
            return Status::OK();
        }
        if (action == "unlock") {
            *result = unlock;
            return Status::OK();
        }
        if (action == "unsetSharding") {
            *result = unsetSharding;
            return Status::OK();
        }
        if (action == "update") {
            *result = update;
            return Status::OK();
        }
        if (action == "userAdmin") {
            *result = userAdmin;
            return Status::OK();
        }
        if (action == "validate") {
            *result = validate;
            return Status::OK();
        }
        if (action == "writebacklisten") {
            *result = writebacklisten;
            return Status::OK();
        }
        if (action == "writeBacksQueued") {
            *result = writeBacksQueued;
            return Status::OK();
        }
        if (action == "_migrateClone") {
            *result = _migrateClone;
            return Status::OK();
        }
        if (action == "_recvChunkAbort") {
            *result = _recvChunkAbort;
            return Status::OK();
        }
        if (action == "_recvChunkCommit") {
            *result = _recvChunkCommit;
            return Status::OK();
        }
        if (action == "_recvChunkStart") {
            *result = _recvChunkStart;
            return Status::OK();
        }
        if (action == "_recvChunkStatus") {
            *result = _recvChunkStatus;
            return Status::OK();
        }
        if (action == "_transferMods") {
            *result = _transferMods;
            return Status::OK();
        }

        return Status(ErrorCodes::FailedToParse,
                      mongoutils::str::stream() << "Unrecognized action privilege string: "
                                                << action,
                      0);
    }

    // Takes an ActionType and returns the string representation
    std::string ActionType::actionToString(const ActionType& action) {
        switch (action.getIdentifier()) {
        case addShardValue:
            return "addShard";
        case captruncValue:
            return "captrunc";
        case cleanValue:
            return "clean";
        case cloneValue:
            return "clone";
        case cloneCollectionLocalSourceValue:
            return "cloneCollectionLocalSource";
        case cloneCollectionTargetValue:
            return "cloneCollectionTarget";
        case closeAllDatabasesValue:
            return "closeAllDatabases";
        case collModValue:
            return "collMod";
        case collStatsValue:
            return "collStats";
        case compactValue:
            return "compact";
        case connPoolStatsValue:
            return "connPoolStats";
        case connPoolSyncValue:
            return "connPoolSync";
        case convertToCappedValue:
            return "convertToCapped";
        case copyDBTargetValue:
            return "copyDBTarget";
        case cpuProfilerValue:
            return "cpuProfiler";
        case createCollectionValue:
            return "createCollection";
        case cursorInfoValue:
            return "cursorInfo";
        case dbHashValue:
            return "dbHash";
        case dbStatsValue:
            return "dbStats";
        case diagLoggingValue:
            return "diagLogging";
        case dropCollectionValue:
            return "dropCollection";
        case dropDatabaseValue:
            return "dropDatabase";
        case dropIndexesValue:
            return "dropIndexes";
        case emptycappedValue:
            return "emptycapped";
        case enableShardingValue:
            return "enableSharding";
        case ensureIndexValue:
            return "ensureIndex";
        case findValue:
            return "find";
        case flushRouterConfigValue:
            return "flushRouterConfig";
        case fsyncValue:
            return "fsync";
        case getCmdLineOptsValue:
            return "getCmdLineOpts";
        case getLogValue:
            return "getLog";
        case getParameterValue:
            return "getParameter";
        case getShardMapValue:
            return "getShardMap";
        case getShardVersionValue:
            return "getShardVersion";
        case handshakeValue:
            return "handshake";
        case hostInfoValue:
            return "hostInfo";
        case indexReadValue:
            return "indexRead";
        case indexStatsValue:
            return "indexStats";
        case inprogValue:
            return "inprog";
        case insertValue:
            return "insert";
        case killCursorsValue:
            return "killCursors";
        case killopValue:
            return "killop";
        case listDatabasesValue:
            return "listDatabases";
        case listShardsValue:
            return "listShards";
        case logRotateValue:
            return "logRotate";
        case mapReduceShardedFinishValue:
            return "mapReduceShardedFinish";
        case moveChunkValue:
            return "moveChunk";
        case movePrimaryValue:
            return "movePrimary";
        case netstatValue:
            return "netstat";
        case profileEnableValue:
            return "profileEnable";
        case profileReadValue:
            return "profileRead";
        case reIndexValue:
            return "reIndex";
        case removeValue:
            return "remove";
        case removeShardValue:
            return "removeShard";
        case renameCollectionSameDBValue:
            return "renameCollectionSameDB";
        case repairDatabaseValue:
            return "repairDatabase";
        case replSetElectValue:
            return "replSetElect";
        case replSetFreezeValue:
            return "replSetFreeze";
        case replSetFreshValue:
            return "replSetFresh";
        case replSetGetRBIDValue:
            return "replSetGetRBID";
        case replSetGetStatusValue:
            return "replSetGetStatus";
        case replSetHeartbeatValue:
            return "replSetHeartbeat";
        case replSetInitiateValue:
            return "replSetInitiate";
        case replSetMaintenanceValue:
            return "replSetMaintenance";
        case replSetReconfigValue:
            return "replSetReconfig";
        case replSetStepDownValue:
            return "replSetStepDown";
        case replSetSyncFromValue:
            return "replSetSyncFrom";
        case resyncValue:
            return "resync";
        case serverStatusValue:
            return "serverStatus";
        case setParameterValue:
            return "setParameter";
        case setShardVersionValue:
            return "setShardVersion";
        case shardCollectionValue:
            return "shardCollection";
        case shardingStateValue:
            return "shardingState";
        case shutdownValue:
            return "shutdown";
        case splitValue:
            return "split";
        case splitChunkValue:
            return "splitChunk";
        case splitVectorValue:
            return "splitVector";
        case storageDetailsValue:
            return "storageDetails";
        case topValue:
            return "top";
        case touchValue:
            return "touch";
        case unlockValue:
            return "unlock";
        case unsetShardingValue:
            return "unsetSharding";
        case updateValue:
            return "update";
        case userAdminValue:
            return "userAdmin";
        case validateValue:
            return "validate";
        case writebacklistenValue:
            return "writebacklisten";
        case writeBacksQueuedValue:
            return "writeBacksQueued";
        case _migrateCloneValue:
            return "_migrateClone";
        case _recvChunkAbortValue:
            return "_recvChunkAbort";
        case _recvChunkCommitValue:
            return "_recvChunkCommit";
        case _recvChunkStartValue:
            return "_recvChunkStart";
        case _recvChunkStatusValue:
            return "_recvChunkStatus";
        case _transferModsValue:
            return "_transferMods";
        default:
            return "";
        }
    }

} // namespace mongo
