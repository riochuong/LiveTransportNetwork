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
        kInvalid,
        kInvalidFormat,
        kInvalidHeaderFormat,
        kInvalidCommand,
        kInvalidEmptyValueHeader,
        kInvalidEmptyKeyHeader,
        kInvalidContentLengthValueType,
        kInvalidHeaderKey,
        kJunkAfterBodyError,
        kConentLengthMistMatchError,
        kParsingMissingEolAfterCommand,
        kParsingMissingEolAfterHeaderValue,
        kParsingMissingBlankLineAfterHeaders,
        kParsingMissingNullAtTheEndOfBody,
        kParsingExtraJunkFoundAfterNullInBody,
        kMissingHeaderValue,
        
        // Validation error 
        kValidationInvalidCommand,
        kValidationMissingRequiredHeaders,
        kValidationInvalidContentLenValue,
        kValidationContentLenMisMatch
    };
    

    std::ostream& operator<<(
        std::ostream& os,
        const StompCommand& command
    );
    
    std::ostream& operator<<(
        std::ostream& os,
        const StompHeader& command
    );
    
    std::ostream& operator<<(
        std::ostream& os,
        const StompError& error
    );
    


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

            /*! \brief Move constructor
             */
            StompFrame(StompError &ec, std::string&& other);


            /*! \brief Copy assignment operator.
             */
            StompFrame &operator=(const StompFrame &other);

            /*! \brief Move assignment operator.
             */
            StompFrame &operator=(StompFrame &&other);

            std::string_view GetFrameBody(void) {return std::string_view(this->body_);}

            using HeadersMap = std::unordered_map<StompHeader, std::string_view>; 

            StompCommand GetCommand() const;

            const std::string_view& GetHeaderValue(const StompHeader& header) const;

            const std::string_view& GetBody(); 

            const bool HasHeader(const StompHeader &header) const{
                 return headers_.find(header) != headers_.end();
            }


        private:
            std::string plain_ {};
            StompCommand command_ {StompCommand::kInvalid};
            HeadersMap headers_ {};
            std::string_view body_ {};
            StompError ParseRawStompFrame(const std::string_view frame);
            StompError ValidateFrame();
    };
};

#endif // _NETWORK_MONITOR_STOMP_FRAME_
 