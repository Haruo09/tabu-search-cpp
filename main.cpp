#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <limits>
#include <chrono>

using namespace std;

int calculateMakespan(const vector<int>& perm, int num_machines, const vector<vector<int>>& proc_times) {
    int n = perm.size();
    if (n == 0) return 0;
    vector<int> completion_times(num_machines, 0);
    for (int i = 0; i < n; ++i) {
        int job = perm[i];
        completion_times[0] += proc_times[job][0];
        for (int m = 1; m < num_machines; ++m) {
            completion_times[m] = max(completion_times[m],
            completion_times[m - 1]) + proc_times[job][m];
        }
    }
    return completion_times[num_machines - 1];
}

vector<int> generateInitialSolution(int num_jobs) {
    vector<int> initial_seq(num_jobs);
    iota(initial_seq.begin(), initial_seq.end(), 0);
    return initial_seq;
}

vector<int> tabuSearchPFSP(int num_jobs, int num_machines, const vector<vector<int>>& proc_times, int max_iterations, int tabu_tenure, int& final_makespan) {
    vector<int> current_sol = generateInitialSolution(num_jobs);
    int current_ms = calculateMakespan(current_sol, num_machines,
    proc_times);
    vector<int> best_sol = current_sol;
    int best_ms = current_ms;

    vector<vector<int>> tabu_list(num_jobs, vector<int>(num_jobs, 0));
    cout << "Makespan Inicial (Ordem Natural): " << best_ms << "\n";
    for (int iter = 1; iter <= max_iterations; ++iter) {
        int best_neighbor_ms = numeric_limits<int>::max();
        int best_from = -1;
        int best_to = -1;
        int best_moved_job = -1;
        vector<int> best_neighbor_sol;

        for (int i = 0; i < num_jobs; ++i) {
            for (int j = 0; j < num_jobs; ++j) {
                if (i == j) continue;

                vector<int> neighbor = current_sol;
                int moved_job = neighbor[i];
                neighbor.erase(neighbor.begin() + i);
                neighbor.insert(neighbor.begin() + j, moved_job);

                int neighbor_ms = calculateMakespan(neighbor, num_machines, proc_times);
                bool is_tabu = (tabu_list[moved_job][j] >= iter);
                bool aspiration = (neighbor_ms < best_ms);

                if ((!is_tabu || aspiration) && neighbor_ms < best_neighbor_ms) {
                    best_neighbor_ms = neighbor_ms;
                    best_from = i;
                    best_to = j;
                    best_moved_job = moved_job;
                    best_neighbor_sol = neighbor;
                }
            }
        }
        if (best_moved_job == -1) break;

        current_sol = best_neighbor_sol;
        current_ms = best_neighbor_ms;
        tabu_list[best_moved_job][best_from] = iter + tabu_tenure;

        if (current_ms < best_ms) {
            best_ms = current_ms;
            best_sol = current_sol;
        }
    }
    final_makespan = best_ms;
    cout << "Melhor Makespan Final: " << best_ms << "\n";
    return best_sol;
}

string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (string::npos == first) return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}
void runAllInstancesORLib(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << filename << "\n";
        return;
    }
    string line;
    int total_instances = 0;

    while (getline(file, line)) {
        string clean_line = trim(line);

        if (clean_line.rfind("instance ", 0) == 0) {
            total_instances++;
            string inst_name = trim(clean_line.substr(9));

            string description;
            while (getline(file, line)) {
                string trimmed = trim(line);
                if (!trimmed.empty() && trimmed.find("+++") == string::npos)
                {
                description = trimmed;
                break;
                }
            }

            int num_jobs = 0, num_machines = 0;
            file >> num_jobs >> num_machines;

            vector<vector<int>> proc_times(num_jobs, vector<int> (num_machines, 0));
            for (int j = 0; j < num_jobs; ++j) {
                for (int m = 0; m < num_machines; ++m) {
                    int machine_id, duration;
                    file >> machine_id >> duration;
                    proc_times[j][machine_id] = duration;
                }
            }
            cout << "====================================================\n";
            cout << "[" << total_instances << "] Instancia: " << inst_name << " (" << description << ")\n";
            cout << "Dimensoes: " << num_jobs << " tarefas x " << num_machines << " maquinas\n";

            int max_iterations = 300;
            int tabu_tenure = max(5, num_jobs *10/ 11 );
            int best_ms = 0;

            auto start_time = chrono::high_resolution_clock::now();
            vector<int> best_seq = tabuSearchPFSP(num_jobs, num_machines, proc_times, max_iterations, tabu_tenure, best_ms);

            auto end_time = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed = end_time - start_time;
            cout << "Tempo de execucao: " << elapsed.count() << " s\n";
            cout << "Melhor sequencia: [";
            for (int job : best_seq) {
                cout << job << ", ";
            }
            cout<<"]";
            cout << "\n";
        }
    }
    file.close();
    cout << "====================================================\n";
    cout << "Processamento concluido! Total de instancias lidas: " <<
    total_instances << "\n";
}

int main() {
    string filename = "instancias.txt";
    runAllInstancesORLib(filename);
    return 0;
}
