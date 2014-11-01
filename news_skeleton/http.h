#ifndef _HTTP_H_
#define _HTTP_H_

// The service this header provides that you're interested in:
//
//  HTTP().get(url, pageContents)
//    If the web page at the string url can be fetched, set the string
//    pageContents to the content of the page and return true;
//    otherwise return false.  For example,
//        string s = "http://www.wikipedia.org/wiki/DNA";
//        string text;
//        if (HTTP().get(s, text))
//            cout << text;
//        else
//            cout << "Error fetching " << s << endl;

#ifdef _MSC_VER  // Windows

#include <windows.h>
#include <wininet.h>

#else  //  Mac OS X and LINUX

#include <cstdio>
#include <unistd.h>
#include <string>

// Mac users:  In the following, replace the string literal "wget", if
// necessary, with the path to the wget command, e.g. "/usr/local/bin/wget"
// or "/Users/yourname/Downloads/wget" or whatever.

const std::string wget = "/Users/Victor/wget";

#endif

#include <iostream>
#include <string>
#include <vector>
#include <cctype>

const int MAX_PAGE_SIZE = 1000000;

class HTTPController
{
    typedef std::string string;

    struct Segment
    {
        Segment(size_t s, size_t ln) : start(s), len(ln) {}
        size_t start;
        size_t len;
    };

public:

      // Meyers singleton pattern
    static HTTPController& getInstance()
    {
        static HTTPController instance;
        return instance;
    }

    bool get(string url, string& pageContents)
    {
        if (splitURL(url).scheme.empty())
            url = "http://" + url;

        // std::cerr << "Getting: " << url << std::endl;

        char buffer[MAX_PAGE_SIZE+1];

        if (!doGet(url, buffer, sizeof(buffer)))
            return false;

        pageContents = buffer;
        return true;
    }

private:

#ifdef _MSC_VER
    HINTERNET m_hINet;
#endif

    HTTPController();
    ~HTTPController();
    HTTPController(const HTTPController&);
    HTTPController& operator=(const HTTPController&);

    bool doGet(string url, char buffer[], unsigned long maxLength);

    struct URLParts
    {
        URLParts(string s, string nl, string pth, string prm, string q, string f)
         : scheme(s), netLoc(nl), path(pth), params(prm), query(q), fragment(f)
        {}
        string scheme;
        string netLoc;
        string path;
        string params;
        string query;
        string fragment;
    };

    const URLParts splitURL(string url)
    {
        string scheme;
        string netLoc;
        string path;
        string params;
        string query;
        string fragment;

        size_t fragPos = url.find('#');
        if (fragPos != string::npos)
        {
            fragment = url.substr(fragPos+1);
            url.erase(fragPos); // erases to end of string
        }
        size_t schemePos = url.find(':', 1);
        if (schemePos != string::npos)
        {
            size_t k;
            for (k = 0; k != schemePos; k++)
                if (!isascii(url[k]) || (!isalpha(url[k]) && url[k] != '+' &&
                                             url[k] != '-' && url[k] != '.'))
                    break;
            if (k == schemePos)
            {
                scheme = url.substr(0, schemePos);
                url.erase(0, schemePos+1);
                for (size_t n = 0; n != scheme.size(); n++)
                    scheme[n] = tolower(scheme[n]);
            }
        }
        if (url.compare(0, 2, "//") == 0)
        {
            size_t netLocPos = url.find('/', 2);
            if (netLocPos == string::npos)
            {
                netLoc.assign(url, 2, string::npos);
                url.clear();
            }
            else
            {
                netLoc.assign(url, 2, netLocPos-2);
                url.erase(0, netLocPos);
            }
        }
        size_t queryPos = url.find('?');
        if (queryPos != string::npos)
        {
            query = url.substr(queryPos+1);
            url.erase(queryPos);
        }
        size_t paramsPos = url.find(';');
        if (paramsPos != string::npos)
        {
            params = url.substr(paramsPos+1);
            url.erase(paramsPos);
        }
        return URLParts(scheme, netLoc, url, params, query, fragment);
    }
};

inline HTTPController& HTTP()
{
    return HTTPController::getInstance();
}

#ifdef _MSC_VER  // Windows

inline HTTPController::HTTPController()
{
    m_hINet = InternetOpen("CS32Proj4", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
}

inline HTTPController::~HTTPController()
{
    InternetCloseHandle(m_hINet);
}

inline bool HTTPController::doGet(string url, char buffer[], unsigned long maxLength)
{
    if (maxLength == 0)
        return false;

    HINTERNET wininetHandle = InternetOpenUrl(m_hINet, url.c_str(), NULL, 0, INTERNET_FLAG_DONT_CACHE, 0) ;
    if ( wininetHandle == NULL )
        return false;

    bool result;
    for (;;)
    {
        unsigned long bytesRead;
        result = InternetReadFile(wininetHandle, buffer, maxLength-1, &bytesRead) ? true : false;
        if (bytesRead == 0)
        {
            buffer[0] = '\0';
            break;
        }
        buffer += bytesRead;
        maxLength -= bytesRead;
    }
    InternetCloseHandle(wininetHandle);
    return result;
}

#else  //  MacOS and LINUX

inline HTTPController::HTTPController()
{
}

inline HTTPController::~HTTPController()
{
}

inline bool HTTPController::doGet(string url, char buffer[], unsigned long maxLength)
{
    if (maxLength == 0)
        return false;

    bool isFile = (url.compare(0, 7, "file://") == 0);
    FILE* f;
    if (isFile)
        f = fopen(url.substr(7).c_str(), "r");
    else
    {
        for (size_t k = 0; k < url.size(); k++)
            if (!isascii(url[k]) || !isprint(url[k]) || url[k] == '\'' || url[k] == '\\')
                return false;
        string cmd = wget + " -q -O - '" + url + "'";
        f = popen(cmd.c_str(), "r");
    }
    if (f == NULL)
        return false;
    size_t length = fread(buffer, 1, maxLength-1, f);
    if (isFile)
        fclose(f);
    else
        pclose(f);
    buffer[length] = '\0';
    return true;
}

#endif // _MSC_VER

#endif // #ifndef _HTTP_H_
