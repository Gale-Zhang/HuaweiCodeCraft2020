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
int *ll, *rr;
int *graph, *gStart;
int *antigraph, *agStart;

bool *visit4;
int *reachable4;
unordered_map<thread::id, int> tid_map;

vector<vector<int>> *bucket;
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

    bucket = new vector<vector<int>>[idLen * 5];
}



void saveAnswer(string filename) {
    auto t = clock();

    int num = 0;
    for(int i = 0; i < 5 * idLen; i++) {
        num += bucket[i].size();
    }

    FILE *fp = fopen(filename.c_str(), "w");
    char data[12];
    int len = sprintf(data, "%d\n", num);
    fwrite(data, sizeof(char), len, fp);

    for(int i = 0; i < 5 * idLen; i++) {
        for(int j = 0; j < bucket[i].size(); j++) {
            int l = bucket[i][j].size() - 1;
            for(int k = 0; k < l; k++) {
                len = sprintf(data, "%d,", bucket[i][j][k]);
                fwrite(data, sizeof(char), len, fp);
            }
            len = sprintf(data, "%d\n", bucket[i][j][l]);
            fwrite(data, sizeof(char), len, fp);
        }
    }

    fclose(fp);
    cout<<"time: "<<(clock() - t)/1000<<endl;
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
                bucket[target + idLen * (temp.size() - 3)].push_back(temp);
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
            if(tidx >= 4) {
                cout<<"error"<<endl;
            } else {
                check(tidx, i);
            }
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
