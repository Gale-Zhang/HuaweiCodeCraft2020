#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

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
    int lnum;   //边数
    int pnum;   //点数
    int list[280000];
    int fg[280000][50];    //正向图
    int bg[280000][50];    //反向图
    char fgl[280000];   //出度
    char bgl[280000];   //入度
    char strlen[280000]; //str长度
    char str[280000][10];  //映射：索引 -> str
} graph;

struct Mem {
    mutex lock;
    int tcount;
    bool *visit4;
    char *distance4;
    int *path4;
    int *update4;
    unordered_map<thread::id, int> tidx;
} mem;

struct AnswerBuffer{
    int *ans[5][4];
    int len[4][5];
} ab;

typedef struct stat Stat_t;

void loadData(string filename) {

    int idx = 0;
    memset(graph.fgl, 0, sizeof(char) * 280000);
    memset(graph.bgl, 0, sizeof(char) * 280000);
    memset(graph.strlen, 0, sizeof(char) * 280000);

    int fd = open(filename.c_str(), O_RDONLY);
    Stat_t s;
    fstat(fd, &s);
    int bufLen = s.st_size;

    char *buffer = (char*)mmap(NULL, bufLen, PROT_READ, MAP_PRIVATE, fd, 0);
    int l = 0;
    int r = 1;

    while(r <= bufLen) {
        if(buffer[r] == '\n') {
            int st = 0, end = 0;
            int cur = r;
            while(buffer[cur] != ',') cur--;
            cur--;
            int bit = 1;
            while(buffer[cur] != ',') {
                end += bit * (buffer[cur] - '0');
                bit *= 10;
                cur--;
            }
            cur--;
            bit = 1;
            while(cur >= l) {
                st += bit * (buffer[cur] - '0');
                bit *= 10;
                cur--;
            }

            graph.lnum++;

            if(graph.strlen[st] == 0) {
                graph.list[idx++] = st;
                int bit = 1000000000, len = 0, cur = st;
                while(bit > cur) bit /= 10;
                while(bit > 0) {
                    graph.str[st][len++] = '0' + (cur / bit);
                    cur %= bit;
                    bit /= 10;
                }
                graph.strlen[st] = len;

            } 
            if(graph.strlen[end] == 0) {
                graph.list[idx++] = end;
                int bit = 1000000000, len = 0, cur = end;
                while(bit > cur) bit /= 10;
                while(bit > 0) {
                    graph.str[end][len++] = '0' + (cur / bit);
                    cur %= bit;
                    bit /= 10;
                }
                graph.strlen[end] = len;

            }

            graph.fg[st][graph.fgl[st]++] = end;
            graph.bg[end][graph.bgl[end]++] = st;

            l = r + 1;
            r += 2;
        } else {
            r++;
        }
    }
    munmap(buffer, bufLen);
    close(fd);

    //Graph Init
    graph.pnum = idx;
    sort(graph.list, graph.list + idx);

    for(int ii = 0; ii < graph.pnum; ii++){
        int i = graph.list[ii];
        sort(graph.fg[i], graph.fg[i] + graph.fgl[i], cmp);
    }
    for(int ii = 0; ii < graph.pnum; ii++){
        int i = graph.list[ii];
        sort(graph.bg[i], graph.bg[i] + graph.bgl[i], cmp);
    }

}

void saveAnswer(string filename) {
    //auto t = clock();
    int num = 0;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 5; j++) {
            num += ab.len[i][j] / (j + 3);
        }
    }

    int BUFLEN = num * 7 * 11;
    char *data = (char*)malloc(BUFLEN);
    int offset = 0;

    int cur = num;
    int bit = 1000000000;
    while(bit > cur) bit /= 10;
    while(bit > 0) {
        data[offset++] = '0' + (cur / bit);
        cur %= bit;
        bit /= 10;
    }
    data[offset++] = '\n';

    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 4; j++) {
            ab.ans[i][j][ab.len[j][i]] = 280000;
        }
    }

    for(int i = 0; i < 5; i++) {
        int l = i + 3;
        int *st = new int[4]{0,0,0,0};
        while(st[0] < ab.len[0][i] || st[1] < ab.len[1][i] || st[2] < ab.len[2][i] || st[3] < ab.len[3][i]) {
            if(st[0] < ab.len[0][i] && ab.ans[i][0][st[0]] < ab.ans[i][1][st[1]] && ab.ans[i][0][st[0]] < ab.ans[i][2][st[2]]
                && ab.ans[i][0][st[0]] < ab.ans[i][3][st[3]]) {
                int first = st[0];
                while(first + l < ab.len[0][i] && ab.ans[i][0][first + l] == ab.ans[i][0][first]) first += l;
                for(int j = first; j >= st[0]; j -= l) {
                    for(int m = j; m < j + l; m++) {
                        cur = ab.ans[i][0][m];
                        memcpy(data + offset, graph.str[cur], graph.strlen[cur]);
                        offset += graph.strlen[cur];
                        data[offset++] = ',';
                    }
                    data[offset - 1] = '\n';
                }
                st[0] = first + l;
            } else if(st[1] < ab.len[1][i] && ab.ans[i][1][st[1]] < ab.ans[i][0][st[0]] && ab.ans[i][1][st[1]] < ab.ans[i][2][st[2]]
                && ab.ans[i][1][st[1]] < ab.ans[i][3][st[3]]) {
                int first = st[1];
                while(first + l < ab.len[1][i] && ab.ans[i][1][first + l] == ab.ans[i][1][first]) first += l;
                for(int j = first; j >= st[1]; j -= l) {
                    for(int m = j; m < j + l; m++) {
                        cur = ab.ans[i][1][m];
                        memcpy(data + offset, graph.str[cur], graph.strlen[cur]);
                        offset += graph.strlen[cur];
                        data[offset++] = ',';
                    }
                    data[offset - 1] = '\n';
                }
                st[1] = first + l;                
            } else if(st[2] < ab.len[2][i] && ab.ans[i][2][st[2]] < ab.ans[i][0][st[0]] && ab.ans[i][2][st[2]] < ab.ans[i][1][st[1]]
                && ab.ans[i][2][st[2]] < ab.ans[i][3][st[3]]) {
                int first = st[2];
                while(first + l < ab.len[2][i] && ab.ans[i][2][first + l] == ab.ans[i][2][first]) first += l;
                for(int j = first; j >= st[2]; j -= l) {
                    for(int m = j; m < j + l; m++) {
                        cur = ab.ans[i][2][m];
                        memcpy(data + offset, graph.str[cur], graph.strlen[cur]);
                        offset += graph.strlen[cur];
                        data[offset++] = ',';
                    }
                    data[offset - 1] = '\n';
                }
                st[2] = first + l;
            } else {
                int first = st[3];
                while(first + l < ab.len[3][i] && ab.ans[i][3][first + l] == ab.ans[i][3][first]) first += l;
                for(int j = first; j >= st[3]; j -= l) {
                    for(int m = j; m < j + l; m++) {
                        cur = ab.ans[i][3][m];
                        memcpy(data + offset, graph.str[cur], graph.strlen[cur]);
                        offset += graph.strlen[cur];
                        data[offset++] = ',';
                    }
                    data[offset - 1] = '\n';
                }
                st[3] = first + l;
            }
        } 
    }

    FILE *fp = fopen(filename.c_str(), "w");
    fwrite(data, sizeof(char), offset, fp);
    fclose(fp);
}

void prune(int cur, int target, bool *visit, char *distance, int dis, int *update, int &udlen) {
    for(int i = 0; i < graph.bgl[cur]; i++) {
        int t = graph.bg[cur][i];
        if(t < target) break;
        if(visit[t]) continue;
        if(dis < distance[t]) {
            distance[t] = dis;
            update[udlen++] = t;
        }

        if(dis == 3) continue;

        visit[t] = true;
        prune(t, target, visit, distance, dis + 1, update, udlen);
        visit[t] = false;
    }
}

void dfs(int cur, int target, bool *visit, char *distance, int *path, int len, int idx) {
    for(int i = 0; i < graph.fgl[cur]; i++) {
        int t = graph.fg[cur][i];

        if(t < target) break;

        if(visit[t] || (len > 3 && (len + distance[t] > 7))) continue;
        //180ms
        if(distance[t] == 1 && len > 1) {
            path[len] = t;
            int *start = ab.ans[len - 2][idx] + ab.len[idx][len - 2];
            memcpy(start, path, (len + 1) * sizeof(int));
            ab.len[idx][len - 2] += len + 1;
        }

        if(len < 6) {
            visit[t] = true;
            path[len] = t;
            dfs(t, target, visit, distance, path, len + 1, idx);
            visit[t] = false;
        }
    }
}

void check(int i) { 
    thread::id tid = this_thread::get_id();
    int idx = 0;
    if(mem.tidx.count(tid) == 0) {
        mem.lock.lock();
        if(mem.tidx.count(tid) == 0) {
            idx = mem.tcount;
            mem.tidx[tid] = mem.tcount++;
        }
        mem.lock.unlock();
    } else {
        idx = mem.tidx[tid];
    }

    bool *visit = mem.visit4 + idx * 280000;
    char *distance = mem.distance4 + idx * 280000;
    int *path = mem.path4 + idx * 7;
    int *update = mem.update4 + idx * 125000;

    int udlen = 0;
    visit[i] = true;
    path[0] = i;
    prune(i, i, visit, distance, 1, update, udlen);
    //370ms
    dfs(i, i, visit, distance, path, 1, idx);
    for(int j = 0; j < udlen; j++) distance[update[j]] = 10;
    visit[i] = false;
}

void checkCircle() {

    //Mem Init
    mem.visit4 = new bool[280000 * 4];
    mem.distance4 = new char[280000 * 4];
    mem.update4 = new int[50 * 50 * 50 * 4];
    memset(mem.visit4, false, 4 * 280000 * sizeof(bool));
    memset(mem.distance4, 10, 4 * 280000 * sizeof(char));
    mem.tcount = 0;
    mem.path4 = new int[4 * 7];

    //Init answer buffer
    for(int i = 0; i < 4; i++) {
        ab.ans[0][i] = new int[3 * 300000];
        ab.ans[1][i] = new int[4 * 300000];
        ab.ans[2][i] = new int[5 * 600000];
        ab.ans[3][i] = new int[6 * 1200000];
        ab.ans[4][i] = new int[7 * 1800000];
        memset(ab.len[i], 0, 5 * sizeof(int));
    }

    ThreadPool pool(4);
    for(int ii = 0; ii < graph.pnum; ii++){
        int i = graph.list[ii];
        if(graph.bgl[i] != 0) {
            pool.enqueue([i] {
                check(i); 
            });
        }
    }
}

#define print_time true

int main(int argc,char* argv[]) {

    string dataFileName = argv[1];
    string answerFileName = "answer_cpp_new2.txt";
#if print_time
    timeval start, end; 
    gettimeofday(&start, NULL);
#endif
    loadData(dataFileName);
#if print_time
    gettimeofday(&end, NULL);
    cout<<"load data \t"<<1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)<<endl;
#endif
    checkCircle();
#if print_time
    gettimeofday(&start, NULL);
    cout<<"check circle \t"<<1000000*(start.tv_sec - end.tv_sec) + (start.tv_usec - end.tv_usec)<<endl;
#endif
    saveAnswer(answerFileName);
#if print_time 
    gettimeofday(&end, NULL);
    cout<<"save answer \t"<<1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)<<endl;
#endif
    return 0;
}