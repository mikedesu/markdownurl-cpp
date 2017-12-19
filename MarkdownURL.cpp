#include "MarkdownURL.h"

MarkdownURL::MarkdownURL(string t, string u) 
{
    this->title = t;
    this->url = u;
}

string MarkdownURL::getTitle() 
{
    return this->title;
}

string MarkdownURL::getURL()
{
    return this->url;
}

