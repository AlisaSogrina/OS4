#include <windows.h>
#include <iostream>
#include <fstream>
#include "Message.cpp"

#pragma warning(disable : 4996)

using namespace std;

HANDLE toRead;
HANDLE toWrite;
HANDLE mutex;
int sizeOfQueue;

void write(char* filename, char* message, char* name) {
	WaitForSingleObject(toWrite, INFINITE);
	WaitForSingleObject(mutex, INFINITE);

	fstream fout(filename, ios::binary | ios::in | ios::out);

	int writePosition;
	char p[10];
	fout.read(p, sizeof(p));
	writePosition = atoi(p);
	cout << "Message write position: " << writePosition << endl;

	Message* m = new Message(name, message);
	int pos = sizeof(p) + sizeof(Message) * writePosition;
	fout.seekp(pos, ios::beg);
	fout.write((char*)m, sizeof(Message));

	writePosition++;
	if (writePosition == sizeOfQueue) {
		writePosition = 0;
	}
	itoa(writePosition, p, 10);
	fout.seekp(0, ios::beg);
	fout.write(p, sizeof(p));

	fout.close();

	ReleaseMutex(mutex);
	ReleaseSemaphore(toRead, 1, NULL);
}

int main(int argc, char* argv[]) {
	toRead = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "Queue is full");
	toWrite = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "Queue is empty");
	mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "Mutex");

	char* filename = argv[1];
	cout << filename << "\n";
	sizeOfQueue = atoi(argv[2]);
	cout << sizeOfQueue << "\n";
	cout << "Enter Sender process name:\n";
	char name[20];
	cin >> name;

	while (true) {
		cout << "Enter message:\n";
		char message[100];
		cin >> message;
		write(filename, message, name);
	}
}