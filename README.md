# markdownurl-cpp

An advanced version of markdownurl written in C++.

Markdownurl is a simple tool for generating hyperlinks as list-items with
timestamps attached.

### Requirements

- libxml2
- libcurl

### Building

`$ make`

### Flags

- h: help
- t: datetimeOn
- l: listItemOn
- i <inputURL>: inputURL
- c <customTitle>: customTitle

### Running

```
./main -h
./main -i mbell.tech
./main -l -t -i mbell.tech -c "customTitle"
./main -l -t -i mbell.tech -c "customTitle | ^1"
```

### Example

```
./main -l -t -i mbell.tech -c "customTitle | ^1"
- **12/18/2017** *23:03* [customTitle | mbell.tech | index](mbell.tech)
```
