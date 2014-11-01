
#include "provided.h"
#include "Mapper.h"
#include <vector>
#include <string>
#include <iostream>
#include <queue>
using namespace std;

class NewsAggregatorImpl
{
public:
    NewsAggregatorImpl();
	~NewsAggregatorImpl();
    void addSourceRSSFeed(string feed);
    int getTopStoriesAndKeywords(double thresholdPercentage,
                    vector<Cluster>& topStories, vector<Keyword>& topKeywords);
private:
	void kernelCluster();
	bool submitCluster(NewsCluster* nc1, NewsCluster* nc2);
	void repeatSubmitCluster( bool& redo);
	bool findDuplicates();
	void topStoriesCluster(vector<Cluster>& topStories, double p);
	void findTopKeywords(vector<Keyword>& topKeywords, double p);
	
	vector<NewsCluster*> v;
	vector<RSSProcessor* > m_rssVector;		//RSSProcessor contains the RSS feed website's list of title and urls
};

NewsAggregatorImpl::NewsAggregatorImpl()
{
	v.clear();
	m_rssVector.clear();
}

NewsAggregatorImpl::~NewsAggregatorImpl()
{
	//delete NewsCluster dynamic memory
	vector<NewsCluster*>::iterator iter = v.begin();
	for (iter; iter != v.end(); iter++)
		delete *iter;

	v.clear();
	m_rssVector.clear();
	//Note RSSProcessor Dynamic memory was already deleted previous in kernelCluster function!
}

void NewsAggregatorImpl::addSourceRSSFeed(string feed)
{
	RSSProcessor* rssLink	= new RSSProcessor(feed);
	if (rssLink->getData() )
		m_rssVector.push_back(rssLink);		//Each RSS feed website is mapped with a RSSProcessor Class
	else
		delete rssLink;						//Cannot get data from RSS Website
}

void NewsAggregatorImpl::kernelCluster()	
{	
//Variables
	string title, url;
//Submit each headline story as a kernel cluster
	for (size_t z = 0; z < m_rssVector.size(); z++)						
	{
		if (m_rssVector[z]->getFirstItem(url, title) )
		{	
			do {										//do-while loop, loops through each story within a RSS feed website
				NewsCluster* nc = new NewsCluster;		//Dyanmically allocate memory for NewsCluster	
				nc->submitKernelStory(title, url);		//Submit each story as a kernel		
				v.push_back(nc);						//push NewsCluster to a vector container
			}while (m_rssVector[z]->getNextItem(url,title));
		}
	}
			
//delete RSSPRocessor dynamic memory (free dynamic memory!)
	vector< RSSProcessor*>::iterator ptr1 = m_rssVector.begin();
	for (ptr1; ptr1 != m_rssVector.end(); ptr1++)
		delete *ptr1;
}

bool NewsAggregatorImpl::submitCluster(NewsCluster* nc1, NewsCluster* nc2)
{
//Variables
	string title, url;
	bool submitNew	= false;							//No stories submitted from nc1 to nc2 yet
//Submit Newscluster nc1 to Newscluster nc2
	if (nc1->getFirstNewsItem(title, url) )				{
		do	{
				if (nc2->submitStory(title, url) )		//This will return true if the headline stories have 3 or more match words
					submitNew = true;					//If nc2's size changed due to submitStory, set a flag
		}while(nc1->getNextNewsItem(title, url));		}//Submit all the stories in nc1 cluster to nc2
	return submitNew;									//return flag
}

void NewsAggregatorImpl::repeatSubmitCluster( bool& redo)
{
//Variables
	string title, url;
	bool submitAgain	= false;
//For each cluster in vector v, submit the cluster to every other cluster besides itself
	for (size_t i =0; i < v.size(); i ++)
	{
		for (size_t f = i+1; f < v.size(); f++)	//submitCluster for index > i
		{	
			if (submitCluster(v[i], v[f]) )		//Call submitCluster function to submit one NewsCluster class to another	
				submitAgain = true; 
		}	
		for (int r = i-1; r >= 0; --r)			//submitCluster for index < i
		{
			if (submitCluster(v[i], v[r]) )		//Call submitCluster function to submit one NewsCluster class to another	
				submitAgain = true;
		}
	}
	redo	= submitAgain;
}

bool NewsAggregatorImpl::findDuplicates()		//This function will loop O(N^2) times, if 'v' has N items
{
//Find matching identifier string and erase it from the vector
	for (size_t i =0; i < v.size(); i ++)
	{
		for (size_t f = i+1; f < v.size(); f++)	//findDuplicates for index > i
			if (v[i]->getIdentifier() == v[f]->getIdentifier() )
			{	
				v.erase(v.begin() + f);			//erase duplicate cluster
				return true;					//Note must return when a matching identifier is found
			}									//since erasing from the vector will change it size, this will change 
	}											//the number of loop iterations
	return false;
}

//For use in a set<Keyword> which will be order in the order of numUses
bool operator<(const Keyword& cur, const Keyword& other)
{
	return (cur.numUses < other.numUses);
}

void NewsAggregatorImpl::topStoriesCluster(vector<Cluster>& topStories, double p)
{
//Initiated Variables
	string title, url;
	Keyword tempKey;
	double Num = (p/100) * (v.size());			//N is threshold number of stories for a cluster to be
//Initiated STLs	
	set <Keyword> highest_url_count;			//This set is used to sort the cluster's url count from least to greatest
	vector <Cluster> tempTopStory;				//This set is used to temporarily store the contents of cluster with story count >= N
	queue<Cluster> process;						//Use for Sorting

//Loop through the cluster to find clusters of url link count greater than N and add all related URL to to temporary vector(to be sorted later)									
	for (size_t b = 0; b < v.size(); b++)		//inserted into the topStories vector
	{
		if (static_cast <double>(v[b]->size()) >= Num && v[b]->getFirstNewsItem(title, url))
		{	
			tempKey.keyword = title;					//An arbitrary headline title
			tempKey.numUses = v[b]->size();				//The number of urls websites relating to the headline topic
			highest_url_count.insert(tempKey);
			Cluster top_Cluster(title);					//First Item Title is the title of the cluster
			top_Cluster.addRelatedURL(url);
			while (v[b]->getNextNewsItem(title, url) )	//repeatedly add all urls belonging to the same cluster
				top_Cluster.addRelatedURL(url);
			tempTopStory.push_back(top_Cluster);	//tempTopStory is added to container which is not sorted in order of url counts
		}
	}

// Now Order the Headline in order of greatest related url link counts
	set<Keyword>::reverse_iterator r_iter = highest_url_count.rbegin();		//iterate from the end of set since highest_url_count is sorted in order of url counts
	for ( r_iter; r_iter != highest_url_count.rend(); 	r_iter ++ )			//Decrement down highest_url_count set, since this is order from least to greatest
	{																					
		for (size_t k = 0; k < tempTopStory.size(); k ++)					//For each item in the highest_url_count set starting from the end
		{																	//Find the associated story title within the temporary cluster vector
			if (tempTopStory[k].getTitle() == r_iter->keyword)
				process.push(tempTopStory[k]);								//Add this to a queue of Clusters
		}						
	}

//Push the clusters from the queue to the topStories vector
	while (!process.empty())
	{
		Cluster tempCluster = process.front();	//Initiate a dummy cluster variable 
		process.pop();
		topStories.push_back(tempCluster);		//Note did not take care of case for two clusters with same number of url count 
	}
}

void NewsAggregatorImpl::findTopKeywords(vector<Keyword>& topKeywords, double p)
{									//When this function is called it assumes the topStories vector is already filled with top stories
//Initialize Variables
	string title, url, keyword0;
	Keyword tempKey;			//temporary variable to hold Keyword( string and int)
	double Num = (p/100) * (v.size());
//Initialize STLs
	StringMapper<Keyword> highest_word_count;
	set <Keyword> highest_word_count_order;
	queue<Keyword> process;

//Loop through each unique story and keep a count of the word uses (to be ordered later)
	for (size_t i =0; i < v.size(); i ++)
	{
		if (v[i]->getFirstNewsItem(title, url))
		{	
			set<string> unique;
			set<string>::iterator p;
			unique.clear();
			WordExtractor headline(title);
			while(headline.getNextWord(keyword0) )
			{
				p = unique.find(keyword0);
				if (keyword0.length() >= MIN_WORD_SIZE && p == unique.end() )		//keyword must be greater than 4 characters && must not be added twice within the same headline
				{
					tempKey.keyword = keyword0;
					if (!highest_word_count.find(keyword0, tempKey))				//Search for StringMapper to locate word, if previously added from other headlines
						tempKey.numUses = 1;
					else
						tempKey.numUses	+= 1;
					unique.insert(keyword0);
					highest_word_count.insert(keyword0, tempKey);
				}
			}//At this point all keywords of character length greater than 4 					
		}	 //is added to a StringMapper<Keyword> container with its word count
	}

//Only keep words of count greater than N
	if (highest_word_count.getFirstPair(keyword0, tempKey))
	{
		do {
			if (static_cast <double>(tempKey.numUses) >= Num )
				highest_word_count_order.insert(tempKey);
		}while(highest_word_count.getNextPair(keyword0, tempKey));
	}

//use a set to order the keyword by the number of times it was used.
	set<Keyword>::reverse_iterator r_iter = highest_word_count_order.rbegin();		//iterate from the end of set since that holds story Headline with the highest url count
	for ( r_iter; r_iter != highest_word_count_order.rend(); r_iter ++ )			//Decrement down highest_url_count set, since this is order from least to greatest
		process.push(*r_iter);	

//Push the Keyword from the queue in order it was first added which is from highest count to lowest count of a word
	while (!process.empty())
	{
		Keyword tempKeyword = process.front();	//Initiate a dummy cluster variable 
		process.pop();
		topKeywords.push_back(tempKeyword);		//Note did not take care of case for two clusters with same number of url count 
	}
}

int NewsAggregatorImpl::getTopStoriesAndKeywords(double thresholdPercentage,
                    vector<Cluster>& topStories, vector<Keyword>& topKeywords)
{ 
   //Assuming all RSS feed websites have been added by calling addSourceRSSFeed function
	if (m_rssVector.size() > 0)
	{
		kernelCluster();								//Create NewsCluster with each distinct story as a kernel
		topKeywords.clear();							//clear topKeywords vector of previous content
		findTopKeywords(topKeywords, thresholdPercentage);	//find the top keywords and insert size_to topKeywords vector
		
		bool redo = true;
		while (redo)									//Continue to submit stories to each cluster until there is no change
			repeatSubmitCluster(redo);
		while(findDuplicates()) {}						//Remove all the duplicate NewsClusters

		topStories.clear();								//clear topStories vector of previous content
		topStoriesCluster(topStories, thresholdPercentage);	//insert top stories in a vector of Cluster class		
	}
	return 0;
}

//******************** NewsAggregator functions *******************************

// These functions simply delegate to NewsAggregatorImpl's functions.

NewsAggregator::NewsAggregator()
{
    m_impl = new NewsAggregatorImpl();
}

NewsAggregator::~NewsAggregator()
{
    delete m_impl;
}

void NewsAggregator::addSourceRSSFeed(string feed)
{
    m_impl->addSourceRSSFeed(feed);
}

int NewsAggregator::getTopStoriesAndKeywords(double thresholdPercentage,
                    vector<Cluster>& topStories, vector<Keyword>& topKeywords)
{
    return m_impl->getTopStoriesAndKeywords(thresholdPercentage, topStories,
                                                                  topKeywords);
}
