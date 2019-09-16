#ifndef _ROSCAR_CAR_CLI_CLI
#define _ROSCAR_CAR_CLI_CLI

#include <string>
#include <vector>
#include "udsClient.h"

namespace roscar
{
namespace car
{
namespace cli
{

class Cli
{
public:
    typedef bool (*FUNC_SEND_SIGNALING)(std::string &sig);

    Cli(FUNC_SEND_SIGNALING func) : sendSig(func){};
    virtual ~Cli() = default;

    /**
     * @brief process new input command
     * 
     * @param cmd: input command
     * @return true: waiting for next command.
     * @return false: stop process.
     */
    bool onCmd(std::vector<std::string> &cmd);

    /**
     * @brief callback function, for new signaling has been received
     * 
     * @param buffer: UDSClient buffer object
     * @param sig: received signaling object
     * @return true: continue working
     * @return false: stop working
     */
    bool onSignaling(UDSClient::BufType &buffer, rapidjson::Document &sig);

protected:
    FUNC_SEND_SIGNALING sendSig;

    void sendRequest(std::string &req);
    bool onCmd_Info(std::vector<std::string> &cmd);
    bool onSigInfoResp(rapidjson::Document &sig);
    bool onCmd_Move(std::vector<std::string> &cmd);
    bool onSigMoveResp(rapidjson::Document &sig);
    bool onCmd_Video(std::vector<std::string> &cmd);
    bool onSigVideoResp(rapidjson::Document &sig);
};

} // namespace cli
} // namespace car
} // namespace roscar

#endif // _ROSCAR_CAR_CLI_CLI
