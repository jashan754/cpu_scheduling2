#include <bits/stdc++.h>
#include "parser.h"

#define all(v) v.begin(),v.end()

using namespace std;


class Process {
    public:
        string name;
        int service_time;
        int arrival_time;
        double response_ratio;
};


/** Global Constants **/
const string TRACE = "trace";
const string SHOW_STATISTICS = "stats";
const string ALGORITHMS[9] = {"", "FCFS", "RR", "SPN", "SRT", "HRRN", "FB-1", "FB-2i", "AGING"};

bool sortByServiceTime(const tuple<string, int, int> &a, const tuple<string, int, int> &b)
{
    return (get<2>(a) < get<2>(b));
}
bool sortByArrivalTime(const tuple<string, int, int> &a, const tuple<string, int, int> &b)
{
    return (get<1>(a) < get<1>(b));
}


bool descendingly_by_response_ratio(tuple<string, double, int> a, tuple<string, double, int> b)
{
    return get<1>(a) > get<1>(b);
}

string getProcessName(tuple<string, int, int> &a)
{
    return get<0>(a);
}

int getArrivalTime(tuple<string, int, int> &a)
{
    return get<1>(a);
}

int getServiceTime(tuple<string, int, int> &a)
{
    return get<2>(a);
}
void decrementServiceTime(tuple<string, int, int> &a)
{
    get<2>(a) = get<2>(a) - 1;
}

void firstComeFirstServe()
{
    int time = getArrivalTime(processes[0]);
    for (int i = 0; i < process_count; i++)
    {
        string processName = getProcessName(processes[i]);
        int processIndex = processToIndex[processName];
        int arrivalTime = getArrivalTime(processes[i]);
        int serviceTime = getServiceTime(processes[i]);

        finishTime[processIndex] = (time + serviceTime);
        turnAroundTime[processIndex] = (finishTime[processIndex] - arrivalTime);
        normTurn[processIndex] = (turnAroundTime[processIndex] * 1.0 / serviceTime);

        for (int j = time; j < finishTime[processIndex]; j++)
            timeline[j][processIndex] = '*';
        for (int j = arrivalTime; j < time; j++)
            timeline[j][processIndex] = '.';
        time += serviceTime;
    }
}

void roundRobin()
{
}

void shortestProcessNext()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq; // pair of service time and index
    int j = 0;
    for (int i = 0; i < last_instant; i++)
    {
        for (; j < process_count;)
        {
            if (getArrivalTime(processes[j]) <= i)
            {
                pq.push(make_pair(getServiceTime(processes[j]), j));
                j++;
            }
            else
                break;
        }
        if (!pq.empty())
        {
            int processIndex = pq.top().second;
            int arrivalTime = getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);
            pq.pop();

            int temp = arrivalTime;
            for (; temp < i; temp++)
                timeline[temp][processIndex] = '.';

            temp = i;
            for (; temp < i + serviceTime; temp++)
                timeline[temp][processIndex] = '*';

            finishTime[processIndex] = (i + serviceTime);
            turnAroundTime[processIndex] = (finishTime[processIndex] - arrivalTime);
            normTurn[processIndex] = (turnAroundTime[processIndex] * 1.0 / serviceTime);
            i = temp - 1;
        }
    }
}

void shortestRemainingTime()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    int j = 0;
    for (int i = 0; i < last_instant; i++)
    {
        for (; j < process_count;)
        {
            if (getArrivalTime(processes[j]) == i)
            {
                pq.push(make_pair(getServiceTime(processes[j]), j));
                j++;
            }
            else
                break;
        }
        if (!pq.empty())
        {
            int processIndex = pq.top().second;
            int remainingTime = pq.top().first;
            pq.pop();
            int serviceTime = getServiceTime(processes[processIndex]);
            int arrivalTime = getArrivalTime(processes[processIndex]);
            timeline[i][processIndex] = '*';

            if (remainingTime == 1) // process finished
            {
                finishTime[processIndex] = i + 1;
                turnAroundTime[processIndex] = (finishTime[processIndex] - arrivalTime);
                normTurn[processIndex] = (turnAroundTime[processIndex] * 1.0 / serviceTime);
            }
            else
            {
                pq.push(make_pair(remainingTime - 1, processIndex));
            }
        }
    }

    for (int i = 0; i < process_count; i++)
    {
        int arrivalTime = getArrivalTime(processes[i]);
        for (int k = arrivalTime; k < finishTime[i]; k++)
        {
            if (timeline[k][i] != '*')
                timeline[k][i] = '.';
        }
    }
}

double calculate_response_ratio(int wait_time, int service_time)
{
    return (wait_time + service_time) / service_time;
}

void highestResponseRatioNext()
{
    // Vector of tuple <process_name, process_response_ratio, time_in_service> for processes that are in the ready queue
    vector< tuple<string, double, int> > present_processes;
    for (int current_instant = 0; current_instant < last_instant; current_instant++)
    {
        for(auto proc : processes)
        {
            // If current_instant == process_arrival_time: response ratio = 1.0 as wait time == 0.
            if( getArrivalTime(proc) == current_instant )
                present_processes.push_back( make_tuple(getProcessName(proc), 1.0, 0) );
        }

        // Calculate response ratio for every process
        for(auto &proc : present_processes)
        {
            string process_name = get<0>(proc);
            int time__in__service = get<2>(proc);
            int process_index = processToIndex[process_name];
            int wait_time = current_instant - getArrivalTime(processes[process_index]) + time__in__service;
            int service_time = getServiceTime(processes[process_index]);

            get<1>(proc) = calculate_response_ratio(wait_time, service_time);
        }

        // Sort present processes by highest to lowest response ratio
        sort(all(present_processes), descendingly_by_response_ratio);

        if( !present_processes.empty() )
        {
            for(int i = 1; i < (int)present_processes.size(); i++)
            {
                int process_index = processToIndex[get<0>(present_processes[i])];
                timeline[current_instant][process_index] = '.';
            }

            int process_index = processToIndex[get<0>(present_processes[0])];
            timeline[current_instant][process_index] = '*';
            get<2>(present_processes[0]) ++;    // Increment time_in_service

            if( get<2>(present_processes[0]) == getServiceTime(processes[process_index]) )
            {
                swap(present_processes[0], present_processes[present_processes.size() - 1]);
                present_processes.pop_back();
                finishTime[process_index] = current_instant + 1;
                turnAroundTime[process_index] = finishTime[process_index] - getArrivalTime(processes[process_index]);
                normTurn[process_index] = (turnAroundTime[process_index] * 1.0 / getServiceTime(processes[process_index]));
            }
        }
    }
}

void feedbackQ1()
{
}

void feedbackQ2i()
{
}

void printAlgorithm()
{
    cout << ALGORITHMS[stoi(algorithms[0])] << endl;
}

void printProcesses()
{
    cout << "Process    ";
    for (int i = 0; i < process_count; i++)
        cout << "| " << getProcessName(processes[i]) << " ";
    cout << "|" << endl;
}
void printArrivalTime()
{
    cout << "Arrival    ";
    for (int i = 0; i < process_count; i++)
        cout << "| " << getArrivalTime(processes[i]) << " ";

    cout << "|" << endl;
}
void printServiceTime()
{
    cout << "Service    ";
    for (int i = 0; i < process_count; i++)
        cout << "| " << getServiceTime(processes[i]) << " ";
    cout << "| Mean|" << endl;
}
void printFinishTime()
{
    cout << "Finish     ";
    for (int i = 0; i < process_count; i++)
        cout << "| " << finishTime[i] << " ";
    cout << "|-----|" << endl;
}
void printTurnAroundTime()
{
    cout << "Turnaround ";
    int sum = 0;
    for (int i = 0; i < process_count; i++)
    {
        cout << "| " << turnAroundTime[i] << " ";
        sum += turnAroundTime[i];
    }
    cout << fixed << setprecision(2);
    cout << "| " << (1.0 * sum / turnAroundTime.size()) << "|" << endl;
}

void printNormTurn()
{
    cout << "NormTurn   ";
    cout << fixed << setprecision(2);
    float sum = 0;
    for (int i = 0; i < process_count; i++)
    {
        cout << "| " << normTurn[i] << " ";
        sum += normTurn[i];
    }

    cout << "| " << (1.0 * sum / normTurn.size()) << "|" << endl;
}
void printStats()
{
    printAlgorithm();
    printProcesses();
    printArrivalTime();
    printServiceTime();
    printFinishTime();
    printTurnAroundTime();
    printNormTurn();
}

void printTimeline()
{
    cout << ALGORITHMS[stoi(algorithms[0])] << " ";
    for (int i = 0; i <= last_instant; i++)
        cout << " " << i % 10;
    cout << endl;
    cout << "------------------------------------------------" << endl;
    for (int i = 0; i < process_count; i++)
    {
        cout << getProcessName(processes[i]) << "\t\b\b";
        for (int j = 0; j < last_instant; j++)
        {
            cout << "|" << timeline[j][i];
        }
        cout << "|" << endl;
    }

    cout << "------------------------------------------------" << endl;
}

int main()
{
    // freopen("input.txt","r",stdin);
    parse();

    // shortestProcessNext();
    // firstComeFirstServe();

    highestResponseRatioNext();

    if(operation == TRACE)
        printTimeline();
    else if(operation == SHOW_STATISTICS)
        printStats();
    return 0;
}
