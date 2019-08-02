#include "interface.h"

#include "rapidjson/writer.h"

#include "roscar_common/rcmp.h"
#include "pilot/Info.h"

using namespace std;
using namespace rapidjson;
using namespace roscar::car::roscar_common;

namespace roscar
{
namespace car
{
namespace interface
{

std::map<const char *, Interface::FUNC_ONSIG> Interface::gSigFuncMap;
ros::ServiceClient Interface::gSvrInfo;

void Interface::init(ros::NodeHandle &nh)
{
    gSigFuncMap[RCMP::SIG_PING] = &onSigPing;
    gSigFuncMap[RCMP::SIG_PONG] = &onSigPong;
    gSigFuncMap[RCMP::SIG_INFO] = &onSigInfo;

    gSvrInfo = nh.serviceClient<pilot::Info>("info");
}

bool Interface::onSignaling(UDS::SESSION_t &sess, rapidjson::Document &sig)
{
    const char *cmd = sig[RCMP::FIELD_CMD].GetString();

    auto onSigFuncPare = gSigFuncMap.find(cmd);
    if (onSigFuncPare == gSigFuncMap.end())
    {
        // unsupported signaling
        ROS_ERROR("Err: unsupported signaling(cmd: [%s])", cmd);
        return false;
    }

    auto onSigFunc = onSigFuncPare->second;
    return onSigFunc(sess, sig);
}

bool Interface::onSigPing(UDS::SESSION_t &sess, rapidjson::Document &sig)
{
    return sendSignaling(sess, RCMP::convertToResp(sig));
}

bool Interface::onSigPong(UDS::SESSION_t &sess, rapidjson::Document &sig)
{
    ROS_DEBUG("Debug: recv PONG.");
    return true;
}

bool Interface::onSigInfo(UDS::SESSION_t &sess, rapidjson::Document &sig)
{
    pilot::Info info;
    if (!gSvrInfo.call(info))
    {
        ROS_ERROR("Fail to call service: [pilot::Info]");
        return false;
    }

    auto &alloc = sig.GetAllocator();
    Value payload(kObjectType);
    payload.AddMember(Value::StringRefType(RCMP::FIELD_INFORESP_ID),
                      Value::StringRefType(info.response.id.c_str()),
                      alloc);
    payload.AddMember(Value::StringRefType(RCMP::FIELD_INFORESP_TYPE),
                      Value::StringRefType(info.response.type.c_str()),
                      alloc);
    payload.AddMember(Value::StringRefType(RCMP::FIELD_INFORESP_NAME),
                      Value::StringRefType(info.response.name.c_str()),
                      alloc);

    return sendSignaling(sess, RCMP::convertToResp(sig, &payload));
}

bool Interface::sendSignaling(UDS::SESSION_t &sess, rapidjson::Document &sig)
{
    // convert signaling object into jsong string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    sig.Accept(writer);

    // check empty send buffer size
    auto len = buffer.GetSize();
    if (RCMP::RCMP_MAXPAYLOAD - sess.sendBufEnd < len)
    {
        if (sess.sendBufPos == 0 ||
            (RCMP::RCMP_MAXPAYLOAD - sess.sendBufEnd + sess.sendBufPos < len))
        {
            // send buffer full
            ROS_DEBUG("Debug: send buffer fulll(soc: [%d]", sess.soc);
            return false;
        }

        // defrag buffer
        int bufLen = sess.sendBufEnd - sess.sendBufPos;
        memmove(sess.sendBuf, sess.sendBuf + sess.sendBufPos, bufLen);
        sess.sendBufPos = 0;
        sess.sendBufEnd = bufLen;
    }

    memcpy(sess.sendBuf + sess.sendBufEnd, buffer.GetString(), len);
    sess.sendBufEnd += len;

    return true;
}

} // namespace interface
} // namespace car
} // namespace roscar
