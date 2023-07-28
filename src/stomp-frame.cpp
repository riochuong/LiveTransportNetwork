#include <boost/bimap.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <utility>
#include <string_view>
#include <charconv>

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

static const auto gStompErrorStrings {
    MakeBimap<StompError, std::string_view>(
        {
            {StompError::kOk,                              "Ok"},
            {StompError::kInvalidFormat,                   "Invalid Format"},
            {StompError::kInvalidHeaderFormat,             "Invalid Header Format"},
            {StompError::kInvalidCommand,                  "Invalid Command"},
            {StompError::kInvalidEmptyValueHeader,         "Invalid Empty Value Header"},
            {StompError::kParsingMissingEolAfterHeaderValue,   "Missing EOL After Header Value"},
            {StompError::kParsingMissingBlankLineAfterHeaders,   "Missing Blank Line After Headers"},
            {StompError::kInvalidHeaderKey,                "Invalid Header Key"},
            {StompError::kJunkAfterBodyError,              "Junk After Body Error"},
            {StompError::kConentLengthMistMatchError,      "Content Length Mismatch Error"},
            {StompError::kParsingMissingEolAfterCommand,   "Missing End-Of-Line After Command"},
            {StompError::kMissingHeaderValue,              "Missing Header Vlaue" },
            {StompError::kParsingMissingNullAtTheEndOfBody,      "Missing NULL At The End of Body" },
            {StompError::kParsingExtraJunkFoundAfterNullInBody,  "Extra Junk Found After Null in Body" },
            {StompError::kInvalid,                         "Invalid"}
        }
    )
}; 

std::ostream &NetworkMonitor::operator<<(std::ostream &os, const StompCommand &command){
    auto it = gStompComamndStrings.left.find(command);
    auto invalidCommand = gStompComamndStrings.left.at(StompCommand::kInvalid);
    if (it == gStompComamndStrings.left.end()){
        os << invalidCommand;
    }
    os << it->second;
    return os;
}

std::ostream &NetworkMonitor::operator<<(std::ostream &os, const StompError& error){
    auto it = gStompErrorStrings.left.find(error);
    auto invalidError = gStompErrorStrings.left.at(StompError::kInvalid);
    if (it == gStompErrorStrings.left.end()){
        os << invalidError;
    }
    os << it->second;
    return os;
}


std::ostream &NetworkMonitor::operator<<(std::ostream &os, const StompHeader &header){
    auto it = gStompHeaderStrings.left.find(header);
    auto invalidHeader = gStompHeaderStrings.left.at(StompHeader::kInvalid);
    if(it == gStompHeaderStrings.left.end()){
        os << invalidHeader;
    }
    os << it->second;
    return os;
}

static std::string ToString(const StompCommand &command){
   return std::string(gStompComamndStrings.left.find(command)->second);
}

static const StompCommand ToCommand(const std::string_view command){
    auto it = gStompComamndStrings.right.find(command);
    if (it == gStompComamndStrings.right.end()){
        return StompCommand::kInvalid;
    }
    return it->second;
}

static const StompHeader ToHeader(const std::string_view header){
    auto it = gStompHeaderStrings.right.find(header);
    if (it == gStompHeaderStrings.right.end()){
        return StompHeader::kInvalid;
    }
    return it->second;
}


// main parser here
StompFrame::StompFrame(StompError& ec, const std::string& frame){
    this->plain_ = frame;
    ec = ParseRawStompFrame(this->plain_);
    if (ec != StompError::kOk){
        logger::error("Invalid Stomp Frame");
        this->plain_ = {};
        this->headers_.clear();
        this->body_ = {};
    }
}

StompFrame::StompFrame(StompError& ec, std::string&& frame){
    this->plain_ = std::move(frame);
    ec = ParseRawStompFrame(this->plain_);
    if (ec != StompError::kOk){
        logger::error("Invalid Stomp Frame");
        this->plain_ = {};
        this->headers_.clear();
        this->body_ = {};
    }
}

static bool StringViewToI(const std::string_view &sv, size_t& result){
        auto convResult = std::from_chars(
            sv.data(),
            sv.data() + sv.size(),
            result
        );
        return convResult.ec != std::errc::invalid_argument;
}


StompError StompFrame::ParseRawStompFrame(const std::string_view frame)
{
    static const char null {'\0'};
    static const char colon {':'};
    static const char newline {'\n'};

    size_t command_end = frame.find(newline);
    if (command_end == std::string::npos){
        return StompError::kParsingMissingEolAfterCommand;
    }
    command_ = ToCommand(plain_.substr(0, command_end));
    if (command_ == StompCommand::kInvalid){
        logger::error("Invalid Stomp Command {} ", plain_.substr(0, command_end));
        return StompError::kInvalidCommand;   
    }     
    // parsing headers
    size_t headerKeyStart {command_end + 1};
    size_t headerValueStart {};
    bool finish_header_parsing = false;
    while(!finish_header_parsing){
        headerValueStart = frame.find(colon, headerKeyStart);
        size_t headerValueEnd = frame.find(newline, headerKeyStart);
        if (headerValueStart == std::string::npos){
           finish_header_parsing = true;
           continue; 
        }
        headerValueStart++; // skip the collon 
        if (headerValueEnd == std::string::npos){
            return StompError::kParsingMissingEolAfterHeaderValue;
        }

        auto headerKey = frame.substr(headerKeyStart, headerValueStart - headerKeyStart - 1);
        auto headerValue = frame.substr(headerValueStart, headerValueEnd - headerValueStart);
        if (headerValue.empty()){
           logger::error("Found empty header value for key {}", headerKey);
           return StompError::kInvalidEmptyValueHeader;
        }
        auto stompHeader = ToHeader(headerKey);
        if (stompHeader == StompHeader::kInvalid){
            logger::error("Invalid StompHeader {}", headerKey);
            return StompError::kInvalidHeaderKey;
        }
        headers_.emplace(stompHeader, headerValue);
        headerKeyStart = headerValueEnd + 1;
        if (headerKeyStart >= frame.size()){
            // invalid here as we need to see a new line  at end of frame 
            // even for empty body
            return StompError::kInvalidFormat;    
        }
        // check to see if we need to jump to parsing body
        if (frame.substr(headerKeyStart, 1).find(newline) != std::string::npos){
            logger::info("Found newline after header complete parsing ! Jump to body");
            finish_header_parsing = true;
        }
    }
    // parsing body 
    size_t body_start = headerKeyStart;
    if (frame.substr(body_start, 1).find(newline) == std::string::npos){
        logger::error("No EOL between headers and body/command");
        return StompError::kParsingMissingBlankLineAfterHeaders;
    }

    body_ = frame.substr(body_start + 1);
    size_t null_index = body_.find(null);
    // check if there is junk after null
    if (null_index == std::string::npos){
        logger::error("Missing null at the end of body {}", body_);
        return StompError::kParsingMissingNullAtTheEndOfBody;
    }
    if (null_index != (body_.size() - 1)){
        logger::error("Extra junk after null in body {}", body_);
        return StompError::kParsingExtraJunkFoundAfterNullInBody;
    }
    // remove null from body 
    body_ = body_.substr(0, body_.size() -1);
    auto content_len_it = headers_.find(StompHeader::kContentLength);
    if (content_len_it != headers_.end()){
        size_t contLen = 0;
        bool convSuccess = StringViewToI(content_len_it->second, contLen);
        if (!convSuccess){
            logger::error("Invalid conent length value {}", content_len_it->second);
            return StompError::kInvalidContentLengthValueType;
        }
        if (contLen != body_.size()){
            logger::error("Content Length {} is mismatched with body size {}", contLen, body_.size());
            return StompError::kConentLengthMistMatchError;
        }
    }
    return StompError::kOk;
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

StompCommand StompFrame::GetCommand() const
{
    return command_;
}

const std::string_view &StompFrame::GetHeaderValue(const StompHeader &header) const {
    static const std::string_view emptyHeaderValue {""};
    if (headers_.find(header) == headers_.end()){
        return emptyHeaderValue;
    }
    // need to use at as bracket operator does not guarantee const 
    return headers_.at(header);

}

const std::string_view &StompFrame::GetBody()
{
    return body_;
}
