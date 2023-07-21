#ifndef _NETWORK_MONITOR_STOMP_FRAME_
#define _NETWORK_MONITOR_STOMP_FRAME_

#include <string>
#include <unordered_map>

namespace NetworkMonitor {

    enum class StompCommand {
        kInvalid,
        kAbort,
        kAck,
        kBegin,
        kCommit,
        kConnect,
        kConnected,
        kDisconnect,
        kError,
        kMessage,
        kNack,
        kReceipt,
        kSend,
        kStomp,
        kSubscribe,
        kUnsubscribe,
    };

    enum class StompHeader {
        kInvalid,
        kAcceptVersion,
        kAck,
        kContentLength,
        kContentType,
        kDestination,
        kHeartBeat,
        kHost,
        kId,
        kLogin,
        kMessage,
        kMessageId,
        kPasscode,
        kReceipt,
        kReceiptId,
        kSession,
        kSubscription,
        kTransaction,
        kServer,
        kVersion
    };

    enum class StompError {
        kOk = 0,
        kInvalidFormat,
        kInvalidHeaderFormat,
        kInvalidCommand,
        kInvalidEmptyValueHeader,
        kInvalidEmptyKeyHeader,
        kInvalidHeaderKey
    };
    

    std::ostream& operator<<(
        std::ostream& os,
        const StompCommand& command
    );
    
    std::ostream& operator<<(
        std::ostream& os,
        const StompHeader& command
    );

    std::string ToString(const StompCommand& command);
    
    StompCommand ToCommand(const std::string_view command);


    /* \brief STOMP Frame version 1.20 representation  
    */
    class StompFrame {
        public:
           /*! \brief Default constructor. Corresponds to an emtpy, invalid STOMP frame 
            */
            StompFrame(): command_(StompCommand::kInvalid), body_("") {};

            /*! \brief Construct STOMP frame from a string. The string is copied 
             * 
             *   The result of the operation is stored in the error code 
             */
            StompFrame(
                StompError& ec,
                const std::string& frame
            );

            /*! \brief Copy constructor
             */
            StompFrame(const StompFrame& other);


            /*! \brief Move constructor
             */
            StompFrame(StompFrame&& other);

            /*! \brief Copy assignment operator.
             */
            StompFrame &operator=(const StompFrame &other);

            /*! \brief Move assignment operator.
             */
            StompFrame &operator=(StompFrame &&other);

            std::string_view GetFrameBody(void) {return std::string_view(this->body_);}

        private:
            StompCommand command_;
            std::unordered_map<StompHeader, std::string> headers_;
            std::string body_;
    };
};

#endif // _NETWORK_MONITOR_STOMP_FRAME_
 