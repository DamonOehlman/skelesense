#include <string>
#include <sstream>
#include <XnCppWrapper.h>

std::string CHECK_RC(const unsigned int rc, const char* const description) {
    if(rc != XN_STATUS_OK) {
        std::ostringstream msg;
        
        msg << description << " failed: " << xnGetStatusString(rc);
        
        return msg.str();
    }

    return "";
}