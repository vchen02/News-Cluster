#include "provided.h"
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

struct clusterInfo 
{
	string keywords;
	string title;
	string url;

	clusterInfo(const string& keyword, const string& strTitle, const string& strUrl)
		: keywords(keyword), title(strTitle), url(strUrl) {}
};
bool operator<(const clusterInfo& cur, const clusterInfo& other)
{
	return (cur.title < other.title);
}

class NewsClusterImpl
{
public:
    NewsClusterImpl();
	~NewsClusterImpl();
    bool submitKernelStory(string headline, string url);
    bool submitStory(string headline, string url);
    string getIdentifier() const;
    bool getFirstNewsItem(string& headline, string& url);
    bool getNextNewsItem(string& headline, string& url);
    int size() const;

private:
	string keepImportant(string headline) const;
	vector <clusterInfo>::iterator	ptr;	//iterator pointer
	vector <clusterInfo>	m_rssCluster;	//vector [0] holds the kernel story 
};

NewsClusterImpl::NewsClusterImpl()
{
	m_rssCluster.clear();
}

NewsClusterImpl::~NewsClusterImpl()
{
	m_rssCluster.clear();
}

string NewsClusterImpl::keepImportant(string headline) const	//only keep one copy of each keyword, duplicates are ignored
{
//Variables
	string word, keyword, concat_str = "";
	WordExtractor story(headline);		

//Given a headline, parse the headline word by word and only retain words of greater than MIN_WORD_SIZE	
	while ( story.getNextWord(word) )			//Check headline string word by word
	{
		if (word.length() >= MIN_WORD_SIZE )	//keep only headline words with at least 4 characters
		{
			WordExtractor keywords(concat_str);	
			while ( keywords.getNextWord(keyword))
			{	if (keyword == word ){ continue; }	}	//The current headline word is already in concat_str
			concat_str	+= word + " ";					//word is a unique word within concat_str
		}
	}
	if (concat_str.length() > 0)
		concat_str.erase( concat_str.length()-1);		//erase an the extra " " at the end
	return concat_str;
}

bool NewsClusterImpl::submitKernelStory(string headline, string url)
{
	string keywords = keepImportant(headline);		//Contains only unique words of >= 4 chars 
	if (m_rssCluster.empty() &&  keywords.length() != 0)				
	{	
		m_rssCluster.push_back( clusterInfo(keywords, headline, url) ); //add into vector
		return true;
	}
	else	return false;
}

bool NewsClusterImpl::submitStory(string headline, string url)
{
	 for (size_t k = 0; k < m_rssCluster.size(); k++)	//Check for same url in the NewsCluster
	 {													//if found, do not add again!
		 if(url == m_rssCluster[k].url)
			 return false;
	 }

	string keywords = keepImportant(headline);		//Contains only unique words of >= 4 chars 
	int counter = 0;								
	if (keywords.length() != 0)
	{
		string ClusterWord, HeadlineWord;					//Variables
		for (size_t i = 0; i < m_rssCluster.size(); i++)	//loop through entire Cluster headlines
		{
			counter = 0;									//reset counter to 0, since this checks a new headline
			WordExtractor curHeadline(keywords);				//submitted headline's unique keywords
			while ( curHeadline.getNextWord(HeadlineWord) )		//Check headline's unique keyword word by word,
			{
				WordExtractor* curCluster	= new WordExtractor(m_rssCluster[i].keywords);	//current headline's unique keywords within a Cluster
				while ( curCluster->getNextWord(ClusterWord) )	{	//Check current headline's keywords word by word within a Cluster
					if ( HeadlineWord == ClusterWord)	{			
						counter++;									//Increment counter if a headline word equals the cluster headline word
						break;							}		}
				delete curCluster;									//delete dynamic memory 
			}		
			if (counter >= REQUIRED_WORDS_IN_COMMON)		{								
				m_rssCluster.push_back( clusterInfo(keywords, headline, url) );	//Add headline story to Cluster if there are at least 3 matching keywords
				break;										}
		}	
	}
	return ( counter >= REQUIRED_WORDS_IN_COMMON); 
}

string NewsClusterImpl::getIdentifier() const
{
	vector<clusterInfo> temp;						//create a temporary vector
	temp	= m_rssCluster;							//copy argument vector to temp vector
	sort (temp.begin(), temp.end() );				//sort the temporary vector
	string	str = "";					
	for (size_t k = 0; k < temp.size(); k++)
		str += temp[k].title + "+";					//concatenate the headline titles
	if (str.length() > 0)
		str.erase( str.length()-1);						//erase the extra "+" at the end;
	return str;
}

bool NewsClusterImpl::getFirstNewsItem(string& headline, string& url)
{													
 	ptr			= m_rssCluster.begin();	
	return getNextNewsItem(headline, url);
}

bool NewsClusterImpl::getNextNewsItem(string& headline, string& url)
{
	if (ptr == m_rssCluster.end())				
		return false;
	headline	= ptr->title;
	url			= ptr->url;	
	ptr ++;
	return true;   
}

int NewsClusterImpl::size() const
{
    return m_rssCluster.size();
}

//******************** NewsCluster functions **********************************

// These functions simply delegate to NewsClusterImpl's functions.

NewsCluster::NewsCluster()
{
    m_impl = new NewsClusterImpl();
}

NewsCluster::~NewsCluster()
{
    delete m_impl;
}

bool NewsCluster::submitKernelStory(string headline, string url)
{
    return m_impl->submitKernelStory(headline, url);
}

bool NewsCluster::submitStory(string headline, string url)
{
    return m_impl->submitStory(headline, url);
}

string NewsCluster::getIdentifier() const
{
    return m_impl->getIdentifier();
}

bool NewsCluster::getFirstNewsItem(string& headline, string& url)
{
    return m_impl->getFirstNewsItem(headline, url);
}

bool NewsCluster::getNextNewsItem(string& headline, string& url)
{
    return m_impl->getNextNewsItem(headline, url);
}

int NewsCluster::size() const
{
    return m_impl->size();
}
