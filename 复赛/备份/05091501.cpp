#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

#define TNUM 8
#define VNUM 2000000

struct Line {
    int point;
    int weight;
};

struct LineArr {
    int len;
    int maxLen;
    Line *line;
}

struct Graph {
    int lnum;   //边数
    int pnum;   //点数
    int list[VNUM];
    int id[VNUM];
    vector<Line> out[VNUM];   //出边
    vector<Line> in[VNUM];    //入边
    char odr[VNUM];   //出度
    char idr[VNUM];   //入度
    char strlen[VNUM];  //str长度
    char str[VNUM][11]; //映射：索引 -> str
} graph;

struct AnswerBuffer{
    char *ans[TNUM][5];
    int len[TNUM][5];
    int num[TNUM];
} ab;

bool cmp(Line a, Line b) {
    return a.point < b.point;
}

void loadData(string filename) {

    int idx = 0;
    memset(graph.odr, 0, sizeof(char) * VNUM);
    memset(graph.idr, 0, sizeof(char) * VNUM);
    memset(graph.strlen, 0, sizeof(char) * VNUM);
    // for(int i = 0; i < VNUM; i++) {
    //     graph.out[i].reserve(50);
    //     graph.in[i].reserve(50);
    // }

    int fd = open(filename.c_str(), O_RDONLY);
    struct stat s;
    fstat(fd, &s);
    int bufLen = s.st_size;

    char *buffer = (char*)mmap(NULL, bufLen, PROT_READ, MAP_PRIVATE, fd, 0);

    int cur = 0;
    unordered_map<int, int> idxmap;//{{},VNUM};

    while(cur < bufLen) {

        ++graph.lnum;

        int st = 0, end = 0, cost = 0;
        int stIdx = 0, endIdx = 0;
        char c;

        int last = cur;
        while((c = buffer[cur]) != ',') {
            st = st * 10 + (c - '0');
            ++cur;
        }
        if(idxmap.find(st) == idxmap.end()) {
            graph.list[idx] = st;
            stIdx = idx;
            graph.id[stIdx] = st;
            idxmap[st] = idx++;
            memcpy(graph.str[stIdx], buffer + last, cur - last);
            graph.strlen[stIdx] = cur - last;
            graph.str[stIdx][graph.strlen[stIdx]++] = ',';
        } else {
            stIdx = idxmap[st];
        }
        ++cur;

        last = cur;
        while((c = buffer[cur]) != ',') {
            end = end * 10 + (c - '0');
            ++cur;
        }
        if(idxmap.find(end) == idxmap.end()) {
            graph.list[idx] = end;
            endIdx = idx;
            graph.id[endIdx] = end;
            idxmap[end] = idx++;
            memcpy(graph.str[endIdx], buffer + last, cur - last);
            graph.strlen[endIdx] = cur - last;
            graph.str[endIdx][graph.strlen[endIdx]++] = ',';
        } else {
            endIdx = idxmap[end];
        }
        ++cur;

        while((c = buffer[cur]) != '\n') {
            cost = cost * 10 + (c - '0');
            ++cur;
        }
        ++cur;

        // while((c = buffer[cur]) != '\r') {
        //     cost = cost * 10 + (c - '0');
        //     ++cur;
        // }
        // cur += 2;

        graph.out[stIdx].push_back({end, cost});
        ++graph.odr[stIdx]; 
        graph.in[endIdx].push_back({st, cost});
        ++graph.idr[endIdx];
    }
    munmap(buffer, bufLen);
    close(fd);

    //Graph Init
    graph.pnum = idx;
    sort(graph.list, graph.list + graph.pnum);
    for(int i = 0; i < graph.pnum; i++) {
        graph.list[i] = idxmap[graph.list[i]];
    }

    for(int i = 0; i < graph.pnum; i++) {
        sort(graph.out[i].begin(), graph.out[i].end(), cmp);
        for(int j = 0; j < graph.odr[i]; j++) {
            graph.out[i][j].point = idxmap[graph.out[i][j].point];
        }
    }
    for(int i = 0; i < graph.pnum; i++) {
        sort(graph.in[i].begin(), graph.in[i].end(), cmp);
        for(int j = 0; j < graph.idr[i]; j++) {
            graph.in[i][j].point = idxmap[graph.in[i][j].point];
        }
    }

}

void saveAnswer(string filename) {

    char data[10];
    int offset = 0;

    int cur = 0;
    for(int i = 0; i < TNUM; i++) {
        cur += ab.num[i];
        //cout<<"thread "<<i<<" find "<<ab.num[i]<<"\t circle!"<<endl;
    }
    int bit = 1000000000;
    while(bit > cur) bit /= 10;
    while(bit > 0) {
        data[offset++] = '0' + (cur / bit);
        cur %= bit;
        bit /= 10;
    }
    data[offset++] = '\n';

    FILE *fp = fopen(filename.c_str(), "w");
    fwrite(data, sizeof(char), offset, fp);
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < TNUM; j++) {
            fwrite(ab.ans[j][i], sizeof(char), ab.len[j][i], fp);
        }
    }
    fclose(fp);
    //exit(0);
}

inline bool valid(double a, double b) {
    return b <= a * 3.0 && b >= a * 0.2;
}

void prune(int cur, int target, bool *visit, char *distance, int *cost, int dis, int *update, int &udlen) {
    for(int i = 0; i < graph.idr[cur]; ++i) {
        int t = graph.in[cur][i].point;

        if(graph.id[t] < target || visit[t]) continue;

        if(dis < distance[t]) {
            distance[t] = dis;
            cost[t] = graph.in[cur][i].weight;
            update[udlen++] = t;
        }

        if(dis == 3) continue;

        visit[t] = true;
        prune(t, target, visit, distance, cost, dis + 1, update, udlen);
        visit[t] = false;
    }
}

void dfs(int cur, int target, bool *visit, char *distance, int *cost, int *path, int *weight, int len, int idx) {
    for(int i = 0; i < graph.odr[cur]; ++i) {
        int t = graph.out[cur][i].point;

        if(len + distance[t] > 7) continue;
        if(graph.id[t] < target) continue;
        if(visit[t]) continue;
        if(len > 1 && !valid(weight[len - 1], graph.out[cur][i].weight)) continue;

        weight[len] = graph.out[cur][i].weight;
        path[len] = t;

        if(distance[t] == 1 && len > 1 && valid(weight[len], cost[t]) && valid(cost[t], weight[1])) {
            ++ab.num[idx];
            char *start = ab.ans[idx][len - 2] + ab.len[idx][len - 2];
            int offset = 0;
            for(int j = 0; j <= len; ++j) {
                memcpy(start + offset, graph.str[path[j]], graph.strlen[path[j]]);
                offset += graph.strlen[path[j]];
            }
            start[offset - 1] = '\n';
            ab.len[idx][len - 2] += offset;
        }

        if(len < 6) {
            visit[t] = true;
            dfs(t, target, visit, distance, cost, path, weight, len + 1, idx);
            visit[t] = false;
        }
    }
}

int border[TNUM + 1];

void *check(void *arg) {
    int idx = *(int*)arg;
    
    bool *visit = (bool*)malloc(VNUM);
    char *distance = (char*)malloc(VNUM);
    int *update = (int*)malloc(125000 * sizeof(int));
    int *cost = (int*)malloc(VNUM * sizeof(int));
    memset(visit, false, VNUM * sizeof(bool));
    memset(distance, 4, VNUM * sizeof(char));
    int path[7];
    int weight[7];

    //Init answer buffer
    ab.ans[idx][0] = (char*)malloc(50 * 1024 * 1024);
    ab.ans[idx][1] = (char*)malloc(100 * 1024 * 1024);
    ab.ans[idx][2] = (char*)malloc(200 * 1024 * 1024);
    ab.ans[idx][3] = (char*)malloc(350 * 1024 * 1024);
    ab.ans[idx][4] = (char*)malloc(700 * 1024 * 1024);
    memset(ab.len[idx], 0, 5 * sizeof(int));
    ab.num[idx] = 0;

    for(int ii = border[idx]; ii < border[idx + 1]; ++ii){     
        int i = graph.list[ii];
        //cout<<i<<endl;
        int udlen = 0;
        visit[i] = true;
        path[0] = i;
        prune(i, graph.id[i], visit, distance, cost, 1, update, udlen);
        //850ms
        dfs(i, graph.id[i], visit, distance, cost, path, weight, 1, idx);

        for(int j = 0; j < udlen; ++j) distance[update[j]] = 4;
        visit[i] = false;
    } 
}

void checkCircle() {
    border[0] = 0;
    border[1] = 25 * graph.pnum / 1000;
    border[2] = 55 * graph.pnum / 1000;
    border[3] = 90 * graph.pnum / 1000;
    border[4] = 130 * graph.pnum / 1000;
    border[5] = 175 * graph.pnum / 1000;
    border[6] = 230 * graph.pnum / 1000;
    border[7] = 350 * graph.pnum / 1000;
    border[8] = graph.pnum;
    pthread_t tid[TNUM];

    int *arg = new int[TNUM];
    for(int i = 0; i < TNUM; ++i) {
        arg[i] = i;
    }
    for(int i = 0; i < TNUM; ++i) {
        pthread_create(&tid[i], NULL, check, arg + i);
    }
    for(int i = 0; i < TNUM; ++i) {
        pthread_join(tid[i], NULL);
    }
}

#define print_time true

int main(int argc,char* argv[]) {

    string dataFileName = "data/19630345/test_data.txt";//argv[1];
    string answerFileName = "2.txt";//argv[2];
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