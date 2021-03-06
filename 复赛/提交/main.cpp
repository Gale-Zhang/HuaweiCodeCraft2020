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

// struct Line {
//     uint pouint;
//     uint weight;
// };

struct LineArr {
    uint len;
    uint maxLen;
    pair<uint, uint> *line; //pouint, weight
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

struct State {
    uint cost;
    bool visit;
    char distance;
};
struct AnswerBuffer{
    char *ans[TNUM][5];
    uint len[TNUM][5];
    uint num[TNUM];
} ab;

bool cmp(pair<uint, uint> a, pair<uint, uint> b) {
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

        uint st = 0, end = 0, cost = 0;
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
                graph.out[stIdx].line = (pair<uint, uint>*)malloc(64 * sizeof(pair<uint, uint>));
            } else {
                //cout<<"resize graph.out["<<stIdx<<"]"<<endl;
                graph.out[stIdx].maxLen *= 2;
                graph.out[stIdx].line = (pair<uint, uint>*)realloc(graph.out[stIdx].line, graph.out[stIdx].maxLen * sizeof(pair<uint, uint>));
            }
            //cout<<"succeed"<<endl;
        }
        if(graph.in[endIdx].len == graph.in[endIdx].maxLen) {
            //cout<<"resize graph.in["<<endIdx<<"]"<<endl;
            if(graph.in[endIdx].maxLen == 0) {
                graph.in[endIdx].maxLen = 64;
                graph.in[endIdx].line = (pair<uint, uint>*)malloc(64 * sizeof(pair<uint, uint>));
            } else {
                //cout<<"resize graph.in["<<endIdx<<"]"<<endl;
                graph.in[endIdx].maxLen *= 2;
                graph.in[endIdx].line = (pair<uint, uint>*)realloc(graph.in[endIdx].line, graph.in[endIdx].maxLen * sizeof(pair<uint, uint>));
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
        //cout<<"thread "<<i<<" find "<<ab.num[i]<<"\t circle!"<<endl;
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
    for(uint i = 0; i < 5; i++) {
        for(uint j = 0; j < TNUM; j++) {
            fwrite(ab.ans[j][i], sizeof(char), ab.len[j][i], fp);
        }
    }
    fclose(fp);
    //exit(0);
}

inline bool valid(double a, double b) {
    return b <= a * 3.0 && a <= b * 5.0;
}

void prune(uint cur, uint target, State* state, uint *update, uint &udlen) {
    LineArr t1 = graph.in[cur];
    for(uint i1 = 0; i1 < t1.len; ++i1) {
        uint p1 = t1.line[i1].first;
        uint w1 = t1.line[i1].second;

        if(graph.id[p1] < target || state[p1].visit) continue;

        if(1 < state[p1].distance) {
            state[p1].distance = 1;
            state[p1].cost = w1;
            update[udlen++] = p1;
        }

        state[p1].visit = true;
        
        LineArr t2 = graph.in[p1];
        for(uint i2 = 0; i2 < t2.len; ++i2) {
            uint p2 = t2.line[i2].first;
            uint w2 = t2.line[i2].second;

            if(graph.id[p2] < target || state[p2].visit) continue;

            if(2 < state[p2].distance && valid(w2, w1)) {
                state[p2].distance = 2;
                state[p2].cost = w2;
                update[udlen++] = p2;
            }

            state[p2].visit = true;

            LineArr t3 = graph.in[p2];
            for(uint i3 = 0; i3 < t3.len; ++i3) {
                uint p3 = t3.line[i3].first;
                uint w3 = t3.line[i3].second;

                if(3 < state[p3].distance && valid(w3, w2) && graph.id[p3] > target && !state[p3].visit) {
                    state[p3].distance = 3;
                    state[p3].cost = w3;
                    update[udlen++] = p3;
                }
            }             
            state[p2].visit = false;
        }        
        state[p1].visit = false;
    }
}

void dfs(uint cur, uint target, State *state, uint path0, uint path1, uint w0, uint idx) {
    
    LineArr t1 = graph.out[cur];

    for(uint i1 = 0; i1 < t1.len; ++i1) {
        uint p1 = t1.line[i1].first;
        uint w1 = t1.line[i1].second;
        State s1 = state[p1];

        if(2 + s1.distance > 7 || graph.id[p1] < target || !valid(w0, w1) || s1.visit) continue;

        if(s1.distance == 1 && valid(w1, s1.cost) && valid(s1.cost, w0)) {
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

        state[p1].visit = true;

        LineArr t2 = graph.out[p1];

        for(uint i2 = 0; i2 < t2.len; ++i2) {
            uint p2 = t2.line[i2].first;
            uint w2 = t2.line[i2].second;
            State s2 = state[p2];

            if(3 + s2.distance > 7 || graph.id[p2] < target || !valid(w1, w2) || s2.visit) continue;

            if(s2.distance == 1 && valid(w2, s2.cost) && valid(s2.cost, w0)) {
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

            state[p2].visit = true;

            LineArr t3 = graph.out[p2];

            for(uint i3 = 0; i3 < t3.len; ++i3) {
                uint p3 = t3.line[i3].first;
                uint w3 = t3.line[i3].second;
                State s3 = state[p3];

                if(4 + s3.distance > 7 || graph.id[p3] < target || !valid(w2, w3) || s3.visit) continue;

                if(s3.distance == 1 && valid(w3, s3.cost) && valid(s3.cost, w0)) {
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

                state[p3].visit = true;

                LineArr t4 = graph.out[p3];

                for(uint i4 = 0; i4 < t4.len; ++i4) {
                    uint p4 = t4.line[i4].first;
                    uint w4 = t4.line[i4].second;
                    State s4 = state[p4];

                    if(5 + s4.distance > 7 || graph.id[p4] < target || !valid(w3, w4) || s4.visit) continue;

                    if(s4.distance == 1 && valid(w4, s4.cost) && valid(s4.cost, w0)) {
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

                    state[p4].visit = true;

                    LineArr t5 = graph.out[p4];

                    for(uint i5 = 0; i5 < t5.len; ++i5) {
                        uint p5 = t5.line[i5].first;
                        uint w5 = t5.line[i5].second;
                        State s5 = state[p5];

                        if(s5.distance == 1 && graph.id[p5] > target && valid(w4, w5) && !s5.visit 
                            && valid(w5, s5.cost) && valid(s5.cost, w0)) {
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
                    }                    
                    state[p4].visit = false;
                }                
                state[p3].visit = false;
            }            
            state[p2].visit = false;
        }        
        state[p1].visit = false;
    }
}

uint border[TNUM + 1];

void *check(void *arg) {
    uint idx = *(uint*)arg;
    
    // bool *visit = (bool*)malloc(graph.pnum);
    // char *distance = (char*)malloc(graph.pnum);
    // uint *cost = (uint*)malloc(graph.pnum * sizeof(uint));
    // memset(visit, false, graph.pnum * sizeof(bool));
    // memset(distance, 4, graph.pnum * sizeof(char));

    State *state = (State*)malloc(graph.pnum * sizeof(State));
    for(uint i = 0; i < graph.pnum; i++) state[i].distance = 4;

    uint *update = (uint*)malloc(125000 * sizeof(uint));

    //Init answer buffer
    ab.ans[idx][0] = (char*)malloc(50 * 1024 * 1024);
    ab.ans[idx][1] = (char*)malloc(100 * 1024 * 1024);
    ab.ans[idx][2] = (char*)malloc(200 * 1024 * 1024);
    ab.ans[idx][3] = (char*)malloc(350 * 1024 * 1024);
    ab.ans[idx][4] = (char*)malloc(700 * 1024 * 1024);
    memset(ab.len[idx], 0, 5 * sizeof(uint));
    ab.num[idx] = 0;

    for(uint ii = border[idx]; ii < border[idx + 1]; ++ii){     
        uint i = graph.list[ii];
        //cout<<i<<endl;
        uint udlen = 0;
        state[i].visit = true;
        prune(i, graph.id[i], state, update, udlen);
        
        LineArr t = graph.out[i];
        for(uint j = 0; j < t.len; ++j) {
            uint p = t.line[j].first;
            uint w = t.line[j].second;

            if(graph.id[p] < graph.id[i]) continue;

            state[p].visit = true;
            dfs(p, graph.id[i], state, i, p, w, idx);
            state[p].visit = false;
        }

        for(uint j = 0; j < udlen; ++j) state[update[j]].distance = 4;
        state[i].visit = false;
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