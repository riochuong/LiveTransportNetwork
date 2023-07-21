#include <boost/bimap.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include "logging.h"
#include "network-monitor/stomp-frame.h"

using namespace NetworkMonitor;

// utility function to generate boost::bimap
template<typename L, typename R>
static boost::bimap<L, R> MakeBimap(
    std::initializer_list<typename boost::bimap<L,R>::value_type> list
){
      return boost::bimap<L, R>(list.begin(), list.end());
}

static const auto gStompComamndStrings {
    MakeBimap<StompCommand, std::string_view>(
        {
            {StompCommand::kAbort      , "ABORT"      },
            {StompCommand::kAck        , "ACK"        },
            {StompCommand::kBegin      , "BEGIN"      },
            {StompCommand::kCommit     , "COMMIT"     },
            {StompCommand::kConnect    , "CONNECT"    },
            {StompCommand::kConnected  , "CONNECTED"  },
            {StompCommand::kDisconnect , "DISCONNECT" },
            {StompCommand::kError      , "ERROR"      },
            {StompCommand::kMessage    , "MESSAGE"    },
            {StompCommand::kNack       , "NACK"       },
            {StompCommand::kReceipt    , "RECEIPT"    },
            {StompCommand::kSend       , "SEND"       },
            {StompCommand::kStomp      , "STOMP"      },
            {StompCommand::kSubscribe  , "SUBSCRIBE"  },
            {StompCommand::kUnsubscribe, "UNSUBSCRIBE"},
        }
    )
};

static const auto gStompHeaderStrings {
    MakeBimap<StompHeader, std::string_view>(
        {
            {StompHeader::kInvalid, "invalid"},
            {StompHeader::kAcceptVersion, "accept-version"},
            {StompHeader::kAck, "ack"},
            {StompHeader::kContentLength, "content-length"},
            {StompHeader::kContentType, "content-type"},
            {StompHeader::kDestination, "destination"},
            {StompHeader::kHeartBeat, "heart-beat"},
            {StompHeader::kHost, "host"},
            {StompHeader::kId, "id"},
            {StompHeader::kLogin, "login"},
            {StompHeader::kMessage, "message"},
            {StompHeader::kMessageId, "message-id"},
            {StompHeader::kPasscode, "passcode"},
            {StompHeader::kReceipt, "receipt"},
            {StompHeader::kReceiptId, "receipt-id"},
            {StompHeader::kSession, "session"},
            {StompHeader::kSubscription, "subscription"},
            {StompHeader::kTransaction, "transaction"},
            {StompHeader::kServer, "server"},
            {StompHeader::kVersion, "version"},
        }
    )
};

std::ostream &NetworkMonitor::operator<<(std::ostream &os, const StompCommand &command){
    auto it = gStompComamndStrings.left.find(command);
    if (it == gStompComamndStrings.left.end()){
        os << "StompCommand::kInvalid";
    }
    os << it->second;
    return os;
}

std::ostream &NetworkMonitor::operator<<(std::ostream &os, const StompHeader &header){
    auto it = gStompHeaderStrings.left.find(header);
    if(it == gStompHeaderStrings.left.end()){
        os << "StompHeader::kInvalid";
    }
    os << it->second;
    return os;
}

std::string ToString(const StompCommand &command){
   return std::string(gStompComamndStrings.left.find(command)->second);
}

StompCommand ToCommand(const std::string_view command){
    auto it = gStompComamndStrings.right.find(command);
    if (it == gStompComamndStrings.right.end()){
        return StompCommand::kInvalid;
    }
    return it->second;
}

// main parser here
StompFrame::StompFrame(StompError& ec, const std::string& frame){
   
    bool found_command = false;
    bool headers_valid = true;
    bool found_body = false;
    ec = StompError::kOk;

    std::istringstream sstream {frame};
    for (std::string line; std::getline(sstream, line); ){
        boost::trim(line);
        if (!found_command){
            auto it = gStompComamndStrings.right.find(line);
            if (it == gStompComamndStrings.right.end()){
                logger::error("Invalid stomp command {}. Stop parsing !", line);
                ec = StompError::kInvalidCommand;
                break;
            }
            command_ = {it->second};
            found_command = true;
        } else if (line.empty() || found_body){
            found_body = true;
            body_.append(line);
            body_.append("\n");

        } else { // this should be headers here
            std::size_t index = line.find(":");            
            if (index == std::string::npos){
                logger::error("Invallid StompHeader format {}", line);
                ec = StompError::kInvalidHeaderFormat;
                headers_valid = false;
                break;
            }
            std::string key = line.substr(0, index);
            // empty value
            if ((index + 1) == line.size()){
                ec = StompError::kInvalidEmptyValueHeader;
                logger::error("Invalid empty value for line {}", line);
                headers_valid = false;
                break;
            }
            std::string value = line.substr(index + 1);
            if (key.empty()){
                ec = StompError::kInvalidEmptyKeyHeader;
                logger::error("Invalid empty key for line {}", line);
                headers_valid = false;
                break;
            }
            auto it = gStompHeaderStrings.right.find(key);
            if (it == gStompHeaderStrings.right.end()){
                ec = StompError::kInvalidHeaderKey;
                logger::error("Header value {} is not an expected value from the predefined list", key);
                headers_valid = false;
            }
            headers_.emplace(it->second, value);
        }

        // reset everything if there is error during parsing 
        if (!found_body || !headers_valid || !found_command){
            command_ = StompCommand::kInvalid;
            headers_.clear();
            body_ = {};
        }
    }
    


}

StompFrame::StompFrame(const StompFrame& other) {
    command_ = {other.command_};
    for (auto it = other.headers_.begin(); it != other.headers_.end(); it++){
        headers_.emplace(it->first, it->second);
    }
    body_ = {other.body_};
}

StompFrame::StompFrame(StompFrame&& other){
    command_ = std::move(other.command_);
    headers_ = std::move(other.headers_);
    body_ = std::move(other.body_);
}

StompFrame &StompFrame::operator=(const StompFrame& other){
    command_ = std::move(other.command_);
    headers_ = std::move(other.headers_);
    body_ = std::move(other.body_);    
    return *this;
}


StompFrame &StompFrame::operator=(StompFrame&& other){
    command_ = std::move(other.command_);
    headers_ = std::move(other.headers_);
    body_ = std::move(other.body_);
    return *this;
}