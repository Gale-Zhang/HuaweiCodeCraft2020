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

struct Str {
    int len;
    char arr[11];
};

struct Graph {
    int pnum;   //点数
    int list[VNUM];
    int id[VNUM];
    LineArr out[VNUM];   //出边
    LineArr in[VNUM];    //入边
    Str str[VNUM];
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
            memcpy(graph.str[stIdx].arr, buffer + last, cur - last);
            graph.str[stIdx].len = cur - last;
            graph.str[stIdx].arr[graph.str[stIdx].len++] = ',';
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
            memcpy(graph.str[endIdx].arr, buffer + last, cur - last);
            graph.str[endIdx].len = cur - last;
            graph.str[endIdx].arr[graph.str[endIdx].len++] = ',';
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

void prune(int cur, int target, bool *visit, char *distance, int *cost, int *update, int &udlen) {
    LineArr t1 = graph.in[cur];
    for(int i1 = 0; i1 < t1.len; ++i1) {
        int p1 = t1.line[i1].point;
        int w1 = t1.line[i1].weight;

        if(graph.id[p1] < target || visit[p1]) continue;

        if(1 < distance[p1]) {
            distance[p1] = 1;
            cost[p1] = w1;
            update[udlen++] = p1;
        }

        visit[p1] = true;
        
        LineArr t2 = graph.in[p1];
        for(int i2 = 0; i2 < t2.len; ++i2) {
            int p2 = t2.line[i2].point;
            int w2 = t2.line[i2].weight;

            if(graph.id[p2] < target || visit[p2]) continue;

            if(2 < distance[p2] && valid(w2, w1)) {
                distance[p2] = 2;
                cost[p2] = w2;
                update[udlen++] = p2;
            }

            visit[p2] = true;

            LineArr t3 = graph.in[p2];
            for(int i3 = 0; i3 < t3.len; ++i3) {
                int p3 = t3.line[i3].point;
                int w3 = t3.line[i3].weight;

                if(3 < distance[p3] && valid(w3, w2) && graph.id[p3] > target && !visit[p3]) {
                    distance[p3] = 3;
                    cost[p3] = w3;
                    update[udlen++] = p3;
                }
            }             
            visit[p2] = false;
        }        
        visit[p1] = false;
    }
}

void dfs(int cur, int target, bool *visit, char *distance, int *cost, int path0, int path1, int w0, int idx) {
    
    LineArr t1 = graph.out[cur];

    for(int i1 = 0; i1 < t1.len; ++i1) {
        int p1 = t1.line[i1].point;
        int w1 = t1.line[i1].weight;

        if(2 + distance[p1] > 7 || graph.id[p1] < target || !valid(w0, w1) || visit[p1]) continue;

        if(distance[p1] == 1 && valid(w1, cost[p1]) && valid(cost[p1], w0)) {
            ++ab.num[idx];
            char *start = ab.ans[idx][0] + ab.len[idx][0];
            int offset = 0;

            memcpy(start + offset, graph.str[path0].arr, graph.str[path0].len);
            offset += graph.str[path0].len;
            memcpy(start + offset, graph.str[path1].arr, graph.str[path1].len);
            offset += graph.str[path1].len;
            memcpy(start + offset, graph.str[p1].arr, graph.str[p1].len);
            offset += graph.str[p1].len;

            start[offset - 1] = '\n';
            ab.len[idx][0] += offset;
        }

        visit[p1] = true;

        LineArr t2 = graph.out[p1];

        for(int i2 = 0; i2 < t2.len; ++i2) {
            int p2 = t2.line[i2].point;
            int w2 = t2.line[i2].weight;

            if(3 + distance[p2] > 7 || graph.id[p2] < target || !valid(w1, w2) || visit[p2]) continue;

            if(distance[p2] == 1 && valid(w2, cost[p2]) && valid(cost[p2], w0)) {
                ++ab.num[idx];
                char *start = ab.ans[idx][1] + ab.len[idx][1];
                int offset = 0;

                memcpy(start + offset, graph.str[path0].arr, graph.str[path0].len);
                offset += graph.str[path0].len;
                memcpy(start + offset, graph.str[path1].arr, graph.str[path1].len);
                offset += graph.str[path1].len;
                memcpy(start + offset, graph.str[p1].arr, graph.str[p1].len);
                offset += graph.str[p1].len;
                memcpy(start + offset, graph.str[p2].arr, graph.str[p2].len);
                offset += graph.str[p2].len;

                start[offset - 1] = '\n';
                ab.len[idx][1] += offset;
            }

            visit[p2] = true;

            LineArr t3 = graph.out[p2];

            for(int i3 = 0; i3 < t3.len; ++i3) {
                int p3 = t3.line[i3].point;
                int w3 = t3.line[i3].weight;

                if(4 + distance[p3] > 7 || graph.id[p3] < target || !valid(w2, w3) || visit[p3]) continue;

                if(distance[p3] == 1 && valid(w3, cost[p3]) && valid(cost[p3], w0)) {
                    ++ab.num[idx];
                    char *start = ab.ans[idx][2] + ab.len[idx][2];
                    int offset = 0;

                    memcpy(start + offset, graph.str[path0].arr, graph.str[path0].len);
                    offset += graph.str[path0].len;
                    memcpy(start + offset, graph.str[path1].arr, graph.str[path1].len);
                    offset += graph.str[path1].len;
                    memcpy(start + offset, graph.str[p1].arr, graph.str[p1].len);
                    offset += graph.str[p1].len;
                    memcpy(start + offset, graph.str[p2].arr, graph.str[p2].len);
                    offset += graph.str[p2].len;
                    memcpy(start + offset, graph.str[p3].arr, graph.str[p3].len);
                    offset += graph.str[p3].len;

                    start[offset - 1] = '\n';
                    ab.len[idx][2] += offset;
                }

                visit[p3] = true;

                LineArr t4 = graph.out[p3];

                for(int i4 = 0; i4 < t4.len; ++i4) {
                    int p4 = t4.line[i4].point;
                    int w4 = t4.line[i4].weight;

                    if(5 + distance[p4] > 7 || graph.id[p4] < target || !valid(w3, w4) || visit[p4]) continue;

                    if(distance[p4] == 1 && valid(w4, cost[p4]) && valid(cost[p4], w0)) {
                        ++ab.num[idx];
                        char *start = ab.ans[idx][3] + ab.len[idx][3];
                        int offset = 0;

                        memcpy(start + offset, graph.str[path0].arr, graph.str[path0].len);
                        offset += graph.str[path0].len;
                        memcpy(start + offset, graph.str[path1].arr, graph.str[path1].len);
                        offset += graph.str[path1].len;
                        memcpy(start + offset, graph.str[p1].arr, graph.str[p1].len);
                        offset += graph.str[p1].len;
                        memcpy(start + offset, graph.str[p2].arr, graph.str[p2].len);
                        offset += graph.str[p2].len;
                        memcpy(start + offset, graph.str[p3].arr, graph.str[p3].len);
                        offset += graph.str[p3].len;
                        memcpy(start + offset, graph.str[p4].arr, graph.str[p4].len);
                        offset += graph.str[p4].len;

                        start[offset - 1] = '\n';
                        ab.len[idx][3] += offset;
                    }

                    visit[p4] = true;

                    LineArr t5 = graph.out[p4];

                    for(int i5 = 0; i5 < t5.len; ++i5) {
                        int p5 = t5.line[i5].point;
                        int w5 = t5.line[i5].weight;

                        if(distance[p5] == 1 && graph.id[p5] > target && valid(w4, w5) && !visit[p5] 
                            && valid(w5, cost[p5]) && valid(cost[p5], w0)) {
                            ++ab.num[idx];
                            char *start = ab.ans[idx][4] + ab.len[idx][4];
                            int offset = 0;

                            memcpy(start + offset, graph.str[path0].arr, graph.str[path0].len);
                            offset += graph.str[path0].len;
                            memcpy(start + offset, graph.str[path1].arr, graph.str[path1].len);
                            offset += graph.str[path1].len;
                            memcpy(start + offset, graph.str[p1].arr, graph.str[p1].len);
                            offset += graph.str[p1].len;
                            memcpy(start + offset, graph.str[p2].arr, graph.str[p2].len);
                            offset += graph.str[p2].len;
                            memcpy(start + offset, graph.str[p3].arr, graph.str[p3].len);
                            offset += graph.str[p3].len;
                            memcpy(start + offset, graph.str[p4].arr, graph.str[p4].len);
                            offset += graph.str[p4].len;
                            memcpy(start + offset, graph.str[p5].arr, graph.str[p5].len);
                            offset += graph.str[p5].len;

                            start[offset - 1] = '\n';
                            ab.len[idx][4] += offset;
                        }
                    }                    
                    visit[p4] = false;
                }                
                visit[p3] = false;
            }            
            visit[p2] = false;
        }        
        visit[p1] = false;
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
        prune(i, graph.id[i], visit, distance, cost, update, udlen);
        
        LineArr t = graph.out[i];
        for(int j = 0; j < t.len; ++j) {
            int p = t.line[j].point;
            int w = t.line[j].weight;

            if(graph.id[p] < graph.id[i]) continue;

            visit[p] = true;
            dfs(p, graph.id[i], visit, distance, cost, i, p, w, idx);
            visit[p] = false;
        }

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