#include "provided.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
using namespace std;

int main(int argc, char* argv[])
{
	if (argc !=3 || argv[1] == NULL)
	{
		cout << "usage: proj4.exe RSSlist thresholdPercentage";
		exit(1);
	}
	NewsAggregator News;
	vector<Cluster> topStories;
	vector<Keyword> topKeywords;
	ifstream infile(argv[1]);
	if (!infile)
		cout << "error: unable to open RSS list file: theNameOfTheRSSFile" << endl;
	string s;
	while (getline(infile, s) )
		News.addSourceRSSFeed(s);

	double p = strtod(argv[2], NULL);
	cout << "Finding the top news stories and keywords..." <<endl << endl;
	if (News.getTopStoriesAndKeywords(p, topStories, topKeywords) == 0)
	{
		if (topStories.size() == 0)
			cout << "No newsworthy topics were found" << endl;
		else
		{
			cout << "There were " << topStories.size() << " newsworthy topics: " << endl <<endl;
			string url;
			for (size_t i = 0; i< topStories.size(); i++)
			{
				if (topStories[i].getFirstURL(url))
				{
					cout << "Cluster # " << i+1 << " has " << topStories[i].getNumURLs();
					cout << " stories about: " << topStories[i].getTitle() <<endl;
					do{
						cout << " " << url << endl;
					}while (topStories[i].getNextURL(url));
					cout << endl;
				}
			}
			cout << endl;
		}
		if (topKeywords.size() == 0 )
			cout << "No important keywords were found." << endl;
		else
		{
			cout << "There were " << topKeywords.size() << " newsworthy keywords: " << endl <<endl;
			string keyword;
			for (size_t j = 0; j< topKeywords.size(); j++)
				cout << " " << topKeywords[j].keyword << ": " << topKeywords[j].numUses << " uses\n";
		}
	}
	return 0;
}

