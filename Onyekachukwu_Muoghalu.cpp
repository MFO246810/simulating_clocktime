// Name: Onyekachukwu Muoghalu 
// Id: 2188281
#include <iostream>
#include <queue>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <climits>
using namespace std;


struct process {
   int pid;
   int arrival_time = 0;
   int cpu_time_used = 0;
   int ssd_accesses = 0;
   int user_interactions = 0;
   string priority;
   string status = "Ready";
   vector<string> events;
};


void HandleCpuEvents(process* P, int &clock_time) {
   int temp = stoi(P->events[0].substr(3));  
   P->cpu_time_used += temp;
   P->arrival_time = clock_time;
   clock_time += temp;  
   P->events.erase(P->events.begin());  
}


void HandleUserEvents(process* P, map <int, process*> &u, int &clock_time) {
   int temp = stoi(P->events[0].substr(4));  
   P->user_interactions += 1;
   P->events.erase(P->events.begin());  
   u[clock_time + temp] = P;  
   P->status = "Blocked";  
}


void HandleSSDEvents(process* P, int &clock_time) {
   int temp = stoi(P->events[0].substr(3));  
   P->ssd_accesses += 1;
   P->priority = "Low"; 
   P->events.erase(P->events.begin()); 
}


void Print_Table(vector<process*>& p) {
    
   cout << "Process Table" << endl;
   for(int i = 0; i < p.size(); i++) {
       cout << "Process " << p[i]->pid << " is " << p[i]->status << endl;
   }
   for(int i = 0; i < p.size(); i++){
       if(p[i]->status == "Terminated"){
            p.erase(p.begin() + i);
       } 
    }
}


int main() {
   istream& input = cin;
   if(input) {
       string line;
       vector<string> lines;


       while (getline(input, line)) {
           line.erase(remove(line.begin(), line.end(), '\r'), line.end());
           line.erase(remove(line.begin(), line.end(), ' '), line.end());
           lines.push_back(line);
       }


       
       string Ncores = "";
       Ncores = Ncores + lines[0].substr(6);
       
       int numCores = stoi(Ncores);
       vector<int> cores(numCores, 0); 


       lines.erase(lines.begin());
       int sizes = lines.size();
       lines.pop_back();
       
       vector<process*> processes;
       queue<process*> Hpevents;
       queue<process*> Lpevents;
       queue<int> SSDevents;
       vector<process*> Completed;
       vector<string> temp;
       vector<process*> running_on_core(numCores);
       queue<process*> waitin_process;
       int clock_time = 0;


       for (int i = 0; i < sizes; i++) {
           if (lines[i][0] == 'N' && i != 0) {
               process* p = new process;  
               p->events = temp;
               processes.push_back(p);
               temp.clear();
           }
           temp.push_back(lines[i]);
           if (i == sizes - 2) {
               process* p = new process;  
               p->events = temp;
               processes.push_back(p);
               temp.clear();
           }
       }


       vector<int> Start_time;
       map<int, process*> User_int_t;


       for(int i = 0; i < processes.size(); i++) {
           int temp = stoi(processes[i]->events[0].substr(3));
           Start_time.push_back(temp);
           processes[i]->pid = i;
           processes[i]->arrival_time = temp;
           processes[i]->status = "Ready";
           processes[i]->priority = "High";
           processes[i]->events.erase(processes[i]->events.begin());
       }
       
       sort(Start_time.begin(), Start_time.end());
       auto last = unique(Start_time.begin(), Start_time.end());
       Start_time.erase(last, Start_time.end());
       while(!Start_time.empty()) {
           for(int i = 0; i < processes.size(); i++) {
               if(Start_time[0] == processes[i]->arrival_time) {
                   waitin_process.push(processes[i]);
                  
               }
           }
           Start_time.erase(Start_time.begin());
       }
       
       for(int i = 0; i < numCores; i++){
           Hpevents.push(waitin_process.front());
           waitin_process.pop();
       }
       while (!Hpevents.empty() || !Lpevents.empty() || !User_int_t.empty()|| !waitin_process.empty()) {
           
           for (int i = 0; i < numCores; i++) {
               if (cores[i] == 0 && !Hpevents.empty()) {
                   process* p = Hpevents.front();
                  


                   if (p->events.empty()) {
                       p->status = "Terminated";
                       running_on_core[i] = nullptr;
                       Hpevents.pop();


                   }
                   else if (p->events[0][0] == 'C') {
                       running_on_core[i] = p;
                       int cpu_time = stoi(p->events[0].substr(3));
                       if(p->cpu_time_used == 0){
                           cores[i] = cpu_time + p->arrival_time;
                       }
                       else {
                           cores[i] = cpu_time;
                       }
                      
                       p->cpu_time_used += cpu_time;
                       p->events.erase(p->events.begin());
                       p->status = "Running";
                       Hpevents.pop();
                   } else if (p->events[0][0] == 'S') {
                       HandleSSDEvents(p, clock_time);
                       Lpevents.push(p);
                       SSDevents.push(p->pid);
                       running_on_core[i] = nullptr;
                       Hpevents.pop();
                   } else if (p->events[0][0] == 'U') {
                       HandleUserEvents(p, User_int_t, clock_time);
                       running_on_core[i] = nullptr;
                       Hpevents.pop();
                   }
               }
               if(Hpevents.empty() && !waitin_process.empty()){
                   for(int i = 0; i < numCores; i++){
                       Hpevents.push(waitin_process.front());
                       waitin_process.pop();
                   }
               }
               if (Hpevents.empty() && !Lpevents.empty()) {
               
                   while(!Lpevents.empty()){
                       Hpevents.push(Lpevents.front());
                       Lpevents.pop();
                   }
                  
               }
               }


           vector<int> keysToErase;
           for (const auto& pair : User_int_t) {
               
               if (clock_time >= pair.first) {
                   pair.second->status = "Running";
                   Hpevents.push(pair.second);
                   keysToErase.push_back(pair.first);
               }
           }


           for (const int key : keysToErase) {
               User_int_t.erase(key);
           }


           int min_time = INT_MAX;
           for (int i = 0; i < numCores; i++) {
               if (cores[i] > 0) {
                   min_time = min(min_time, cores[i]);
               }
           }


           if (min_time == INT_MAX) {
               min_time = 0;
           }


           clock_time += min_time;


           for (int i = 0; i < numCores; i++) {
               if (cores[i] > 0) {
                   cores[i] -= min_time;
                   
                   if (cores[i] == 0) {
                       process* p = running_on_core[i];
                       if (p->events.empty() && p->status != "Terminated") {
                           p->status = "Terminated";
                           cout << "Process " << p->pid << " terminated at time t = " << clock_time << "ms" <<endl;
                           cout << "It started at t = " << p->arrival_time << "ms" << endl;
                           cout << "It used " << p->cpu_time_used << " ms of CPU time, performed " << p->ssd_accesses << " SSD access(es), and interacted " << p->user_interactions << " time(s) with its user." << endl;
                           Print_Table(processes);


                           Completed.push_back(p);
                       } else {
                           Hpevents.push(p);
                       }
                   }
               }
           }


          


           if (Hpevents.empty() && Lpevents.empty() && !User_int_t.empty()) {
               auto user = User_int_t.begin();
               clock_time = user->first;
               process* t = user->second;
               Hpevents.push(t);
               User_int_t.erase(user);
           }


          
           
       }


       
   } else {
       cout << "File Not Found" << endl;
   }


   return 0;  
}
