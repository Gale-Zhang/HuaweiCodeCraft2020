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

// struct Line {
//     int point;
//     int weight;
// };

struct LineArr {
    int len;
    int maxLen;
    pair<int, int> *line; //point, weight
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

struct State {
    bool visit;
    char distance;
    int cost;
};

struct AnswerBuffer{
    char *ans[TNUM][5];
    int len[TNUM][5];
    int num[TNUM];
} ab;

bool cmp(pair<int, int> a, pair<int, int> b) {
    return a.first < b.first;
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
        if(graph.out[stIdx].len == graph.out[stIdx].maxLen) {
            //cout<<"resize graph.out["<<stIdx<<"]"<<endl;
            if(graph.out[stIdx].maxLen == 0) {
                graph.out[stIdx].maxLen = 64;
                graph.out[stIdx].line = (pair<int, int>*)malloc(64 * sizeof(pair<int, int>));
            } else {
                //cout<<"resize graph.out["<<stIdx<<"]"<<endl;
                graph.out[stIdx].maxLen *= 2;
                graph.out[stIdx].line = (pair<int, int>*)realloc(graph.out[stIdx].line, graph.out[stIdx].maxLen * sizeof(pair<int, int>));
            }
            //cout<<"succeed"<<endl;
        }
        if(graph.in[endIdx].len == graph.in[endIdx].maxLen) {
            //cout<<"resize graph.in["<<endIdx<<"]"<<endl;
            if(graph.in[endIdx].maxLen == 0) {
                graph.in[endIdx].maxLen = 64;
                graph.in[endIdx].line = (pair<int, int>*)malloc(64 * sizeof(pair<int, int>));
            } else {
                //cout<<"resize graph.in["<<endIdx<<"]"<<endl;
                graph.in[endIdx].maxLen *= 2;
                graph.in[endIdx].line = (pair<int, int>*)realloc(graph.in[endIdx].line, graph.in[endIdx].maxLen * sizeof(pair<int, int>));
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
            graph.out[i].line[j].first = idxmap[graph.out[i].line[j].first];
        }
    }
    for(int i = 0; i < graph.pnum; i++) {
        sort(graph.in[i].line, graph.in[i].line + graph.in[i].len, cmp);
        for(int j = 0; j < graph.in[i].len; j++) {
            graph.in[i].line[j].first = idxmap[graph.in[i].line[j].first];
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

void prune(int cur, int target, State* state, int *update, int &udlen) {
    LineArr t1 = graph.in[cur];
    for(int i1 = 0; i1 < t1.len; ++i1) {
        int p1 = t1.line[i1].first;
        int w1 = t1.line[i1].second;

        if(graph.id[p1] < target || state[p1].visit) continue;

        if(1 < state[p1].distance) {
            state[p1].distance = 1;
            state[p1].cost = w1;
            update[udlen++] = p1;
        }

        state[p1].visit = true;
        
        LineArr t2 = graph.in[p1];
        for(int i2 = 0; i2 < t2.len; ++i2) {
            int p2 = t2.line[i2].first;
            int w2 = t2.line[i2].second;

            if(graph.id[p2] < target || state[p2].visit) continue;

            if(2 < state[p2].distance && valid(w2, w1)) {
                state[p2].distance = 2;
                state[p2].cost = w2;
                update[udlen++] = p2;
            }

            state[p2].visit = true;

            LineArr t3 = graph.in[p2];
            for(int i3 = 0; i3 < t3.len; ++i3) {
                int p3 = t3.line[i3].first;
                int w3 = t3.line[i3].second;

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

void dfs(int cur, int target, State *state, int path0, int path1, int w0, int idx) {
    
    LineArr t1 = graph.out[cur];

    for(int i1 = 0; i1 < t1.len; ++i1) {
        int p1 = t1.line[i1].first;
        int w1 = t1.line[i1].second;

        if(2 + state[p1].distance > 7 || graph.id[p1] < target || !valid(w0, w1) || state[p1].visit) continue;

        if(state[p1].distance == 1 && valid(w1, state[p1].cost) && valid(state[p1].cost, w0)) {
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

        state[p1].visit = true;

        LineArr t2 = graph.out[p1];

        for(int i2 = 0; i2 < t2.len; ++i2) {
            int p2 = t2.line[i2].first;
            int w2 = t2.line[i2].second;

            if(3 + state[p2].distance > 7 || graph.id[p2] < target || !valid(w1, w2) || state[p2].visit) continue;

            if(state[p2].distance == 1 && valid(w2, state[p2].cost) && valid(state[p2].cost, w0)) {
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

            state[p2].visit = true;

            LineArr t3 = graph.out[p2];

            for(int i3 = 0; i3 < t3.len; ++i3) {
                int p3 = t3.line[i3].first;
                int w3 = t3.line[i3].second;

                if(4 + state[p3].distance > 7 || graph.id[p3] < target || !valid(w2, w3) || state[p3].visit) continue;

                if(state[p3].distance == 1 && valid(w3, state[p3].cost) && valid(state[p3].cost, w0)) {
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

                state[p3].visit = true;

                LineArr t4 = graph.out[p3];

                for(int i4 = 0; i4 < t4.len; ++i4) {
                    int p4 = t4.line[i4].first;
                    int w4 = t4.line[i4].second;

                    if(5 + state[p4].distance > 7 || graph.id[p4] < target || !valid(w3, w4) || state[p4].visit) continue;

                    if(state[p4].distance == 1 && valid(w4, state[p4].cost) && valid(state[p4].cost, w0)) {
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

                    state[p4].visit = true;

                    LineArr t5 = graph.out[p4];

                    for(int i5 = 0; i5 < t5.len; ++i5) {
                        int p5 = t5.line[i5].first;
                        int w5 = t5.line[i5].second;

                        if(state[p5].distance == 1 && graph.id[p5] > target && valid(w4, w5) && !state[p5].visit 
                            && valid(w5, state[p5].cost) && valid(state[p5].cost, w0)) {
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
                    state[p4].visit = false;
                }                
                state[p3].visit = false;
            }            
            state[p2].visit = false;
        }        
        state[p1].visit = false;
    }
}

int border[TNUM + 1];

void *check(void *arg) {
    int idx = *(int*)arg;
    
    // bool *visit = (bool*)malloc(graph.pnum);
    // char *distance = (char*)malloc(graph.pnum);
    // int *cost = (int*)malloc(graph.pnum * sizeof(int));
    // memset(visit, false, graph.pnum * sizeof(bool));
    // memset(distance, 4, graph.pnum * sizeof(char));

    State *state = (State*)malloc(graph.pnum * sizeof(State));
    for(int i = 0; i < graph.pnum; i++) state[i].distance = 4;

    int *update = (int*)malloc(125000 * sizeof(int));

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
        state[i].visit = true;
        prune(i, graph.id[i], state, update, udlen);
        
        LineArr t = graph.out[i];
        for(int j = 0; j < t.len; ++j) {
            int p = t.line[j].first;
            int w = t.line[j].second;

            if(graph.id[p] < graph.id[i]) continue;

            state[p].visit = true;
            dfs(p, graph.id[i], state, i, p, w, idx);
            state[p].visit = false;
        }

        for(int j = 0; j < udlen; ++j) state[update[j]].distance = 4;
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

    string dataFileName = argv[1];//"data/19630345/test_data.txt";
    string answerFileName = argv[2];//"2.txt";//
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