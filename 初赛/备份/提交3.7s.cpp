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
    return a > b;
}

struct Graph {
    int pnum;   //点数
    int *id;    //映射：索引 -> id
    int *ll;
    int *rr;    //索引号对应的排序位置
    int *fg;    //正向图
    int *bg;    //反向图
    int *fgp;   //正向图区间
    int *bgp;   //反向图区间
} graph;

struct Mem {
    mutex lock;
    int tcount;
    bool *visit4;
    int *reachable4;
    int *path4;
    unordered_map<thread::id, int> tidx;
} mem;

struct AnswerBuffer{
    int *ans[5][4];
    int len[4][5];
    int maxLen[5];
} ab;

void loadData(string filename) {

    FILE* file=fopen(filename.c_str(),"r");
    int st, end, cost;
    int idx = 0;

    unordered_map<int, int> id2idx;
    unordered_map<int, int> idx2id;
    vector<vector<int>> fg;
    vector<vector<int>> bg;

    while(fscanf(file, "%u,%u,%u", &st, &end, &cost) != EOF){
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


    //Graph Init
    graph.pnum = idx;
    graph.id = new int[graph.pnum];
    for(int i = 0; i < graph.pnum; i++) {
        graph.id[i] = idx2id[i];
    }

    graph.ll = new int[graph.pnum];
    graph.rr = new int[graph.pnum];
    int temp = 0;
    for(auto i = id2idx.begin(); i != id2idx.end(); i++) {
        graph.ll[temp++] = i->first;
    }
    sort(graph.ll, graph.ll + graph.pnum);
    for(int i = 0; i < graph.pnum; i++) {
        graph.ll[i] = id2idx[graph.ll[i]];
        graph.rr[graph.ll[i]] = i;
    }

    graph.fgp = new int[graph.pnum + 1];
    graph.fgp[0] = 0;

    for(int i = 0; i < fg.size(); i++) {
        graph.fgp[i + 1] = fg[i].size() + graph.fgp[i];
    }

    graph.fg = new int[graph.fgp[graph.pnum]];
    for(int i = 0; i < fg.size(); i++) {
        sort(fg[i].begin(), fg[i].end(), cmp);
        for(int j = graph.fgp[i]; j < graph.fgp[i + 1]; j++) {
            graph.fg[j] = id2idx[fg[i][j - graph.fgp[i]]];
        }
    }

    graph.bgp = new int[graph.pnum + 1];
    graph.bgp[0] = 0;
    for(int i = 0; i < bg.size(); i++) {
        graph.bgp[i + 1] = bg[i].size() + graph.bgp[i];
    }

    graph.bg = new int[graph.bgp[graph.pnum]];
    for(int i = 0; i < bg.size(); i++) {
        sort(bg[i].begin(), bg[i].end(), cmp);
        for(int j = graph.bgp[i]; j < graph.bgp[i + 1]; j++) {
            graph.bg[j] = id2idx[bg[i][j - graph.bgp[i]]];
        }
    }

    //Mem Init
    mem.visit4 = new bool[graph.pnum * 4];
    mem.reachable4 = new int[graph.pnum * 4];
    memset(mem.visit4, false, 4 * graph.pnum);
    memset(mem.reachable4, -1, 4 * graph.pnum);//初始化的长度是否正确？
    mem.tcount = 0;
    mem.path4 = new int[4 * 7];
    memset(mem.path4, 0, 4 * 7);

    //Init answer buffer
    for(int i = 0; i < 4; i++) {
        ab.ans[0][i] = new int[3 * 500000 * 4];
        ab.ans[1][i] = new int[4 * 500000 * 4];
        ab.ans[2][i] = new int[5 * 1000000 * 4];
        ab.ans[3][i] = new int[6 * 2000000 * 4];
        ab.ans[4][i] = new int[7 * 3000000 * 4];
        memset(ab.len[i], 0, 5 * sizeof(int));
    }
    ab.maxLen[0] = 500000;
    ab.maxLen[1] = 500000;
    ab.maxLen[2] = 1000000;
    ab.maxLen[3] = 2000000;
    ab.maxLen[4] = 3000000;
}

void saveAnswer(string filename) {
    int num = 0;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 5; j++) {
            num += ab.len[i][j] / (j + 3);
        }
    }

    FILE *fp = fopen(filename.c_str(), "w");
    char data[12];
    int len = sprintf(data, "%d\n", num);
    fwrite(data, sizeof(char), len, fp);

    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 4; j++) {
            ab.ans[i][j][ab.len[j][i]] = 2147483647;
        }
    }

    for(int i = 0; i < 5; i++) {
        int l = i + 3;
        int *st = new int[4]{0,0,0,0};
        while(st[0] < ab.len[0][i] || st[1] < ab.len[1][i] || st[2] < ab.len[2][i] || st[3] < ab.len[3][i]) {
            //cout<<ab.ans[i][0][st[0]]<<" "<<ab.ans[i][1][st[1]]<<" "<<ab.ans[i][2][st[2]]<<" "<<ab.ans[i][3][st[3]]<<" "<<endl;
            if(st[0] < ab.len[0][i] && ab.ans[i][0][st[0]] < ab.ans[i][1][st[1]] && ab.ans[i][0][st[0]] < ab.ans[i][2][st[2]] 
                && ab.ans[i][0][st[0]] < ab.ans[i][3][st[3]]) {
                //cout<<0<<endl;
                int first = st[0];
                while(first + l < ab.len[0][i] && ab.ans[i][0][first + l] == ab.ans[i][0][first]) first += l;
                for(int j = first; j >= st[0]; j -= l) {
                    int last = j + l - 1;
                    for(int m = j; m < last; m++) {
                        len = sprintf(data, "%d,", ab.ans[i][0][m]);
                        fwrite(data, sizeof(char), len, fp);
                    }
                    len = sprintf(data, "%d\n", ab.ans[i][0][last]);
                    fwrite(data, sizeof(char), len, fp);
                }
                st[0] = first + l;
            } else if(st[1] < ab.len[1][i] && ab.ans[i][1][st[1]] < ab.ans[i][0][st[0]] && ab.ans[i][1][st[1]] < ab.ans[i][2][st[2]]
                && ab.ans[i][1][st[1]] < ab.ans[i][3][st[3]]) {
                //cout<<1<<endl;
                int first = st[1];
                while(first + l < ab.len[1][i] && ab.ans[i][1][first + l] == ab.ans[i][1][first]) first += l;
                for(int j = first; j >= st[1]; j -= l) {
                    int last = j + l - 1;
                    for(int m = j; m < last; m++) {
                        len = sprintf(data, "%d,", ab.ans[i][1][m]);
                        fwrite(data, sizeof(char), len, fp);
                    }
                    len = sprintf(data, "%d\n", ab.ans[i][1][last]);
                    fwrite(data, sizeof(char), len, fp);
                }
                st[1] = first + l;                
            } else if(st[2] < ab.len[2][i] && ab.ans[i][2][st[2]] < ab.ans[i][0][st[0]] && ab.ans[i][2][st[2]] < ab.ans[i][1][st[1]]
                && ab.ans[i][2][st[2]] < ab.ans[i][3][st[3]]) {
                //cout<<2<<endl;
                int first = st[2];
                while(first + l < ab.len[2][i] && ab.ans[i][2][first + l] == ab.ans[i][2][first]) first += l;
                for(int j = first; j >= st[2]; j -= l) {
                    int last = j + l - 1;
                    for(int m = j; m < last; m++) {
                        len = sprintf(data, "%d,", ab.ans[i][2][m]);
                        fwrite(data, sizeof(char), len, fp);
                    }
                    len = sprintf(data, "%d\n", ab.ans[i][2][last]);
                    fwrite(data, sizeof(char), len, fp);
                }
                st[2] = first + l;
            } else {
                //cout<<3<<endl;
                int first = st[3];
                while(first + l < ab.len[3][i] && ab.ans[i][3][first + l] == ab.ans[i][3][first]) first += l;
                for(int j = first; j >= st[3]; j -= l) {
                    int last = j + l - 1;
                    for(int m = j; m < last; m++) {
                        len = sprintf(data, "%d,", ab.ans[i][3][m]);
                        fwrite(data, sizeof(char), len, fp);
                    }
                    len = sprintf(data, "%d\n", ab.ans[i][3][last]);
                    fwrite(data, sizeof(char), len, fp);
                }
                st[3] = first + l;
            }
        } 
    }

    fclose(fp);
}

void dfs(int cur, int target, bool *visit, int *reachable, int *path, int len, int idx) {
    for(int i = graph.fgp[cur]; i < graph.fgp[cur + 1]; i++) {
        int t = graph.fg[i];
        if(graph.id[t] < graph.id[target]) break;
        if(reachable[t] == -2 && !visit[t]) {
            path[len] = graph.id[t];
            if(len > 1) {
                int *start = ab.ans[len - 2][idx] + ab.len[idx][len - 2];
                memcpy(start, path, (len + 1) * sizeof(int));
                ab.len[idx][len - 2] += len + 1;
            }
        }
        if(visit[t] || (reachable[t] != target && reachable[t] != -2)) continue;
        if(len == 6 || t == target) continue;
        visit[t] = true;
        path[len] = graph.id[t];
        dfs(t, target, visit, reachable, path, len + 1, idx);
        visit[t] = false;
    }
}

void prune_fg(int cur, int target, bool * visit, int *reachable, int distance) {
    for(int i = graph.fgp[cur]; i < graph.fgp[cur + 1]; i++) {
        int t = graph.fg[i];
        if(graph.id[t] < graph.id[target]) break;
        if(visit[t]) continue;
        reachable[t] = target;
        if(distance == 3) continue;
        visit[t] = true;
        prune_fg(t, target, visit, reachable, distance + 1);
        visit[t] = false;
    }
}

void prune_bg(int cur, int target, bool *visit, int *reachable, int distance) {
    for(int i = graph.bgp[cur]; i < graph.bgp[cur + 1]; i++) {
        int t = graph.bg[i];
        if(graph.id[t] < graph.id[target]) break;
        if(visit[t]) continue;
        reachable[t] = target;
        if(distance == 3) continue;
        visit[t] = true;
        prune_bg(t, target, visit, reachable, distance + 1);
        visit[t] = false;
    }
}

void check4(int i) { 
    thread::id tid = this_thread::get_id();
    int idx = 0;
    if(mem.tidx.count(tid) == 0) {
        mem.lock.lock();
        if(mem.tidx.count(tid) == 0) {
            idx = mem.tcount;
            mem.tidx[tid] = mem.tcount++;
            //cout<<tid<<" "<<idx<<endl;
        }
        mem.lock.unlock();
    } else {
        idx = mem.tidx[tid];
    }
    bool *visit = mem.visit4 + idx * graph.pnum;
    int *reachable = mem.reachable4 + idx * graph.pnum;
    int *path = mem.path4 + idx * 7;
    path[0] = graph.id[i];
    prune_fg(i, i, visit, reachable, 1);
    prune_bg(i, i, visit, reachable, 1);
    for(int j = graph.bgp[i]; j < graph.bgp[i + 1]; j++) {
        reachable[graph.bg[j]] = -2;
    }
    dfs(i, i, visit, reachable, path, 1, idx);
    for(int j = graph.bgp[i]; j < graph.bgp[i + 1]; j++) {
        reachable[graph.bg[j]] = i;
    }
}

void check1() {
    bool *visit = mem.visit4;
    int *reachable = mem.reachable4;
    int *path = mem.path4;
    for(int ii = 0; ii < graph.pnum; ii++) {
        int i = graph.ll[ii];
        path[0] = graph.id[i];
        prune_fg(i, i, visit, reachable, 1);
        prune_bg(i, i, visit, reachable, 1);
        for(int j = graph.bgp[i]; j < graph.bgp[i + 1]; j++) {
            reachable[graph.bg[j]] = -2;
        }
        dfs(i, i, visit, reachable, path, 1, 0);
        for(int j = graph.bgp[i]; j < graph.bgp[i + 1]; j++) {
            reachable[graph.bg[j]] = i;
        } 
    }     
}

void checkCircle() {
    if(graph.pnum < 10000) {
        check1();
    } else {
        ThreadPool pool(4);
        for(int ii = 0; ii < graph.pnum; ii++) {
            int i = graph.ll[ii];
            pool.enqueue([i] {
                check4(i); 
            });
        }
    }
}

int main() {

    string dataFileName = "/data/test_data.txt";
    string answerFileName = "/projects/student/result.txt";

    loadData(dataFileName);
    checkCircle();
    saveAnswer(answerFileName);

    return 0;
}
