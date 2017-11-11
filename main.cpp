#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <iostream>
#include <string>
#include <algorithm>

#include "DateTimeStamper.h"

//  Case-insensitive string comparison
#ifdef _MSC_VER
#define COMPARE(a, b) (!_stricmp((a), (b)))
#else
#define COMPARE(a, b) (!strcasecmp((a), (b)))
#endif

bool invalidChar(char c) ;
void stripUnicode(std::string &str) ;

//  libxml callback context structure
struct Context {
    Context(): addTitle(false) { }

    bool addTitle;
    std::string title;
};

//  libcurl variables for error strings and returned data
static char errorBuffer[CURL_ERROR_SIZE];
static std::string buffer;

//  libcurl write callback function
static int writer(char *data, size_t size, size_t nmemb, std::string *writerData) ;

//  libcurl connection initialization
static bool init(CURL *&conn, char *url) ;

//  libxml start element callback function
static void StartElement(void *voidContext, const xmlChar *name, const xmlChar **attributes) ;

//  libxml end element callback function
static void EndElement(void *voidContext, const xmlChar *name) ;

//  Text handling helper function
static void handleCharacters(Context *context, const xmlChar *chars, int length) ;

//  libxml PCDATA callback function
static void Characters(void *voidContext, const xmlChar *chars, int length) ;

//  libxml CDATA callback function
static void cdata(void *voidContext, const xmlChar *chars, int length) ;

//  libxml SAX callback structure
static htmlSAXHandler saxHandler = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    StartElement,
    EndElement,
    NULL,
    Characters,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    cdata,
    NULL
};

//  Parse given (assumed to be) HTML text and return the title
static void parseHtml(const std::string &html, std::string &title) ;

int main(int argc, char *argv[]) {

    CURL *conn = NULL;
    CURLcode code;
    std::string title;
    std::string url;
    DateTimeStamper d;

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <url>\n", argv[0]);
        fprintf(stderr, "Usage: %s <url> <title>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Initialize CURL connection
    if(!init(conn, argv[1])) {
        fprintf(stderr, "Connection initializion failed\n");
        exit(EXIT_FAILURE);
    }

    // Retrieve content for the URL
    code = curl_easy_perform(conn);
    curl_easy_cleanup(conn);

    if(code != CURLE_OK) {
        fprintf(stderr, "Failed to get '%s' [%s]\n", argv[1], errorBuffer);
        exit(EXIT_FAILURE);
    }
    
    url = std::string(argv[1]);
    
    if (argc == 2) {
        // Parse the (assumed) HTML code
        parseHtml(buffer, title);
    }
    else {
        title = std::string(argv[2]);
    }

    // Strip the unicode from the title
    stripUnicode( title );

    // Get the time
    std::string timeStr = d.getMilitaryTimeString();

    // Display the extracted title
    //This line works for most sites but I found a site that was breaking this code...
    //http://moonbit.co.in
    //printf("- *%s* [%s](%s)\n", timeStr.c_str(), title.c_str(), url.c_str() );
    //To work around this, I print the title character-by-character and omit printing
    //Of newlines, linefeeds, and tabs
    const char *title_cstr = title.c_str();
    std::string title_new_str = "";
    printf("- *%s* [", timeStr.c_str());
    for (int i = 0; i < title.length(); i++) {
        char c = title_cstr[i];
        if (c != '\n' && c != '\t' && c != 13) {
            // Fixed for twitter links
            // If a link title has an '@' in it, escape it
            // '@' breaks markdown link formatting
            if (c == '@') {
                title_new_str += "\\@";
            }
            // Fixed for anything with hashtags in title
            else if (c == '#') {
                title_new_str += "\\#";
            }
            // Normal
            else {
                title_new_str += c;
            }
        }
    }
    printf("%s](%s)\n", title_new_str.c_str(), url.c_str());
    return EXIT_SUCCESS;
}

bool invalidChar(char c) {
    int d = (int) c;
    return ! ( d >= 0 && d < 128 );
}

void stripUnicode(std::string &str) {
    str.erase( std::remove_if( str.begin(), str.end(), invalidChar ), str.end() );
}

static int writer(char *data, size_t size, size_t nmemb, std::string *writerData) {
    if(writerData == NULL)
        return 0;
    writerData->append(data, size*nmemb);
    return size * nmemb;
}

static bool init(CURL *&conn, char *url) {
    CURLcode code;

    conn = curl_easy_init();

    if(conn == NULL) {
        fprintf(stderr, "Failed to create CURL connection\n");
        exit(EXIT_FAILURE);
    }

    code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, errorBuffer);
    if(code != CURLE_OK) {
        fprintf(stderr, "Failed to set error buffer [%d]\n", code);
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_URL, url);
    if(code != CURLE_OK) {
        fprintf(stderr, "Failed to set URL [%s]\n", errorBuffer);
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
    if(code != CURLE_OK) {
        fprintf(stderr, "Failed to set redirect option [%s]\n", errorBuffer);
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
    if(code != CURLE_OK) {
        fprintf(stderr, "Failed to set writer [%s]\n", errorBuffer);
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, &buffer);
    if(code != CURLE_OK) {
        fprintf(stderr, "Failed to set write data [%s]\n", errorBuffer);
        return false;
    }

    return true;
}

static void StartElement(void *voidContext, const xmlChar *name, const xmlChar **attributes) {
    Context *context = (Context *)voidContext;
    if(COMPARE((char *)name, "TITLE")) {
        context->title = "";
        context->addTitle = true;
    }
    (void) attributes;
}

static void EndElement(void *voidContext, const xmlChar *name) {
    Context *context = (Context *)voidContext;
    if(COMPARE((char *)name, "TITLE"))
        context->addTitle = false;
}

static void handleCharacters(Context *context, const xmlChar *chars, int length) {
    if(context->addTitle)
        context->title.append((char *)chars, length);
}

static void Characters(void *voidContext, const xmlChar *chars, int length) {
    Context *context = (Context *)voidContext;
    handleCharacters(context, chars, length);
}

static void cdata(void *voidContext, const xmlChar *chars, int length) {
    Context *context = (Context *)voidContext;
    handleCharacters(context, chars, length);
}

static void parseHtml(const std::string &html, std::string &title) {
    htmlParserCtxtPtr ctxt;
    Context context;
    ctxt = htmlCreatePushParserCtxt(&saxHandler, &context, "", 0, "", XML_CHAR_ENCODING_NONE);
    htmlParseChunk(ctxt, html.c_str(), html.size(), 0);
    htmlParseChunk(ctxt, "", 0, 1);
    htmlFreeParserCtxt(ctxt);
    title = context.title;
}

