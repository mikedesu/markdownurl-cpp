#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <iostream>
#include <string>
#include <algorithm>

#include <getopt.h>

#include "DateTimeStamper.h"
#include "MarkdownURL.h"

//  Case-insensitive string comparison
#ifdef _MSC_VER
#define COMPARE(a, b) (!_stricmp((a), (b)))
#else
#define COMPARE(a, b) (!strcasecmp((a), (b)))
#endif

using namespace std;

bool invalidChar(char c) ;
void stripUnicode(string &str) ;

//  libxml callback context structure
struct Context {
    Context(): addTitle(false) { }
    bool addTitle;
    string title;
};

void printUsage(char *cmdString) ;

//  libcurl variables for error strings and returned data
static char errorBuffer[CURL_ERROR_SIZE];
static string buffer;

//  libcurl write callback function
static int writer(char *data, size_t size, size_t nmemb, string *writerData) ;

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
static void parseHtml(const string &html, string &title);

// Parse title given on command line and handle special characters
// Currently using ^1 to mean "main page title"
string handleTitle(string cmdLineTitle, string title) {
    size_t beginPos = title.find( "^1" );
    if ( beginPos != string::npos ) {
        size_t endPos = beginPos + 3;
        title.replace( beginPos, endPos, cmdLineTitle );
    }
    return title;
}

int main(int argc, char *argv[]) {
    CURL *conn = NULL;
    CURLcode code;
    string title;
    string url;
    string inputURL = "";
    string customTitle = "";
    DateTimeStamper d;

    int c;
    bool timeOn = false;
    bool printHelp = false;
    bool verboseOn = false;

    while ((c = getopt(argc, argv, "thi:c:")) != -1) {
        switch (c) {
            case -1:
            case 0:
            break;

            case 't':
            timeOn = true;
            break;

            case 'h':
            printHelp = true;
            break;

            case 'i':
            inputURL = string(optarg);
            break;

            case 'c':
            customTitle = string(optarg);
            break;

            case 'v':
            verboseOn = true;
            break;
        }
    }
    
    if (printHelp) {
        printUsage(argv[0]);
        exit(EXIT_SUCCESS);
    }

    if (verboseOn) {printf("curl_global_init\n");}

    curl_global_init(CURL_GLOBAL_DEFAULT);

    char c_URL[512] = {0};
    strcpy(c_URL, inputURL.c_str());

    if (strcmp(c_URL, "")==0) {
        fprintf(stderr, "No URL specified!\n\n");
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }
    
    if (verboseOn) {printf("URL: %s\n", c_URL);}

    // Initialize CURL connection
    if(!init(conn, c_URL)) {
        fprintf(stderr, "Connection initializion failed\n");
        exit(EXIT_FAILURE);
    }

    // Retrieve content for the URL
    code = curl_easy_perform(conn);
    curl_easy_cleanup(conn);

    if(code != CURLE_OK) {
        fprintf(stderr, "Failed to get '%s' [%s]\n", inputURL.c_str(), errorBuffer);
        exit(EXIT_FAILURE);
    }
    
    url = inputURL;
    if (strcmp(customTitle.c_str(), "")==0) {
        // Parse the (assumed) HTML code
        parseHtml(buffer, title);
        if (verboseOn) {printf("title: %s\n", title.c_str());}
    }
    else {
        string title0;
        parseHtml(buffer, title0);

        title = handleTitle(title0, string(customTitle));
        if (verboseOn) {printf("title: %s\n", title.c_str());}
    }

    // Strip the unicode from the title
    stripUnicode( title );

    // Get the date and time
    string dateStr = d.getDateString();
    string timeStr = d.getMilitaryTimeString();

    // Display the extracted title
    // This line works for most sites but I found a site that was breaking this code...
    // http://moonbit.co.in
    // printf("- *%s* [%s](%s)\n", timeStr.c_str(), title.c_str(), url.c_str() );
    // To work around this, I print the title character-by-character and omit printing
    // Of newlines, linefeeds, and tabs
    const char *title_cstr = title.c_str();
    string title_new_str = "";
    
    if (timeOn) {
        printf("- **%s** *%s* ", dateStr.c_str(), timeStr.c_str());
    }
    else {
        printf("- ");
    }
    
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
    
    // Create a new object representing the Title and URL
    MarkdownURL mURL = MarkdownURL(title_new_str, url);

    printf("[%s](%s)\n", mURL.getTitle().c_str(), mURL.getURL().c_str());

    return EXIT_SUCCESS;
}

bool invalidChar(char c) {
    int d = (int) c;
    return ! ( d >= 0 && d < 128 );
}

void stripUnicode(string &str) {
    str.erase( std::remove_if( str.begin(), str.end(), invalidChar ), str.end() );
}

static int writer(char *data, size_t size, size_t nmemb, string *writerData) {
    if(writerData == NULL) {
        return 0;
    }
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
    if(COMPARE((char *)name, "TITLE")) {
        context->addTitle = false;
    }
}

static void handleCharacters(Context *context, const xmlChar *chars, int length) {
    if(context->addTitle) {
        context->title.append((char *)chars, length);
    }
}

static void Characters(void *voidContext, const xmlChar *chars, int length) {
    Context *context = (Context *)voidContext;
    handleCharacters(context, chars, length);
}

static void cdata(void *voidContext, const xmlChar *chars, int length) {
    Context *context = (Context *)voidContext;
    handleCharacters(context, chars, length);
}

static void parseHtml(const string &html, string &title) {
    htmlParserCtxtPtr ctxt;
    Context context;
    ctxt = htmlCreatePushParserCtxt(&saxHandler, &context, "", 0, "", XML_CHAR_ENCODING_NONE);
    htmlParseChunk(ctxt, html.c_str(), html.size(), 0);
    htmlParseChunk(ctxt, "", 0, 1);
    htmlFreeParserCtxt(ctxt);
    title = context.title;
}

void printUsage(char *cmdString) {
    printf("Usage:\n\n");
    printf("%s [-h] [-t] <i - inputURL> [c - customTitle]\n", cmdString);
    printf("-h: Prints help\n");
    printf("-i <inputURL>: specifies the URL to grab the title of\n");
    printf("-c <customTitle>: specifies the custom Title to use in the output\n");
    printf("\n");
}

