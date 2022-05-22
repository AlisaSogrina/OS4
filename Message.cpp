#include <iostream>
#pragma warning(disable : 4996)

using namespace std;

struct Message {
	char name[100];
	char text[100];

	Message() {
		strcpy(name, "");
		strcpy(text, "");
	}

	Message(const char* name, const char* text) {
		strcpy(this->name, name);
		strcpy(this->text, text);
	}
};