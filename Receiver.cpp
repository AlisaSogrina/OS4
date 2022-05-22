#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Message.cpp"

#pragma warning(disable : 4996)

using namespace std;

HANDLE toRead;
HANDLE toWrite;
HANDLE mutex;
int readPosition = 0;
int sizeOfQueue;

string GetExeFileName() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	return std::string(buffer);
}

string GetExePath() {
	std::string f = GetExeFileName();
	return f.substr(0, f.find_last_of("\\/"));
}

void createBinaryFile(char* filename, int queueSize) {
	ofstream fout(filename, ios::binary);
	int position = 0;
	char p[10];
	itoa(position, p, 10);
	fout.write(p, sizeof(p));
	Message* m = new Message("empty", "empty");
	for (int i = 0; i < queueSize; i++) {
		fout.write((char*)&m, sizeof(Message));
	}
	fout.close();
}

void read(char* filename) {
	WaitForSingleObject(toRead, INFINITE);
	cout << "Message read position: " << readPosition << endl;

	fstream fin(filename, ios::binary | ios::in | ios::out);
	if (!fin.is_open()) {
		cout << "error\n";
		return;
	}

	Message* m = new Message();
	char writeIter[10];
	int pos = sizeof(writeIter) + sizeof(Message) * readPosition;

	fin.seekg(pos, ios::beg);
	fin.read((char*)m, sizeof(Message));
	cout << m->name << " - " << m->text << endl;

	fin.seekp(pos, ios::beg);
	m = new Message("deleted", "deleted");
	fin.write((char*)m, sizeof(Message));

	readPosition++;
	if (readPosition == sizeOfQueue) {
		readPosition = 0;
	}

	fin.close();

	ReleaseSemaphore(toWrite, 1, NULL);
}

void runSenderProcesses(char* filename, int amountOfSenderProcesses) {
	STARTUPINFO* si = new STARTUPINFO[amountOfSenderProcesses];
	PROCESS_INFORMATION* pi = new PROCESS_INFORMATION[amountOfSenderProcesses];
	char data[50] = "Sender ";
	strcat(data, filename);
	strcat(data, " ");
	char num[10];
	strcpy(num, to_string(sizeOfQueue).c_str());
	strcat(data, num);

	char path[200];
	strcpy(path, GetExePath().c_str());
	strcat(path, "\\Sender.exe");

	for (int i = 0; i < amountOfSenderProcesses; i++) {
		ZeroMemory(&si[i], sizeof(STARTUPINFO));
		si[i].cb = sizeof(STARTUPINFO);

		if (!CreateProcess(NULL, data, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si[i], &pi[i]))
		{
			cout << "The new process is not created.\n";
			return;
		}
	}
}

int main() {
	cout << "Enter file name:\n";
	char filename[100];
	cin >> filename;

	cout << "Enter size of queue:\n";
	cin >> sizeOfQueue;

	cout << "Enter amount of Sender processes:\n";
	int amountOfSenderProcesses;
	cin >> amountOfSenderProcesses;
	toRead = CreateSemaphore(NULL, 0, sizeOfQueue, "Queue is full");
	toWrite = CreateSemaphore(NULL, sizeOfQueue, sizeOfQueue, "Queue is empty");
	mutex = CreateMutex(NULL, FALSE, "Mutex");

	createBinaryFile(filename, sizeOfQueue);
	runSenderProcesses(filename, amountOfSenderProcesses);

	while (true) {
		cout << "1 - Read\n2 - Exit\n";
		int key;
		cin >> key;
		if (key == 1) {
			cout << "Read message is: \n";
			read(filename);
		}
		else if (key == 2) {
			return 0;
		}
	}

	CloseHandle(toRead);
	CloseHandle(toWrite);
	CloseHandle(mutex);
}
