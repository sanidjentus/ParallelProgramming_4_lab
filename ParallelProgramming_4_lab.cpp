#include <iostream>
#include <thread>
#include <Windows.h>
#include <stack>

#define producerAmnt 3
#define consumerAmnt 4
#define workAmnt 10

using namespace std;

volatile long producerWorkCompleted = 10;
volatile long consumerWorkCompleted = 10;

class ThreadSafeStack {
private:
	stack <int> stack;
	HANDLE hEvent;
public:
	ThreadSafeStack() {
		hEvent = CreateEvent(nullptr, false, true, nullptr);
	}

	~ThreadSafeStack() {
		CloseHandle(hEvent);
	}

	void push(int elem, int id) {
		WaitForSingleObject(hEvent, INFINITE);
		stack.push(elem);
		cout << "Producer " << id << " -> " << elem << endl;
		SetEvent(hEvent); //переводим в свободное состояние
	}

	bool tryPop(int& elem, int id) {
		bool result = false;
		WaitForSingleObject(hEvent, INFINITE);
		if (!stack.empty()) {
			result = true;
			elem = stack.top();
			stack.pop();
			cout << "Consumer " << id << " <- " << elem << endl;
		}
		else {
			cout << "Consumer " << id << " sleep" << endl;
		}
		SetEvent(hEvent);
		return result;
	}
};

ThreadSafeStack TSS;

void taskProducer(int id) {
	while (_InterlockedExchangeAdd(&producerWorkCompleted, -1) > 0) {
		int elem = rand() % 100;
		this_thread::sleep_for(chrono::milliseconds(2));
		TSS.push(elem + id, id);
	}
}

void taskConsumer(int id) {
	while (_InterlockedExchangeAdd(&consumerWorkCompleted, -1) > 0) {
		int elem;
		if (TSS.tryPop(elem, id)) {
			this_thread::sleep_for(chrono::milliseconds(5));
		}
		else {
			InterlockedAdd(&consumerWorkCompleted, 1);
		}
	}
}

int main() {
	srand(GetTickCount64());

	thread producerThread[producerAmnt];
	for (int i = 0; i < producerAmnt; i++)
		producerThread[i] = thread(taskProducer, i);

	thread consumerThread[consumerAmnt];
	for (int i = 0; i < consumerAmnt; i++)
		consumerThread[i] = thread(taskConsumer, i);

	for (int i = 0; i < producerAmnt; i++)
		producerThread[i].join();

	for (int i = 0; i < consumerAmnt; i++)
		consumerThread[i].join();

	return 0;
}