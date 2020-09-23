#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

#define TNUM 8
#define VNUM 4000000

struct Line {
    int point;
    int weight;
};

struct LineArr {
    int len;
    int maxLen;
    Line *line;
};

struct Graph {
    int pnum;   //点数
    int list[VNUM];
    int id[VNUM];
    LineArr out[VNUM];   //出边
    LineArr in[VNUM];    //入边
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
    memset(graph.strlen, 0, sizeof(char) * VNUM);

    int fd = open(filename.c_str(), O_RDONLY);
    struct stat s;
    fstat(fd, &s);
    int bufLen = s.st_size;

    char *buffer = (char*)mmap(NULL, bufLen, PROT_READ, MAP_PRIVATE, fd, 0);

    int cur = 0;
    unordered_map<int, int> idxmap;//{{},VNUM};

    while(cur < bufLen) {

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

        // while((c = buffer[cur]) != '\n') {
        //     cost = cost * 10 + (c - '0');
        //     ++cur;
        // }
        // ++cur;

        while((c = buffer[cur]) != '\r') {
            cost = cost * 10 + (c - '0');
            ++cur;
        }
        cur += 2;
        if(graph.out[stIdx].len == graph.out[stIdx].maxLen) {
            //cout<<"resize graph.out["<<stIdx<<"]"<<endl;
            if(graph.out[stIdx].maxLen == 0) {
                graph.out[stIdx].maxLen = 64;
                graph.out[stIdx].line = (Line*)malloc(64 * sizeof(Line));
            } else {
                //cout<<"resize graph.out["<<stIdx<<"]"<<endl;
                graph.out[stIdx].maxLen *= 2;
                graph.out[stIdx].line = (Line*)realloc(graph.out[stIdx].line, graph.out[stIdx].maxLen * sizeof(Line));
            }
            //cout<<"succeed"<<endl;
        }
        if(graph.in[endIdx].len == graph.in[endIdx].maxLen) {
            //cout<<"resize graph.in["<<endIdx<<"]"<<endl;
            if(graph.in[endIdx].maxLen == 0) {
                graph.in[endIdx].maxLen = 64;
                graph.in[endIdx].line = (Line*)malloc(64 * sizeof(Line));
            } else {
                //cout<<"resize graph.in["<<endIdx<<"]"<<endl;
                graph.in[endIdx].maxLen *= 2;
                graph.in[endIdx].line = (Line*)realloc(graph.in[endIdx].line, graph.in[endIdx].maxLen * sizeof(Line));
            }
            //cout<<"succeed"<<endl;
        }
        graph.out[stIdx].line[graph.out[stIdx].len++] = {end, cost};
        graph.in[endIdx].line[graph.in[endIdx].len++] = {st, cost};
    }
    munmap(buffer, bufLen);
    close(fd);

    //cout<<"succeed"<<endl;
    //Graph Init
    graph.pnum = idx;
    sort(graph.list, graph.list + graph.pnum);
    for(int i = 0; i < graph.pnum; i++) {
        graph.list[i] = idxmap[graph.list[i]];
    }

    for(int i = 0; i < graph.pnum; i++) {
        sort(graph.out[i].line, graph.out[i].line + graph.out[i].len, cmp);
        for(int j = 0; j < graph.out[i].len; j++) {
            graph.out[i].line[j].point = idxmap[graph.out[i].line[j].point];
        }
    }
    for(int i = 0; i < graph.pnum; i++) {
        sort(graph.in[i].line, graph.in[i].line + graph.in[i].len, cmp);
        for(int j = 0; j < graph.in[i].len; j++) {
            graph.in[i].line[j].point = idxmap[graph.in[i].line[j].point];
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
    return b <= a * 3.0 && a <= b * 5.0;
}

void prune(int cur, int target, bool *visit, char *distance, int *cost, int dis, int *update, int &udlen) {
    for(int i = 0; i < graph.in[cur].len; ++i) {
        int t = graph.in[cur].line[i].point;

        if(graph.id[t] < target || visit[t]) continue;

        if(dis < distance[t]) {
            distance[t] = dis;
            cost[t] = graph.in[cur].line[i].weight;
            update[udlen++] = t;
        }

        if(dis == 3) continue;

        visit[t] = true;
        prune(t, target, visit, distance, cost, dis + 1, update, udlen);
        visit[t] = false;
    }
}

void dfs(int cur, int target, bool *visit, char *distance, int *cost, int *path, int *weight, int len, int idx) {
    for(int i = 0; i < graph.out[cur].len; ++i) {
        int t = graph.out[cur].line[i].point;

        if(len + distance[t] > 7) continue;
        if(graph.id[t] < target) continue;
        if(len > 1 && !valid(weight[len - 1], graph.out[cur].line[i].weight)) continue;
        if(visit[t]) continue;

        weight[len] = graph.out[cur].line[i].weight;
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
    
    bool *visit = (bool*)malloc(graph.pnum);
    char *distance = (char*)malloc(graph.pnum);
    int *update = (int*)malloc(125000 * sizeof(int));
    int *cost = (int*)malloc(graph.pnum * sizeof(int));
    memset(visit, false, graph.pnum * sizeof(bool));
    memset(distance, 4, graph.pnum * sizeof(char));
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

int main() {

    string dataFileName = "/data/test_data.txt";
    string answerFileName = "/projects/student/result.txt";

    loadData(dataFileName);
    checkCircle();
    saveAnswer(answerFileName);

    return 0;
}