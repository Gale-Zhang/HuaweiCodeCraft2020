#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <pthread.h>

using namespace std;
#define T_NUM 1

#define debug false

bool cmp(int a, int b) {
	return a < b;
}

// class Solution {

// private:
	int *id, idLen;
	int *ll;
	int *graph, *gStart;
	int *antigraph, *agStart;
	vector<vector<int>> bucket[5][T_NUM];
	unordered_map<pthread_t, int> tid;
	list<int> path;

// public:
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

        ll = new int[idLen];
        int temp = 0;
        for(auto i = id2idx.begin(); i != id2idx.end(); i++) {
        	ll[temp++] = i->first;
        }
        sort(ll, ll + idLen, cmp);
        for(int i = 0; i < idLen; i++) {
        	ll[i] = id2idx[ll[i]];
        }

        gStart = new int[idLen];
        gStart[0] = 0;
        for(int i = 0; i < fg.size(); i++) {
        	gStart[i + 1] = fg[i].size() + gStart[i];
        }
        graph = new int[gStart[idLen - 1]];
        for(int i = 0; i < fg.size(); i++) {
        	sort(fg[i].begin(), fg[i].end(), cmp);
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
        	sort(bg[i].begin(), bg[i].end(), cmp);
        	for(int j = agStart[i]; j < agStart[i + 1]; j++) {
        		antigraph[j] = id2idx[bg[i][j - agStart[i]]];
        	}
        }
	}

	void saveAnswer(string filename) {
		auto t = clock();

	    int fd = open(filename.c_str(), O_CREAT|O_RDWR|O_APPEND, 00777);
	    if(fd < 0) {
	        cout<<"OPEN FILE ERROR!"<<endl;
	    }

	    char data[12];
	    int bufLen = 4096;//2 * 1024 * 1024;

		int len = sprintf(data, "%d\n", bucket[0][0].size() + bucket[1][0].size() + bucket[2][0].size() + bucket[3][0].size() + bucket[4][0].size());

		int count = 0;

		int bucIdx = 0;
		int thdIdx = 0;
	    int rowIdx = 0;
	    int colIdx = 0;	

	    while(bucIdx < 5) {

		    lseek(fd, 0, SEEK_END);
		    write(fd, "\0", bufLen);

		    char *buffer = (char*)mmap(NULL, bufLen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, count * bufLen);
		    memcpy(buffer, data, len);
		    memset(data, '\0', len);

		    int offset = len;
		    
		    while(bucIdx < 5) {
		    	if(bufLen - offset > 12) {
			    	len = sprintf(buffer + offset, "%d", bucket[bucIdx][0][rowIdx][colIdx]);
			    	offset += len;	
			    	colIdx++;
			    	if(colIdx == bucket[bucIdx][0][rowIdx].size()) {
			    		buffer[offset++] = '\n';
			    		colIdx = 0;
			    		rowIdx++;
			    		if(rowIdx == bucket[bucIdx][0].size()) {
			    			rowIdx = 0;
			    			bucIdx++;
			    		}
			    	} else {
			    		buffer[offset++] = ',';
			    	}
		    	} else {
		    		len = sprintf(data, "%d", bucket[bucIdx][0][rowIdx][colIdx]);	
			    	colIdx++;
			    	if(colIdx == bucket[bucIdx][0][rowIdx].size()) {
			    		data[len++] = '\n';
			    		colIdx = 0;
			    		rowIdx++;
			    		if(rowIdx == bucket[bucIdx][0].size()) {
			    			rowIdx = 0;
			    			bucIdx++;
			    		}
			    	} else {
			    		data[len++] = ',';
			    	}
		    		if(offset + len <= bufLen) {
		    			memcpy(buffer + offset, data, len);
		    			offset += len;
		    		} else {
		    			memcpy(buffer + offset, data, bufLen - offset);
		    			len = offset + len - bufLen;
		    			for(int i = 0; i < len; i++) {
		    				data[i] = data[i + bufLen - offset];
		    			}
		    			count++;
		    			break;
		    		}
		    	}
		    }
		    munmap(buffer, bufLen);
		}
	    close(fd);
	}

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
					bucket[temp.size() - 3][0].push_back(temp);
					//bucket[temp.size() - 3][tid[pthread_self()]].push_back(temp);
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
			if(id[t] < id[target]) continue;
			if(visit[t]) continue;
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
			if(id[t] < id[target]) continue;
			if(visit[t]) continue;
			reachable[t] = target;
			if(distance == 3) continue;
			visit[t] = true;
			prune_ag(t, target, visit, reachable, distance + 1);
			visit[t] = false;
		}
	}

	// void* check(void* args) {
	// 	bool visit[idLen];
	// 	memset(visit, false, idLen * sizeof(bool));
	// 	int reachable[idLen];
	// 	memset(reachable, -1, idLen * sizeof(int));
	// 	int t = tid[pthread_self()];
	// 	int st = t * idLen / T_NUM;
	// 	int end = t == (T_NUM - 1 ) ? idLen : t * idLen / T_NUM;
	// 	for(int ii = st; ii < end; ii++) {
	// 		int i = ll[ii];
	// 		path.push_back(i);
	// 		prune_g(i, i, visit, reachable, 1);
	// 		prune_ag(i, i, visit, reachable, 1);
	// 		for(int j = agStart[i]; j < agStart[i + 1]; j++) {
	// 			reachable[antigraph[j]] = -2;
	// 		}
	// 		dfs(i, i, visit, reachable);
	// 		path.pop_back();
	// 		for(int j = agStart[i]; j < agStart[i + 1]; j++) {
	// 			reachable[antigraph[j]] = i;
	// 		}
	// 	}
	// }

	// void checkCircle() {
	// 	pthread_t tids[T_NUM];
	// 	for(int i = 0; i < T_NUM; i++) {
	// 		if(pthread_create(&tids[i], NULL, check, NULL) != 0) {
	// 			cout<<"create thread "<<i<<" error!";
	// 		}
	// 	}
	// 	pthread_exit(NULL);
	// }
	void checkCircle() {
		bool visit[idLen];
		memset(visit, false, idLen * sizeof(bool));
		int reachable[idLen];
		memset(reachable, -1, idLen * sizeof(int));
		for(int ii = 0; ii < idLen; ii++) {
			int i = ll[ii];
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
		}
	}
// };


int main() {
	string dataFileName = "test_data_28w.txt";
    string answerFileName = "answer.txt";

    auto t = clock();
    // Solution s;
    loadData(dataFileName);
    cout<<"load data "<<(clock() - t) / 1000<<endl;
    checkCircle();
    cout<<"check circle "<<(clock() - t) / 1000<<endl;
    saveAnswer(answerFileName);
    cout<<"save answer "<<(clock() - t) / 1000<<endl;
	return 0;
}
