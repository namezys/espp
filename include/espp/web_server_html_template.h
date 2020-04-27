#include <espp/web_server.h>
#include <espp/buffer.h>

namespace espp {

namespace html_template {

const auto DOC_TYPE = "<!DOCTYPE html>\n";
const Buffer YES("yes");
const Buffer NO("no");

const Buffer& yn(bool is)
{
    return is ? YES : NO;
}

class HtmlTag{
public:
    explicit
    HtmlTag(const char* tag, HttpResponse& response, const char* attrs = nullptr):
        _tag(tag),
        _response(response)
    {
        _response << "<" << _tag;
        if(attrs != nullptr and attrs[0] != 0) {
            _response << " " << attrs;
        }
        _response << ">";
    }

    ~HtmlTag()
    {
        _response << "</" << _tag << ">";
    }

    HtmlTag& operator<<(const Buffer& buffer)
    {
        _response << buffer;
        return *this;
    }

protected:
    const char* _tag;
    HttpResponse& _response;
};

}

}

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res
#define UNIQUE_NAME(base) PP_CAT(base, __COUNTER__)
#define OPEN(HTML_TAG, ...) { espp::html_template::HtmlTag UNIQUE_NAME(__tag)(#HTML_TAG, response, #__VA_ARGS__);
#define TAG(HTML_TAG, ...) espp::html_template::HtmlTag(#HTML_TAG, response, #__VA_ARGS__)
#define TITLE(...) TAG(TITLE, __VA_ARGS__)
#define P(...) TAG(P, __VA_ARGS__)
#define B(...) TAG(B, __VA_ARGS__)
#define INPUT(...) TAG(INPUT, __VA_ARGS__)
#define LABEL(...) TAG(LABEL, __VA_ARGS__)

