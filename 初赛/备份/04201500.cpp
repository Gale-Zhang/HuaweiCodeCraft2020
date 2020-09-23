#include <bits/stdc++.h>
// #include <sys/mman.h>
// #include <sys/types.h>
// #include <fcntl.h>

using namespace std;


class ThreadPool {
public:
    ThreadPool(size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
 
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            }
        );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

bool cmp(int a, int b) {
    return a < b;
}

int *id, idLen, tid_count;
int *ll;
int *graph, *gStart;
int *antigraph, *agStart;

bool *visit4;
int *reachable4;
unordered_map<thread::id, int> tid_map;

queue<vector<int>> bucket[5][4];
list<int> path[4];

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

    visit4 = new bool[4 * idLen];
    memset(visit4, false, sizeof(bool) * 4 * idLen);
    reachable4 = new int[4 * idLen];
    memset(reachable4, -1, sizeof(int) * 4 * idLen);
}



void saveAnswer(string filename) {
    auto t = clock();

    int num = 0;
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 4; j++) {
            num += bucket[i][j].size();
        }
    }

    // cout<<"data num : "<<num<<endl;
    // char *data = new char[num * 11 * 7];
    // int offset = sprintf(data, "%d\n", num);
    // cout<<"time "<<(clock() - t) / 1000<<endl;

    FILE *fp = fopen(filename.c_str(), "w");
    char data[12];
    int len = sprintf(data, "%d\n", num);
    fwrite(data, sizeof(char), len, fp);

    for(int i = 0; i < 5; i++) {
        queue<vector<int>> q1, q2;
        while(!bucket[i][0].empty() && !bucket[i][1].empty()) {
            if(bucket[i][0].front()[0] < bucket[i][1].front()[0]) {
                q1.push(bucket[i][0].front());
                bucket[i][0].pop();
            } else {
                q1.push(bucket[i][1].front());
                bucket[i][1].pop();
            }
        }
        while(!bucket[i][0].empty()) {
            q1.push(bucket[i][0].front());
            bucket[i][0].pop();
        }
        while(!bucket[i][1].empty()) {
            q1.push(bucket[i][1].front());
            bucket[i][1].pop();
        }
        while(!bucket[i][2].empty() && !bucket[i][3].empty()) {
            if(bucket[i][2].front()[0] < bucket[i][3].front()[0]) {
                q2.push(bucket[i][2].front());
                bucket[i][2].pop();
            } else {
                q2.push(bucket[i][3].front());
                bucket[i][3].pop();
            }
        }
        while(!bucket[i][2].empty()) {
            q2.push(bucket[i][2].front());
            bucket[i][2].pop();
        }
        while(!bucket[i][3].empty()) {
            q2.push(bucket[i][3].front());
            bucket[i][3].pop();
        }
        while(!q1.empty() && !q2.empty()) {
            if(q1.front()[0] < q2.front()[0]) {
                int l = q1.front().size() - 1;
                for(int j = 0; j < l; j++) {
                    len = sprintf(data, "%d\n", q1.front()[j]);
                    fwrite(data, sizeof(char), len, fp);
                }
                len = sprintf(data, "%d\n", q1.front()[l]);
                fwrite(data, sizeof(char), len, fp);
                q1.pop();
            } else {
                int l = q2.front().size() - 1;
                for(int j = 0; j < l; j++) {
                    len = sprintf(data, "%d\n", q2.front()[j]);
                    fwrite(data, sizeof(char), len, fp);
                }
                len = sprintf(data, "%d\n", q2.front()[l]);
                fwrite(data, sizeof(char), len, fp);
                q2.pop();
            }
        }
        while(!q1.empty()) {
            int l = q1.front().size() - 1;
            for(int j = 0; j < l; j++) {
                len = sprintf(data, "%d\n", q1.front()[j]);
                fwrite(data, sizeof(char), len, fp);
            }
            len = sprintf(data, "%d\n", q1.front()[l]);
            fwrite(data, sizeof(char), len, fp);
            q1.pop();
        }
        while(!q2.empty()) {
            int l = q2.front().size() - 1;
            for(int j = 0; j < l; j++) {
                len = sprintf(data, "%d\n", q2.front()[j]);
                fwrite(data, sizeof(char), len, fp);
            }
            len = sprintf(data, "%d\n", q2.front()[l]);
            fwrite(data, sizeof(char), len, fp);
            q2.pop();            
        }
    }

    fclose(fp);
    cout<<"time: "<<(clock() - t)/1000<<endl;


    // auto t = clock();

    // int num = 0;
    // for(int i = 0; i < 5; i++) {
    //     for(int j = 0; j < 4; j++) {
    //         num += bucket[i][j].size();
    //     }
    // }

    // cout<<"data num : "<<num<<endl;
    // char *data = new char[num * 11 * 7];
    // int offset = sprintf(data, "%d\n", num);
    // cout<<"time "<<(clock() - t) / 1000<<endl;

    // for(int i = 0; i < 5; i++) {
    //     queue<vector<int>> q1, q2;
    //     while(!bucket[i][0].empty() && !bucket[i][1].empty()) {
    //         if(bucket[i][0].front()[0] < bucket[i][1].front()[0]) {
    //             q1.push(bucket[i][0].front());
    //             bucket[i][0].pop();
    //         } else {
    //             q1.push(bucket[i][1].front());
    //             bucket[i][1].pop();
    //         }
    //     }
    //     while(!bucket[i][0].empty()) {
    //         q1.push(bucket[i][0].front());
    //         bucket[i][0].pop();
    //     }
    //     while(!bucket[i][1].empty()) {
    //         q1.push(bucket[i][1].front());
    //         bucket[i][1].pop();
    //     }
    //     while(!bucket[i][2].empty() && !bucket[i][3].empty()) {
    //         if(bucket[i][2].front()[0] < bucket[i][3].front()[0]) {
    //             q2.push(bucket[i][2].front());
    //             bucket[i][2].pop();
    //         } else {
    //             q2.push(bucket[i][3].front());
    //             bucket[i][3].pop();
    //         }
    //     }
    //     while(!bucket[i][2].empty()) {
    //         q2.push(bucket[i][2].front());
    //         bucket[i][2].pop();
    //     }
    //     while(!bucket[i][3].empty()) {
    //         q2.push(bucket[i][3].front());
    //         bucket[i][3].pop();
    //     }
    //     while(!q1.empty() && !q2.empty()) {
    //         if(q1.front()[0] < q2.front()[0]) {
    //             int l = q1.front().size() - 1;
    //             for(int j = 0; j < l; j++) {
    //                 offset += sprintf(data + offset, "%d,", q1.front()[j]);
    //             }
    //             offset += sprintf(data + offset, "%d\n", q1.front()[l]);
    //             q1.pop();
    //         } else {
    //             int l = q2.front().size() - 1;
    //             for(int j = 0; j < l; j++) {
    //                 offset += sprintf(data + offset, "%d,", q2.front()[j]);
    //             }
    //             offset += sprintf(data + offset, "%d\n", q2.front()[l]);
    //             q2.pop();
    //         }
    //     }
    //     while(!q1.empty()) {
    //         int l = q1.front().size() - 1;
    //         for(int j = 0; j < l; j++) {
    //             offset += sprintf(data + offset, "%d,", q1.front()[j]);
    //         }
    //         offset += sprintf(data + offset, "%d\n", q1.front()[l]);
    //         q1.pop();
    //     }
    //     while(!q2.empty()) {
    //         int l = q2.front().size() - 1;
    //         for(int j = 0; j < l; j++) {
    //             offset += sprintf(data + offset, "%d,", q2.front()[j]);
    //         }
    //         offset += sprintf(data + offset, "%d\n", q2.front()[l]);
    //         q2.pop();            
    //     }
    // }

    // cout<<"write data finished "<<(clock() - t) / 1000<<" offset "<<offset<<endl;

    // int fd = open(filename.c_str(), O_CREAT|O_RDWR, 0666);
    // lseek(fd, 0, SEEK_END);
    // write(fd, "\0", offset);
    // close(fd);

    // cout<<"initial file finished "<<(clock() - t) / 1000<<endl;

    // fd = open(filename.c_str(), O_CREAT|O_RDWR, 0666);

    // cout<<"mmap finished "<<(clock() - t) / 1000<<endl;

    // int block = 2 * 1024 * 1024; //2M
    // int cur = 0;
    // while(cur + block < offset) {
    //     char *buffer = (char*)mmap(NULL, block, PROT_READ|PROT_WRITE, MAP_SHARED, fd, cur);
    //     cout<<0<<endl;
    //     memcpy(buffer, data + cur, block);
    //     cout<<1<<endl;
    //     munmap(buffer, block);
    //     cout<<2<<endl;
    //     cur += block;
    //     cout<<cur<<endl;
    // }
    // cout<<"3"<<endl;
    // char *buffer = (char*)mmap(NULL, block, PROT_READ|PROT_WRITE, MAP_SHARED, fd, cur);
    // memcpy(buffer, data + cur, offset - cur);
    // munmap(buffer, block);

    // close(fd);

    // cout<<"write file finished "<<(clock() - t) / 1000<<endl;













    // int fd = open(filename.c_str(), O_CREAT|O_RDWR|O_APPEND, 0666);
    // if(fd < 0) {
    //     cout<<"OPEN FILE ERROR!"<<endl;
    // }

    // char data[12];
    // int bufLen = 4096;//2 * 1024 * 1024;

    // int len = sprintf(data, "%d\n", bucket[0][0].size() + bucket[1][0].size() + bucket[2][0].size() + bucket[3][0].size() + bucket[4][0].size());

    // int count = 0;

    // int bucIdx = 0;
    // int thdIdx = 0;
    // int rowIdx = 0;
    // int colIdx = 0; 

    // while(bucIdx < 5) {

    //     lseek(fd, 0, SEEK_END);
    //     write(fd, "\0", bufLen);

    //     char *buffer = (char*)mmap(NULL, bufLen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, count * bufLen);
    //     memcpy(buffer, data, len);
    //     memset(data, '\0', len);

    //     int offset = len;
        
    //     while(bucIdx < 5) {
    //         if(bufLen - offset > 12) {
    //             len = sprintf(buffer + offset, "%d", bucket[bucIdx][0][rowIdx][colIdx]);
    //             offset += len;  
    //             colIdx++;
    //             if(colIdx == bucket[bucIdx][0][rowIdx].size()) {
    //                 buffer[offset++] = '\n';
    //                 colIdx = 0;
    //                 rowIdx++;
    //                 if(rowIdx == bucket[bucIdx][0].size()) {
    //                     rowIdx = 0;
    //                     bucIdx++;
    //                 }
    //             } else {
    //                 buffer[offset++] = ',';
    //             }
    //         } else {
    //             len = sprintf(data, "%d", bucket[bucIdx][0][rowIdx][colIdx]);   
    //             colIdx++;
    //             if(colIdx == bucket[bucIdx][0][rowIdx].size()) {
    //                 data[len++] = '\n';
    //                 colIdx = 0;
    //                 rowIdx++;
    //                 if(rowIdx == bucket[bucIdx][0].size()) {
    //                     rowIdx = 0;
    //                     bucIdx++;
    //                 }
    //             } else {
    //                 data[len++] = ',';
    //             }
    //             if(offset + len <= bufLen) {
    //                 memcpy(buffer + offset, data, len);
    //                 offset += len;
    //             } else {
    //                 memcpy(buffer + offset, data, bufLen - offset);
    //                 len = offset + len - bufLen;
    //                 for(int i = 0; i < len; i++) {
    //                     data[i] = data[i + bufLen - offset];
    //                 }
    //                 count++;
    //                 break;
    //             }
    //         }
    //     }
    //     munmap(buffer, bufLen);
    // }
    // close(fd);
}

void dfs(int cur, int target, bool *visit, int *reachable, int tidx) {
    for(int i = gStart[cur]; i < gStart[cur + 1]; i++) {
        int t = graph[i];
        if(id[t] < id[target]) continue;
        if(reachable[t] == -2 && !visit[t]) {
            path[tidx].push_back(t);
            if(path[tidx].size() > 2) {
                vector<int> temp;
                for(auto p : path[tidx]) {
                    temp.push_back(id[p]);
                }
                bucket[temp.size() - 3][tidx].push(temp);
            }
            path[tidx].pop_back();
        }
        if(visit[t] || (reachable[t] != target && reachable[t] != -2)) continue;
        if(path[tidx].size() == 6 || t == target) continue;
        visit[t] = true;
        path[tidx].push_back(t);
        dfs(t, target, visit, reachable, tidx);
        path[tidx].pop_back();
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

void check(int tidx, int ii) {
    bool *visit = visit4 + tidx * idLen;
    int *reachable = reachable4 + tidx * idLen;
    //cout<<ii<<" "<<tidx<<endl;
    int i = ll[ii];
    path[tidx].push_back(i);
    prune_g(i, i, visit, reachable, 1);
    prune_ag(i, i, visit, reachable, 1);
    for(int j = agStart[i]; j < agStart[i + 1]; j++) {
        reachable[antigraph[j]] = -2;
    }
    dfs(i, i, visit, reachable, tidx);
    path[tidx].pop_back();
    for(int j = agStart[i]; j < agStart[i + 1]; j++) {
        reachable[antigraph[j]] = i;
    }
}

void checkCircle() {
    tid_count = 0;
    ThreadPool pool(4);
    for(int i = 0; i < idLen; i++) {
        pool.enqueue([i] {
            thread::id tid = this_thread::get_id();
            int tidx = 0;
            if(tid_map.find(tid) == tid_map.end()) {
                tidx = tid_count;
                tid_map[tid] = tid_count++;
            } else {
                tidx = tid_map[tid];
            }
            check(tidx, i);
        });
    }
}

int main(int argc,char* argv[]) {

    string dataFileName = argv[1];
    string answerFileName = "answer_cpp.txt";

    timeval start, end; 
    gettimeofday(&start, NULL);

    loadData(dataFileName);
    gettimeofday(&end, NULL);
    cout<<"load data "<<1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000<<endl;

    checkCircle();
    gettimeofday(&end, NULL);
    cout<<"check circle "<<1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000<<endl;

    saveAnswer(answerFileName);
    gettimeofday(&end, NULL);
    cout<<"save answer "<<1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000<<endl;

    return 0;
}
