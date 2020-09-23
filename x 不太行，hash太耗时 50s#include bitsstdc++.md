```cpp
不太行，hash太耗时 50s
#include <bits/stdc++.h>
using namespace std;

#define debug false

class Solution {
private:
    unordered_map<int, int> id2idx;
	int *id;
	int idLen;
	int *graph, *gStart;
	int *antigraph, *agStart;
	vector<vector<int>> circle;
	list<int> path;

	void dfs(int cur, int target, bool *visit, int *reachable) {
		for(int i = gStart[cur]; i < gStart[cur + 1]; i++) {
			int id_ = graph[i];
			int idx_ = id2idx[id_];
			if(id_ < id[target]) continue;
			if(reachable[idx_] == -2 && !visit[idx_]) {
				path.push_back(id_);
				if(path.size() > 2) {
					vector<int> temp;
					for(auto p : path) {
						temp.push_back(p);
					}
					circle.push_back(temp);
				}
				path.pop_back();
			}
			if(visit[idx_] || (reachable[idx_] != target && reachable[idx_] != -2)) continue;
			if(path.size() == 6 || idx_ == target) continue;
			visit[idx_] = true;
			path.push_back(id_);
			dfs(idx_, target, visit, reachable);
			path.pop_back();
			visit[idx_] = false;
		}
	}
	void prune_g(int cur, int target, bool * visit, int *reachable, int distance) {
		for(int i = gStart[cur]; i < gStart[cur + 1]; i++) {
			int id_ = graph[i];
			int idx_ = id2idx[id_];
			if(id_ < id[target] || visit[idx_]) continue;
			reachable[idx_] = target;
			if(distance == 3) continue;
			visit[idx_] = true;
			prune_g(idx_, target, visit, reachable, distance + 1);
			visit[idx_] = false;
		}
	}
	void prune_ag(int cur, int target, bool * visit, int *reachable, int distance) {
		for(int i = agStart[cur]; i < agStart[cur + 1]; i++) {
			int id_ = antigraph[i];
			int idx_ = id2idx[id_];
			if(id_ < id[target] || visit[idx_]) continue;
			reachable[idx_] = target;
			if(distance == 3) continue;
			visit[idx_] = true;
			prune_ag(idx_, target, visit, reachable, distance + 1);
			visit[idx_] = false;
		}
	}

public:
	void loadData(string filename) {
        FILE* file=fopen(filename.c_str(),"r");
        int st, end, cost;
        int idx = 0;
        unordered_map<int, int> idx2id;
        vector<vector<int>> fg;
        vector<vector<int>> bg;
        while(fscanf(file,"%u,%u,%u",&st,&end,&cost)!=EOF){
            if(id2idx.find(st) == id2idx.end()) {
            	id2idx[st] = idx++;
            	idx2id[idx - 1] = st;
            	st = idx - 1;
            } else {
            	st = id2idx[st];
            }
            if(id2idx.find(end) == id2idx.end()) {
            	id2idx[end] = idx++;
            	idx2id[idx - 1] = end;
            	end = idx - 1;
            } else {
            	end = id2idx[end];
            }
            while(fg.size() < idx) {
            	vector<int> temp;
            	fg.push_back(temp);
            }
            fg[st].push_back(idx2id[end]);
            while(bg.size() < idx) {
            	vector<int> temp;
            	bg.push_back(temp);
            }
            bg[end].push_back(idx2id[st]);
        }
        idLen = idx;
        id = new int[idLen];
        for(int i = 0; i < idLen; i++) {
        	id[i] = idx2id[i];
        }
        gStart = new int[idLen];
        gStart[0] = 0;
        for(int i = 0; i < fg.size(); i++) {
        	gStart[i + 1] = fg[i].size() + gStart[i];
        }
        graph = new int[gStart[idLen - 1]];
        for(int i = 0; i < fg.size(); i++) {
        	sort(fg[i].begin(), fg[i].end());
        	for(int j = gStart[i]; j < gStart[i + 1]; j++) {
        		graph[j] = fg[i][j - gStart[i]];
        	}
        }
        agStart = new int[idLen];
        agStart[0] = 0;
        for(int i = 0; i < bg.size(); i++) {
        	agStart[i + 1] = bg[i].size() + agStart[i];
        }
        antigraph = new int[agStart[idLen - 1]];
        for(int i = 0; i < bg.size(); i++) {
        	sort(bg[i].begin(), bg[i].end());
        	for(int j = agStart[i]; j < agStart[i + 1]; j++) {
        		antigraph[j] = bg[i][j - agStart[i]];
        	}
        }
#if debug
       cout<<"graph size: "<<idx<<endl;
#endif
	}
	void saveAnswer(string filename) {
	    //mmap:https://blog.csdn.net/oceanperfect/article/details/52192602
#if debug
		printf("answer size　: %d\n", circle.size());
#endif
		ofstream out(filename);
		out<<circle.size()<<endl;
		for(int i = 0; i < circle.size(); i++) {
			out<<circle[i][0];
			for(int j = 1; j < circle[i].size(); j++) {
				out<<","<<circle[i][j];
			}
			out<<endl;
		}
	}

	void checkCircle() {
#if debug
		cout<<"checking circle..."<<endl;
#endif
		bool visit[idLen];
		memset(visit, false, idLen * sizeof(bool));
		int reachable[idLen];
		memset(reachable, -1, idLen * sizeof(int));
		for(int i = 0; i < idLen; i++) {
			int t = id[i];
			path.push_back(t);
			prune_g(i, i, visit, reachable, 1);
			prune_ag(i, i, visit, reachable, 1);
			for(int j = agStart[i]; j < agStart[i + 1]; j++) {
				reachable[id2idx[antigraph[j]]] = -2;
			}
			dfs(i, i, visit, reachable);
			path.pop_back();
			for(int j = agStart[i]; j < agStart[i + 1]; j++) {
				reachable[id2idx[antigraph[j]]] = i;
			}
#if debug
			cout<<"point "<<id[i]<<" finished! " << "answer num : " << circle.size()<<endl;
#endif
		}
		sort(circle.begin(), circle.end(), [](vector<int> a,vector<int> b){
			if(a.size() != b.size()) {
				return a.size() < b.size();
			} else {
				return a[0] < b[0];
			}
		});
	}
};


int main() {
	string dataFileName = "test_data_28w.txt";
    string answerFileName = "answer.txt";

    auto t = clock();
    Solution s;
    s.loadData(dataFileName);
    cout<<"load data "<<(clock() - t) / 1000<<endl;
    s.checkCircle();
    cout<<"check circle "<<(clock() - t) / 1000<<endl;
    s.saveAnswer(answerFileName);
    cout<<"save answer "<<(clock() - t) / 1000<<endl;
	return 0;
}

```

```cpp
排序，没有二分查找 22s
#include <bits/stdc++.h>
using namespace std;

#define debug false

class Solution {
private:
	int *id;
	int idLen;
	int *graph, *gStart;
	int *antigraph, *agStart;
	vector<vector<int>> circle;
	list<int> path;

	void dfs(int cur, int target, bool *visit, int *reachable) {
		for(int i = gStart[cur]; i < gStart[cur + 1]; i++) {
			int t = graph[i];
			if(id[t] < id[target]) continue;
			if(reachable[t] == -2 && !visit[t]) {
				path.push_back(t);
				if(path.size() > 2) {
					vector<int> temp;
					for(auto p : path) {
						temp.push_back(id[p]);
					}
					circle.push_back(temp);
				}
				path.pop_back();
			}
			if(visit[t] || (reachable[t] != target && reachable[t] != -2)) continue;
			if(path.size() == 6 || t == target) continue;
			visit[t] = true;
			path.push_back(t);
			dfs(t, target, visit, reachable);
			path.pop_back();
			visit[t] = false;
		}
	}
	void prune_g(int cur, int target, bool * visit, int *reachable, int distance) {
		for(int i = gStart[cur]; i < gStart[cur + 1]; i++) {
			int t = graph[i];
			if(id[t] < id[target] || visit[t]) continue;
			reachable[t] = target;
			if(distance == 3) continue;
			visit[t] = true;
			prune_g(t, target, visit, reachable, distance + 1);
			visit[t] = false;
		}
	}
	void prune_ag(int cur, int target, bool * visit, int *reachable, int distance) {
		for(int i = agStart[cur]; i < agStart[cur + 1]; i++) {
			int t = antigraph[i];
			if(id[t] < id[target] || visit[t]) continue;
			reachable[t] = target;
			if(distance == 3) continue;
			visit[t] = true;
			prune_ag(t, target, visit, reachable, distance + 1);
			visit[t] = false;
		}
	}

public:
	void loadData(string filename) {
        FILE* file=fopen(filename.c_str(),"r");
        int st, end, cost;
        int idx = 0;
    	unordered_map<int, int> id2idx;
        unordered_map<int, int> idx2id;
        vector<vector<int>> fg;
        vector<vector<int>> bg;
        while(fscanf(file,"%u,%u,%u",&st,&end,&cost)!=EOF){
            if(id2idx.find(st) == id2idx.end()) {
            	id2idx[st] = idx++;
            	idx2id[idx - 1] = st;
            	st = idx - 1;
            } else {
            	st = id2idx[st];
            }
            if(id2idx.find(end) == id2idx.end()) {
            	id2idx[end] = idx++;
            	idx2id[idx - 1] = end;
            	end = idx - 1;
            } else {
            	end = id2idx[end];
            }
            while(fg.size() < idx) {
            	vector<int> temp;
            	fg.push_back(temp);
            }
            fg[st].push_back(idx2id[end]);
            while(bg.size() < idx) {
            	vector<int> temp;
            	bg.push_back(temp);
            }
            bg[end].push_back(idx2id[st]);
        }
        idLen = idx;
        id = new int[idLen];
        for(int i = 0; i < idLen; i++) {
        	id[i] = idx2id[i];
        }
        gStart = new int[idLen];
        gStart[0] = 0;
        for(int i = 0; i < fg.size(); i++) {
        	gStart[i + 1] = fg[i].size() + gStart[i];
        }
        graph = new int[gStart[idLen - 1]];
        for(int i = 0; i < fg.size(); i++) {
        	sort(fg[i].begin(), fg[i].end());
        	for(int j = gStart[i]; j < gStart[i + 1]; j++) {
        		graph[j] = id2idx[fg[i][j - gStart[i]]];
        	}
        }
        agStart = new int[idLen];
        agStart[0] = 0;
        for(int i = 0; i < bg.size(); i++) {
        	agStart[i + 1] = bg[i].size() + agStart[i];
        }
        antigraph = new int[agStart[idLen - 1]];
        for(int i = 0; i < bg.size(); i++) {
        	sort(bg[i].begin(), bg[i].end());
        	for(int j = agStart[i]; j < agStart[i + 1]; j++) {
        		antigraph[j] = id2idx[bg[i][j - agStart[i]]];
        	}
        }
#if debug
       cout<<"graph size: "<<idx<<endl;
#endif
	}
	void saveAnswer(string filename) {
	    //mmap:https://blog.csdn.net/oceanperfect/article/details/52192602
#if debug
		printf("answer size　: %d\n", circle.size());
#endif
		ofstream out(filename);
		out<<circle.size()<<endl;
		for(int i = 0; i < circle.size(); i++) {
			out<<circle[i][0];
			for(int j = 1; j < circle[i].size(); j++) {
				out<<","<<circle[i][j];
			}
			out<<endl;
		}
	}

	void checkCircle() {
#if debug
		cout<<"checking circle..."<<endl;
#endif
		bool visit[idLen];
		memset(visit, false, idLen * sizeof(bool));
		int reachable[idLen];
		memset(reachable, -1, idLen * sizeof(int));
		for(int i = 0; i < idLen; i++) {
			path.push_back(i);
			prune_g(i, i, visit, reachable, 1);
			prune_ag(i, i, visit, reachable, 1);
			for(int j = agStart[i]; j < agStart[i + 1]; j++) {
				reachable[antigraph[j]] = -2;
			}
			dfs(i, i, visit, reachable);
			path.pop_back();
			for(int j = agStart[i]; j < agStart[i + 1]; j++) {
				reachable[antigraph[j]] = i;
			}
#if debug
			cout<<"point "<<id[i]<<" finished! " << "answer num : " << circle.size()<<endl;
#endif
		}
		sort(circle.begin(), circle.end(), [](vector<int> a,vector<int> b){
			if(a.size() != b.size()) {
				return a.size() < b.size();
			} else {
				return a[0] < b[0];
			}
		});
	}
};


int main() {
	string dataFileName = "test_data_28w.txt";
    string answerFileName = "answer.txt";

    auto t = clock();
    Solution s;
    s.loadData(dataFileName);
    cout<<"load data "<<(clock() - t) / 1000<<endl;
    s.checkCircle();
    cout<<"check circle "<<(clock() - t) / 1000<<endl;
    s.saveAnswer(answerFileName);
    cout<<"save answer "<<(clock() - t) / 1000<<endl;
	return 0;
}

```

```cpp
排序+二分查找 22s
#include <bits/stdc++.h>
using namespace std;

#define debug false

class Solution {
private:
	int *id, idLen;
	int *graph, *gStart;
	int *antigraph, *agStart;
	vector<vector<int>> circle;
	list<int> path;

	void dfs(int cur, int target, bool *visit, int *reachable) {
		int l = gStart[cur], r = gStart[cur + 1];
		while(l < r) {
			int m = l + (r - l) / 2;
			if(id[graph[m]] < id[target]) l = m + 1;
			else if(id[graph[m]] >= id[target]) r = m;
		}
		for(int i = l; i < gStart[cur + 1]; i++) {
			int t = graph[i];
			//if(id[t] < id[target]) continue;
			if(reachable[t] == -2 && !visit[t]) {
				path.push_back(t);
				if(path.size() > 2) {
					vector<int> temp;
					for(auto p : path) {
						temp.push_back(id[p]);
					}
					circle.push_back(temp);
				}
				path.pop_back();
			}
			if(visit[t] || (reachable[t] != target && reachable[t] != -2)) continue;
			if(path.size() == 6 || t == target) continue;
			visit[t] = true;
			path.push_back(t);
			dfs(t, target, visit, reachable);
			path.pop_back();
			visit[t] = false;
		}
	}
	void prune_g(int cur, int target, bool * visit, int *reachable, int distance) {
		int l = gStart[cur], r = gStart[cur + 1];
		while(l < r) {
			int m = l + (r - l) / 2;
			if(id[graph[m]] < id[target]) l = m + 1;
			else if(id[graph[m]] >= id[target]) r = m;
		}
		for(int i = l; i < gStart[cur + 1]; i++) {
			int t = graph[i];
			if(id[t] < id[target] || visit[t]) continue;
			reachable[t] = target;
			if(distance == 3) continue;
			visit[t] = true;
			prune_g(t, target, visit, reachable, distance + 1);
			visit[t] = false;
		}
	}
	void prune_ag(int cur, int target, bool * visit, int *reachable, int distance) {
		int l = agStart[cur], r = agStart[cur + 1];
		while(l < r) {
			int m = l + (r - l) / 2;
			if(id[antigraph[m]] < id[target]) l = m + 1;
			else if(id[antigraph[m]] >= id[target]) r = m;
		}
		for(int i = l; i < agStart[cur + 1]; i++) {
			int t = antigraph[i];
			if(id[t] < id[target] || visit[t]) continue;
			reachable[t] = target;
			if(distance == 3) continue;
			visit[t] = true;
			prune_ag(t, target, visit, reachable, distance + 1);
			visit[t] = false;
		}
	}

public:
	void loadData(string filename) {
        FILE* file=fopen(filename.c_str(),"r");
        int st, end, cost;
        int idx = 0;
    	unordered_map<int, int> id2idx;
        unordered_map<int, int> idx2id;
        vector<vector<int>> fg;
        vector<vector<int>> bg;
        while(fscanf(file,"%u,%u,%u",&st,&end,&cost)!=EOF){
            if(id2idx.find(st) == id2idx.end()) {
            	id2idx[st] = idx++;
            	idx2id[idx - 1] = st;
            	st = idx - 1;
            } else {
            	st = id2idx[st];
            }
            if(id2idx.find(end) == id2idx.end()) {
            	id2idx[end] = idx++;
            	idx2id[idx - 1] = end;
            	end = idx - 1;
            } else {
            	end = id2idx[end];
            }
            while(fg.size() < idx) {
            	vector<int> temp;
            	fg.push_back(temp);
            }
            fg[st].push_back(idx2id[end]);
            while(bg.size() < idx) {
            	vector<int> temp;
            	bg.push_back(temp);
            }
            bg[end].push_back(idx2id[st]);
        }
        idLen = idx;
        id = new int[idLen];
        for(int i = 0; i < idLen; i++) {
        	id[i] = idx2id[i];
        }
        gStart = new int[idLen];
        gStart[0] = 0;
        for(int i = 0; i < fg.size(); i++) {
        	gStart[i + 1] = fg[i].size() + gStart[i];
        }
        graph = new int[gStart[idLen - 1]];
        for(int i = 0; i < fg.size(); i++) {
        	sort(fg[i].begin(), fg[i].end());
        	for(int j = gStart[i]; j < gStart[i + 1]; j++) {
        		graph[j] = id2idx[fg[i][j - gStart[i]]];
        	}
        }
        agStart = new int[idLen];
        agStart[0] = 0;
        for(int i = 0; i < bg.size(); i++) {
        	agStart[i + 1] = bg[i].size() + agStart[i];
        }
        antigraph = new int[agStart[idLen - 1]];
        for(int i = 0; i < bg.size(); i++) {
        	sort(bg[i].begin(), bg[i].end());
        	for(int j = agStart[i]; j < agStart[i + 1]; j++) {
        		antigraph[j] = id2idx[bg[i][j - agStart[i]]];
        	}
        }
#if debug
       cout<<"graph size: "<<idx<<endl;
#endif
	}
	void saveAnswer(string filename) {
	    //mmap:https://blog.csdn.net/oceanperfect/article/details/52192602
#if debug
		printf("answer size　: %d\n", circle.size());
#endif
		ofstream out(filename);
		out<<circle.size()<<endl;
		for(int i = 0; i < circle.size(); i++) {
			out<<circle[i][0];
			for(int j = 1; j < circle[i].size(); j++) {
				out<<","<<circle[i][j];
			}
			out<<endl;
		}
	}

	void checkCircle() {
#if debug
		cout<<"checking circle..."<<endl;
#endif
		bool visit[idLen];
		memset(visit, false, idLen * sizeof(bool));
		int reachable[idLen];
		memset(reachable, -1, idLen * sizeof(int));
		for(int i = 0; i < idLen; i++) {
			path.push_back(i);
			prune_g(i, i, visit, reachable, 1);
			prune_ag(i, i, visit, reachable, 1);
			for(int j = agStart[i]; j < agStart[i + 1]; j++) {
				reachable[antigraph[j]] = -2;
			}
			dfs(i, i, visit, reachable);
			path.pop_back();
			for(int j = agStart[i]; j < agStart[i + 1]; j++) {
				reachable[antigraph[j]] = i;
			}
#if debug
			cout<<"point "<<id[i]<<" finished! " << "answer num : " << circle.size()<<endl;
#endif
		}
		sort(circle.begin(), circle.end(), [](vector<int> a,vector<int> b){
			if(a.size() != b.size()) {
				return a.size() < b.size();
			} else {
				return a[0] < b[0];
			}
		});
	}
};


int main() {
	string dataFileName = "test_data_28w.txt";
    string answerFileName = "answer.txt";

    auto t = clock();
    Solution s;
    s.loadData(dataFileName);
    cout<<"load data "<<(clock() - t) / 1000<<endl;
    s.checkCircle();
    cout<<"check circle "<<(clock() - t) / 1000<<endl;
    s.saveAnswer(answerFileName);
    cout<<"save answer "<<(clock() - t) / 1000<<endl;
	return 0;
}

```

