#ifndef _PROVIDED_H_
#define _PROVIDED_H_

////////////////////////////
// provided constants
////////////////////////////

const int MIN_WORD_SIZE = 4;             // in letters
const int REQUIRED_WORDS_IN_COMMON = 3;  // how many words must two headlines have in common to be related in the same cluster?


////////////////////////////
// provided classes
////////////////////////////

#include <string>
#include <set>
#include <vector>
#include <cstdlib>
#include <cctype>
#include <iostream>


class WordExtractor
{
public:
    WordExtractor(const std::string& text)
     : m_text(text), m_nextChar(0)
    {}

    bool getNextWord(std::string& word)
    {
        // find first letter
        for ( ; m_nextChar != m_text.size(); m_nextChar++)
        {
            char ch = m_text[m_nextChar];
            if (isascii(ch) && isalpha(ch))
                break;
        }

        // no letter
        if (m_nextChar == m_text.size())
        {
            word.clear();
            return false;
        }

        size_t start = m_nextChar;

        // find next non-letter
        for ( ; m_nextChar != m_text.size(); m_nextChar++)
        {
            char ch = m_text[m_nextChar];
            if ( ! (isascii(ch) && isalpha(ch)) )
                break;
        }

        word.assign(m_text, start, m_nextChar - start);
        return true;
    }

private:
    std::string m_text;
    size_t      m_nextChar;
};


////////////////////////////////////////////////////////////////////////////////////
// required interfaces 
////////////////////////////////////////////////////////////////////////////////////

class RSSProcessorImpl;

class RSSProcessor
{
public:
    RSSProcessor(std::string rssURL);
    ~RSSProcessor();
    bool getData();
    bool getFirstItem(std::string& link, std::string& title);
    bool getNextItem(std::string& link, std::string& title);

private:
    // RSSProcessor can not be copied or assigned.  We enforce this by declaring the copy constructor and assignment operator private and
    // not implementing them.
    RSSProcessor(const RSSProcessor&);
    RSSProcessor& operator=(const RSSProcessor&);
    
    RSSProcessorImpl* m_impl;
};

class NewsClusterImpl;

class NewsCluster
{
public:
    NewsCluster();
    ~NewsCluster();
    bool submitKernelStory(std::string headline, std::string url);
    bool submitStory(std::string headline, std::string url);
    std::string getIdentifier() const;
    bool getFirstNewsItem(std::string& headline, std::string& url);
    bool getNextNewsItem(std::string& headline, std::string& url);
    int size() const;

private:

    NewsClusterImpl* m_impl;
};

class Cluster
{
public:
    Cluster(std::string t)
     : title(t)
    {}

    void addRelatedURL(std::string url)
    {
        urls.insert(url);
    }

    int getNumURLs() const
    {
        return static_cast<int>(urls.size());
    }

    bool getFirstURL(std::string& url)
    {
        current = urls.begin();
        return getNextURL(url);
    }

    bool getNextURL(std::string& url)
    {
        if (current == urls.end())
            return false;

        url = *current;
        current++;
        return true;

    }

    std::string getTitle() const
    {
        return title;
    }

private:
    std::string title;
    std::set<std::string> urls;
    std::set<std::string>::iterator current;
};

struct Keyword
{
    std::string keyword;
    int numUses;
};

class NewsAggregatorImpl;

class NewsAggregator
{
public:
    NewsAggregator();
    ~NewsAggregator();
    void addSourceRSSFeed(std::string feed);
    int getTopStoriesAndKeywords(double thresholdPercentage,
            std::vector<Cluster>& topStories, std::vector<Keyword>& topKeywords);

private:
    // NewsAggregator can not be copied or assigned.  We enforce this by declaring the copy constructor and assignment operator private and
    // not implementing them.
    NewsAggregator(const NewsAggregator&);
    NewsAggregator& operator=(const NewsAggregator&);
    
    NewsAggregatorImpl* m_impl;
};


#endif // #ifndef _PROVIDED_H_
