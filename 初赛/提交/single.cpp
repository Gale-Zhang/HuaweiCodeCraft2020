#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

struct Graph {
    int lnum;   //边数
    int pnum;   //点数
    int id[280000];    //映射：索引 -> id
    int strlen[280000]; //str长度
    char str[280000][10];  //映射：索引 -> str
    int *ll;
    int *rr;    //索引号对应的排序位置
    int fg[280000][50];    //正向图
    int bg[280000][255];    //反向图
    int fgl[280000];   //正向图长度
    int bgl[280000];   //反向图长度
} graph;

struct Mem {
    bool *visit;
    int *reachable;
    int *path;
} mem;

struct AnswerBuffer{
    int *ans[5];
    int len[5];
} ab;

typedef struct stat Stat_t;

void loadData(string filename) {

    int idx = 0;
    memset(graph.fgl, 0, sizeof(int) * 280000);
    memset(graph.bgl, 0, sizeof(int) * 280000);

    unordered_map<int, int> id2idx;

    int fd = open(filename.c_str(), O_RDONLY);
    Stat_t s;
    fstat(fd, &s);
    int bufLen = s.st_size;

    char *buffer = (char*)mmap(NULL, bufLen, PROT_READ, MAP_PRIVATE, fd, 0);
    int l = 0;
    int r = 1;
    while(r <= bufLen) {
        if(buffer[r] == '\n') {
            int st = 0, end = 0, stIdx = 0, endIdx = 0;
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
            if(id2idx.find(st) == id2idx.end()) {
                graph.id[idx] = st;

                int bit = 1000000000, len = 0, cur = st;
                while(bit > cur) bit /= 10;
                while(bit > 0) {
                    graph.str[idx][len++] = '0' + (cur / bit);
                    cur %= bit;
                    bit /= 10;
                }
                graph.strlen[idx] = len;

                stIdx = idx;
                id2idx[st] = idx++;
            } else {
                stIdx = id2idx[st];
            }
            if(id2idx.find(end) == id2idx.end()) {
                graph.id[idx] = end;

                int bit = 1000000000, len = 0, cur = end;
                while(bit > cur) bit /= 10;
                while(bit > 0) {
                    graph.str[idx][len++] = '0' + (cur / bit);
                    cur %= bit;
                    bit /= 10;
                }
                graph.strlen[idx] = len;

                endIdx = idx;
                id2idx[end] = idx++;
            } else {
                endIdx = id2idx[end];
            }
            graph.fg[stIdx][graph.fgl[stIdx]++] = end;
            graph.bg[endIdx][graph.bgl[endIdx]++] = st;

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

    for(int i = 0; i < graph.pnum; i++) {
        sort(graph.fg[i], graph.fg[i] + graph.fgl[i]);
        for(int j = 0; j < graph.fgl[i]; j++) {
            graph.fg[i][j] = id2idx[graph.fg[i][j]];
        }
    }

    for(int i = 0; i < graph.pnum; i++) {
        sort(graph.bg[i], graph.bg[i] + graph.bgl[i]);
        for(int j = 0; j < graph.bgl[i]; j++) {
            graph.bg[i][j] = id2idx[graph.bg[i][j]];
        }
    }

    //Mem Init
    mem.visit = new bool[graph.pnum];
    mem.reachable = new int[graph.pnum];
    memset(mem.visit, false, graph.pnum * sizeof(bool));
    memset(mem.reachable, -1, graph.pnum * sizeof(int));//初始化的长度是否正确？
    mem.path = new int[7];

    //Init answer buffer
    ab.ans[0] = new int[3 * 500000];
    ab.ans[1] = new int[4 * 500000];
    ab.ans[2] = new int[5 * 1000000];
    ab.ans[3] = new int[6 * 2000000];
    ab.ans[4] = new int[7 * 3000000];
    memset(ab.len, 0, 5 * sizeof(int));
    //cout<<graph.pnum<<" "<<graph.lnum<<endl;
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
        int l = i + 3;
        for(int j = 0; j < ab.len[i]; j += l) {
            for(int k = j; k < j + l; k++) {
                int cur = ab.ans[i][k];
                memcpy(data + offset, graph.str[cur], graph.strlen[cur]);
                offset += graph.strlen[cur];
                data[offset++] = ',';
            }
            data[offset - 1] = '\n';
        }
    }

    FILE *fp = fopen(filename.c_str(), "w");
    fwrite(data, sizeof(char), offset, fp);
    fclose(fp);
}

void prune_fg(int cur, int target, bool * visit, int *reachable, int distance) {
    for(int i = 0; i < graph.fgl[cur]; i++) {
        int t = graph.fg[cur][i];
        if(graph.id[t] < graph.id[target]) continue;
        if(visit[t]) continue;
        reachable[t] = target;
        if(distance == 3) continue;
        visit[t] = true;
        prune_fg(t, target, visit, reachable, distance + 1);
        visit[t] = false;
    }
}

void prune_bg(int cur, int target, bool *visit, int *reachable, int distance) {
    for(int i = 0; i < graph.bgl[cur]; i++) {
        int t = graph.bg[cur][i];
        if(graph.id[t] < graph.id[target]) continue;
        if(visit[t]) continue;
        reachable[t] = target;
        if(distance == 3) continue;
        visit[t] = true;
        prune_bg(t, target, visit, reachable, distance + 1);
        visit[t] = false;
    }
}

void dfs_dense(int cur, int target, bool *visit, int *reachable, int *path, int len) {
    for(int i = 0; i < graph.fgl[cur]; i++) {
        int t = graph.fg[cur][i];
        if(graph.id[t] < graph.id[target]) continue;

        //300ms
        if(reachable[t] == -2 && !visit[t]) {
            path[len] = t;
            if(len > 1) {
                int *start = ab.ans[len - 2] + ab.len[len - 2];
                memcpy(start, path, (len + 1) * sizeof(int));
                ab.len[len - 2] += len + 1;
            }
        }

        if(visit[t] || (reachable[t] != target && reachable[t] != -2)) continue;
        if(len == 6 || t == target) continue;
        visit[t] = true;
        path[len] = t;
        dfs_dense(t, target, visit, reachable, path, len + 1, idx);
        visit[t] = false;
    }
}

void checkDenseCircle() {
    bool *visit = mem.visit4;
    int *reachable = mem.reachable4;
    int *path = mem.path4;
    for(int ii = 0; ii < graph.pnum; ii++) {
        int i = graph.ll[ii];
        path[0] = i;
        prune_fg(i, i, visit, reachable, 1);
        prune_bg(i, i, visit, reachable, 1);
        for(int j = 0; j < graph.bgl[i]; j++) {
            reachable[graph.bg[i][j]] = -2;
        }

        //2500ms
        dfs_dense(i, i, visit, reachable, path, 1);

        for(int j = 0; j < graph.bgl[i]; j++) {
            reachable[graph.bg[i][j]] = i;
        } 
    }     
}

void checkCircle() {
    checkDenseCircle();
    // if(graph.lnum / graph.pnum < 5) {
    //     checkSparseCircle();
    // } else {
    //     checkDenseCircle();
    // }
}

int main() {

    string dataFileName = "/data/test_data.txt";
    string answerFileName = "/projects/student/result.txt";

    loadData(dataFileName);
    checkCircle();
    saveAnswer(answerFileName);

    return 0;
}
