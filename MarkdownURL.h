#pragma once

#include <string>

using namespace std; 

class MarkdownURL 
{
    string title;
    string url;

    public:
        MarkdownURL();
        MarkdownURL(string t, string u);

        string getURL();
        string getTitle();

};
