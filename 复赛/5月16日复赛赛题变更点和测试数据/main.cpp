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
#define uint unsigned int

struct LineArr {
    uint len;
    uint maxLen;
    pair<uint, double> *line; //point, weight
};

struct Str {
    uint len;
    char arr[11];
};

struct Graph {
    uint pnum;   //点数
    uint list[VNUM];
    uint id[VNUM];
    LineArr out[VNUM];   //出边
    LineArr in[VNUM];    //入边
    Str str[VNUM];
} graph;

struct AnswerBuffer{
    uint num[TNUM];
    uint len[TNUM][6];
    char *ans[TNUM][6];
} ab;

bool cmp(pair<uint, double> a, pair<uint, double> b) {
    return a.first < b.first;
}

void loadData(string filename) {

    uint idx = 0;

    uint fd = open(filename.c_str(), O_RDONLY);
    struct stat s;
    fstat(fd, &s);
    uint bufLen = s.st_size;

    char *buffer = (char*)mmap(NULL, bufLen, PROT_READ, MAP_PRIVATE, fd, 0);

    uint cur = 0;
    unordered_map<uint, uint> idxmap;//{{},VNUM};

    while(cur < bufLen) {

        uint st = 0, end = 0;
        double cost = 0;
        uint stIdx = 0, endIdx = 0;
        char c;

        uint last = cur;
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

        double t = 10;
        bool flag = true;
#if 0
        while((c = buffer[cur]) != '\n') {
            if(c == '.') {
                flag = false;
            } else {
                if(flag) {
                    cost = cost * 10 + (c - '0');
                } else {
                    cost += (double)(c - '0') / t;
                    t *= 10;
                }
            }
            ++cur;
        }
        ++cur;
#else
        while((c = buffer[cur]) != '\r') {
            if(c == '.') {
                flag = false;
            } else {
                if(flag) {
                    cost = cost * 10 + (c - '0');
                } else {
                    cost += (c - '0') / t;
                    t *= 10;
                }
            }
            ++cur;
        }
        cur += 2;
#endif

        if(graph.out[stIdx].len == graph.out[stIdx].maxLen) {
            if(graph.out[stIdx].maxLen == 0) {
                graph.out[stIdx].maxLen = 64;
                graph.out[stIdx].line = (pair<uint, double>*)malloc(64 * sizeof(pair<uint, double>));
            } else {
                graph.out[stIdx].maxLen *= 2;
                graph.out[stIdx].line = (pair<uint, double>*)realloc(graph.out[stIdx].line, graph.out[stIdx].maxLen * sizeof(pair<uint, double>));
            }
        }
        if(graph.in[endIdx].len == graph.in[endIdx].maxLen) {
            if(graph.in[endIdx].maxLen == 0) {
                graph.in[endIdx].maxLen = 64;
                graph.in[endIdx].line = (pair<uint, double>*)malloc(64 * sizeof(pair<uint, double>));
            } else {
                graph.in[endIdx].maxLen *= 2;
                graph.in[endIdx].line = (pair<uint, double>*)realloc(graph.in[endIdx].line, graph.in[endIdx].maxLen * sizeof(pair<uint, double>));
            }
        }
        graph.out[stIdx].line[graph.out[stIdx].len++] = {end, cost};
        graph.in[endIdx].line[graph.in[endIdx].len++] = {st, cost};
    }
    munmap(buffer, bufLen);
    close(fd);

    //Graph Init
    graph.pnum = idx;
    sort(graph.list, graph.list + graph.pnum);
    for(uint i = 0; i < graph.pnum; i++) {
        graph.list[i] = idxmap[graph.list[i]];
    }

    for(uint i = 0; i < graph.pnum; i++) {
        sort(graph.out[i].line, graph.out[i].line + graph.out[i].len, cmp);
        for(uint j = 0; j < graph.out[i].len; j++) {
            graph.out[i].line[j].first = idxmap[graph.out[i].line[j].first];
        }
    }
    for(uint i = 0; i < graph.pnum; i++) {
        sort(graph.in[i].line, graph.in[i].line + graph.in[i].len, cmp);
        for(uint j = 0; j < graph.in[i].len; j++) {
            graph.in[i].line[j].first = idxmap[graph.in[i].line[j].first];
        }
    }

}

void saveAnswer(string filename) {

    char data[10];
    uint offset = 0;

    uint cur = 0;
    for(uint i = 0; i < TNUM; i++) {
        cur += ab.num[i];
    }
    uint bit = 1000000000;
    while(bit > cur) bit /= 10;
    while(bit > 0) {
        data[offset++] = '0' + (cur / bit);
        cur %= bit;
        bit /= 10;
    }
    data[offset++] = '\n';

    FILE *fp = fopen(filename.c_str(), "w");
    fwrite(data, sizeof(char), offset, fp);
    for(uint i = 0; i < 6; i++) {
        for(uint j = 0; j < TNUM; j++) {
            fwrite(ab.ans[j][i], sizeof(char), ab.len[j][i], fp);
        }
    }
    fclose(fp);
}

inline bool valid(double a, double b) {
    return b <= a * 3.0 && a <= b * 5.0;
}

void prune(uint cur, uint target, bool *visit, char *distance, double *cost, uint *update, uint &udlen) {
    LineArr t1 = graph.in[cur];
    for(uint i1 = 0; i1 < t1.len; ++i1) {
        uint p1 = t1.line[i1].first;
        double w1 = t1.line[i1].second;

        if(graph.id[p1] < target || visit[p1]) continue;


        distance[p1] = 1;
        cost[p1] = w1;
        update[udlen++] = p1;

        visit[p1] = true;
        
        LineArr t2 = graph.in[p1];
        for(uint i2 = 0; i2 < t2.len; ++i2) {
            uint p2 = t2.line[i2].first;
            double w2 = t2.line[i2].second;

            if(graph.id[p2] < target || visit[p2]) continue;

            if(2 < distance[p2] && valid(w2, w1)) {
                distance[p2] = 2;
                cost[p2] = w2;
                update[udlen++] = p2;
            }

            visit[p2] = true;

            LineArr t3 = graph.in[p2];
            for(uint i3 = 0; i3 < t3.len; ++i3) {
                uint p3 = t3.line[i3].first;
                double w3 = t3.line[i3].second;

                if(graph.id[p3] < target || visit[p3]) continue;

                if(3 < distance[p3] && valid(w3, w2)) {
                    distance[p3] = 3;
                    cost[p3] = w3;
                    update[udlen++] = p3;
                }

                visit[p3] = true;

                LineArr t4 = graph.in[p3];
                for(uint i4 = 0; i4 < t4.len; ++i4) {
                    uint p4 = t4.line[i4].first;
                    double w4 = t4.line[i4].second;

                    if(graph.id[p4] < target || visit[p4]) continue;

                    if(4 < distance[p4] && valid(w4, w3)) {
                        distance[p4] = 4;
                        cost[p4] = w4;
                        update[udlen++] = p4;
                    }
                }

                visit[p3] = false;
            }             
            visit[p2] = false;
        }        
        visit[p1] = false;
    }
}

void dfs(uint cur, uint target, bool *visit, char *distance, double *cost, uint path0, uint path1, uint w0, uint idx) {
    
    LineArr t1 = graph.out[cur];

    for(uint i1 = 0; i1 < t1.len; ++i1) {
        auto l1 = t1.line[i1];
        uint p1 = l1.first;
        double w1 = l1.second;

        if(graph.id[p1] < target || !valid(w0, w1) || visit[p1]) continue;

        if(distance[p1] == 1 && valid(w1, cost[p1]) && valid(cost[p1], w0)) {
            ++ab.num[idx];
            char *start = ab.ans[idx][0] + ab.len[idx][0];
            uint offset = 0;

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

        for(uint i2 = 0; i2 < t2.len; ++i2) {
            auto l2 = t2.line[i2];
            uint p2 = l2.first;
            double w2 = l2.second;

            if(graph.id[p2] < target || !valid(w1, w2) || visit[p2]) continue;

            if(distance[p2] == 1 && valid(w2, cost[p2]) && valid(cost[p2], w0)) {
                ++ab.num[idx];
                char *start = ab.ans[idx][1] + ab.len[idx][1];
                uint offset = 0;

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

            for(uint i3 = 0; i3 < t3.len; ++i3) {
                auto l3 = t3.line[i3];
                uint p3 = l3.first;
                double w3 = l3.second;

                if(distance[p3] > 4 || graph.id[p3] < target || !valid(w2, w3) || visit[p3]) continue;

                if(distance[p3] == 1 && valid(w3, cost[p3]) && valid(cost[p3], w0)) {
                    ++ab.num[idx];
                    char *start = ab.ans[idx][2] + ab.len[idx][2];
                    uint offset = 0;

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

                for(uint i4 = 0; i4 < t4.len; ++i4) {
                    auto l4 = t4.line[i4];
                    uint p4 = l4.first;
                    double w4 = l4.second;

                    if(distance[p4] > 3 || graph.id[p4] < target || !valid(w3, w4) || visit[p4]) continue;

                    if(distance[p4] == 1 && valid(w4, cost[p4]) && valid(cost[p4], w0)) {
                        ++ab.num[idx];
                        char *start = ab.ans[idx][3] + ab.len[idx][3];
                        uint offset = 0;

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

                    for(uint i5 = 0; i5 < t5.len; ++i5) {
                        auto l5 = t5.line[i5];
                        uint p5 = l5.first;
                        double w5 = l5.second;

                        if(distance[p5] > 2 || graph.id[p5] < target || !valid(w4, w5) || visit[p5]) continue;

                        if(distance[p5] == 1 && valid(w5, cost[p5]) && valid(cost[p5], w0)) {
                            ++ab.num[idx];
                            char *start = ab.ans[idx][4] + ab.len[idx][4];
                            uint offset = 0;

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

                        visit[p5] = true;

                        LineArr t6 = graph.out[p5];

                        for(uint i6 = 0; i6 < t6.len; ++i6) {
                            auto l6 = t6.line[i6];
                            uint p6 = l6.first;
                            double w6 = l6.second;

                            if(graph.id[p6] < target || !valid(w5, w6) || visit[p6]) continue;

                            if(distance[p6] == 1 && valid(w6, cost[p6]) && valid(cost[p6], w0)) {
                                ++ab.num[idx];
                                char *start = ab.ans[idx][5] + ab.len[idx][5];
                                uint offset = 0;

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
                                memcpy(start + offset, graph.str[p6].arr, graph.str[p6].len);
                                offset += graph.str[p6].len;

                                start[offset - 1] = '\n';
                                ab.len[idx][5] += offset;
                            }
                        }                    
                        visit[p5] = false;
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

uint border[TNUM + 1];

void *check(void *arg) {
    uint idx = *(uint*)arg;
    
    bool *visit = (bool*)malloc(graph.pnum);
    char *distance = (char*)malloc(graph.pnum);
    double *cost = (double*)malloc(graph.pnum * sizeof(double));
    memset(visit, false, graph.pnum * sizeof(bool));
    memset(distance, 5, graph.pnum * sizeof(char));

    uint *update = (uint*)malloc(10000000 * sizeof(uint));

    //Init answer buffer
    ab.ans[idx][0] = (char*)malloc(50 * 1024 * 1024);
    ab.ans[idx][1] = (char*)malloc(100 * 1024 * 1024);
    ab.ans[idx][2] = (char*)malloc(200 * 1024 * 1024);
    ab.ans[idx][3] = (char*)malloc(350 * 1024 * 1024);
    ab.ans[idx][4] = (char*)malloc(700 * 1024 * 1024);
    ab.ans[idx][5] = (char*)malloc(1400 * 1024 * 1024);
    memset(ab.len[idx], 0, 6 * sizeof(uint));
    ab.num[idx] = 0;

    for(uint ii = border[idx]; ii < border[idx + 1]; ++ii){     
        uint i = graph.list[ii];
        //cout<<i<<endl;
        uint udlen = 0;
        visit[i] = true;
        prune(i, graph.id[i], visit, distance, cost, update, udlen);
        
        LineArr t = graph.out[i];
        for(uint j = 0; j < t.len; ++j) {
            uint p = t.line[j].first;
            uint w = t.line[j].second;

            if(graph.id[p] < graph.id[i]) continue;

            visit[p] = true;
            dfs(p, graph.id[i], visit, distance, cost, i, p, w, idx);
            visit[p] = false;
        }

        for(uint j = 0; j < udlen; ++j) distance[update[j]] = 5;
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

    uint *arg = new uint[TNUM];
    for(uint i = 0; i < TNUM; ++i) {
        arg[i] = i;
    }
    for(uint i = 0; i < TNUM; ++i) {
        pthread_create(&tid[i], NULL, check, arg + i);
    }
    for(uint i = 0; i < TNUM; ++i) {
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