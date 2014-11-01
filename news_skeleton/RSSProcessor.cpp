#include "provided.h"
#include <string>
#include "http.h"
#include "Mapper.h"

using namespace std;

class RSSProcessorImpl
{
public:
    RSSProcessorImpl(string rssURL);
	~RSSProcessorImpl();
    bool getData();
    bool getFirstItem(string& link, string& title);
    bool getNextItem(string& link, string& title);
	
private:
	string			m_rssURL;
	string			m_page;
	StringMapper<string> m_rssContent;
};

RSSProcessorImpl::RSSProcessorImpl(string rssURL)
: m_rssURL(rssURL), m_page("")
{
}

RSSProcessorImpl::~RSSProcessorImpl()
{
}

bool RSSProcessorImpl::getData()
{
	if (HTTP().get(m_rssURL, m_page) )				//Get RSS Feed page content from web
	{
		size_t pos	= 0;
		while (pos != string::npos)					//go through the entire m_page string
		{
			pos		= m_page.find("<item>", pos);	//Find <item>, which will contain, within it <title> ,</title> ,<link> ,</link>
			if (pos != string::npos)
			{
				size_t pos_beg	= m_page.find("<title>", pos);			//Position of beginning of title
				pos				= m_page.find("</title>",pos_beg);		//Position of end of title
				string	headline= m_page.substr(pos_beg+7, pos-pos_beg-7);
				pos_beg			= m_page.find("<link>", pos);			//Position of beginning of url link
				pos				= m_page.find("</link>", pos_beg);		//Position of end of url link
				string h_url	= m_page.substr(pos_beg+6, pos-pos_beg-6);
				m_rssContent.insert(h_url, headline);					//Insert parsed headline and url
			}
		}
		return true;
	}
	else	return false;
}

bool RSSProcessorImpl::getFirstItem(string& link, string& title)
{
	return m_rssContent.getFirstPair(link, title);
}

bool RSSProcessorImpl::getNextItem(string& link, string& title)
{
	return m_rssContent.getNextPair(link, title);
}

//******************** RSSProcessor functions **********************************

// These functions simply delegate to RSSProcessorImpl's functions.

RSSProcessor::RSSProcessor(string rssURL)
{
    m_impl = new RSSProcessorImpl(rssURL);
}

RSSProcessor::~RSSProcessor()
{
    delete m_impl;
}

bool RSSProcessor::getData()
{
    return m_impl->getData();
}

bool RSSProcessor::getFirstItem(string& link, string& title)
{
    return m_impl->getFirstItem(link, title);
}

bool RSSProcessor::getNextItem(string& link, string& title)
{
    return m_impl->getNextItem(link, title);
}
