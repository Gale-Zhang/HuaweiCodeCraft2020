#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;


#define TNUM 8
#define VNUM 280000

struct Graph {
    int lnum;   //边数
    int pnum;   //点数
    int list[VNUM];
    int fg[VNUM][50];    //出边
    int bg[VNUM][50];    //入边
    char fgl[VNUM];   //出度
    char bgl[VNUM];   //入度
    char strlen[VNUM]; //str长度
    char str[VNUM][7];  //映射：索引 -> str
} graph;


struct AnswerBuffer{
    char *ans[TNUM][5];
    int len[TNUM][5];
    int num[TNUM];
} ab;

typedef struct stat Stat_t;

void loadData(string filename) {
    //cout<<"enter"<<endl;

    int idx = 0;
    memset(graph.fgl, 0, sizeof(char) * VNUM);
    memset(graph.bgl, 0, sizeof(char) * VNUM);
    memset(graph.strlen, 0, sizeof(char) * VNUM);

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

            ++graph.lnum;
            //cout<<graph.lnum<<" "<<st<<" "<<end<<endl;

            if(graph.strlen[st] == 0) {
                graph.list[idx++] = st;
                int bit = 1000000000, len = 0, cur = st;
                while(bit > cur) bit /= 10;
                while(bit > 0) {
                    graph.str[st][len++] = '0' + (cur / bit);
                    cur %= bit;
                    bit /= 10;
                }
                graph.str[st][len++] = ',';
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
                graph.str[end][len++] = ',';
                graph.strlen[end] = len;

            }

            graph.fg[st][graph.fgl[st]++] = end;
            graph.bg[end][graph.bgl[end]++] = st;

            l = r + 1;
            r += 2;
        } else {
            ++r;
        }
    }
    //cout<<"enter"<<endl;
    // auto t = clock();
    // munmap(buffer, bufLen);
    // close(fd);
    // cout<<clock() - t<<endl;

    //cout<<"finish"<<endl;

    //Graph Init
    graph.pnum = idx;
    sort(graph.list, graph.list + idx);
    while(graph.pnum > 0 && graph.list[graph.pnum - 1] > 50000) --graph.pnum;

    for(int ii = 0; ii < graph.pnum; ++ii){
        int i = graph.list[ii];
        sort(graph.fg[i], graph.fg[i] + graph.fgl[i]);
    }
    for(int ii = 0; ii < graph.pnum; ++ii){
        int i = graph.list[ii];
        sort(graph.bg[i], graph.bg[i] + graph.bgl[i]);
    }
}

void saveAnswer(string filename) {

    char data[10];
    int offset = 0;

    int cur = 0;
    for(int i = 0; i < TNUM; i++) cur += ab.num[i];
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

void prune(int cur, int target, bool *visit, char *distance, int dis, int *update, int &udlen) {
    for(int i = 0; i < graph.bgl[cur]; ++i) {
        int t = graph.bg[cur][i];

        if(t < target || visit[t]) continue;

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
    for(int i = 0; i < graph.fgl[cur]; ++i) {
        int t = graph.fg[cur][i];

        if((len > 3 && (len + distance[t] > 7)) || visit[t] || t < target) continue;
        //180ms

        path[len] = t;
        if(distance[t] == 1 && len > 1) {
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
            dfs(t, target, visit, distance, path, len + 1, idx);
            visit[t] = false;
        }
    }
}

int border[TNUM + 1];

void *check(void *arg) {
    int idx = *(int*)arg;
    //cout<<"thread "<<idx<<" start! ["<<border[idx]<<","<<border[idx + 1]<<")"<<endl;
    //Mem Init
    
    bool *visit = (bool*)malloc(VNUM);
    char *distance = (char*)malloc(VNUM);
    int *update = (int*)malloc(125000);
    memset(visit, false, VNUM * sizeof(bool));
    memset(distance, 10, VNUM * sizeof(char));
    int *path = (int*)malloc(7);

    //Init answer buffer
    ab.ans[idx][0] = (char*)malloc(5 * 1024 * 1024);
    ab.ans[idx][1] = (char*)malloc(10 * 1024 * 1024);
    ab.ans[idx][2] = (char*)malloc(20 * 1024 * 1024);
    ab.ans[idx][3] = (char*)malloc(35 * 1024 * 1024);
    ab.ans[idx][4] = (char*)malloc(70 * 1024 * 1024);
    memset(ab.len[idx], 0, 5 * sizeof(int));
    ab.num[idx] = 0;

    for(int ii = border[idx]; ii < border[idx + 1]; ++ii){     
        int i = graph.list[ii];
        //cout<<i<<endl;
        int udlen = 0;
        visit[i] = true;
        path[0] = i;
        prune(i, i, visit, distance, 1, update, udlen);
        //850ms
        //dfs(i, i, visit, distance, path, 1, idx);

    // {
    //     cout<<i<<"---------------------------"<<endl;
        int *stack = new int[10]{0};
        int cur = 0;
        while(stack[0] < graph.fgl[i]) {
            while(stack[cur] < graph.fgl[path[cur]]) {
                int t = graph.fg[path[cur]][stack[cur]];
                // cout<<path[cur]<<" "<<t<<" ";
                // for(int i = 1; i <= 8; i++) cout<<visit[i]<<" ";
                //     cout<<endl;
                if((cur > 2 && (cur + distance[t] > 6)) || visit[t] || t < path[0]) {
                    ++stack[cur];
                    continue;
                }
                //cout<<cur + 1<<" "<<t<<endl;
                path[cur + 1] = t;
                visit[t] = true;
                if(distance[t] == 1 && cur > 0) {
                    ++ab.num[idx];
                    char *start = ab.ans[idx][cur - 1] + ab.len[idx][cur - 1];
                    int offset = 0;
                    for(int j = 0; j <= cur + 1; ++j) {
                        memcpy(start + offset, graph.str[path[j]], graph.strlen[path[j]]);
                        offset += graph.strlen[path[j]];
                    }
                    start[offset - 1] = '\n';
                    ab.len[idx][cur - 1] += offset;
                }

                if(cur < 5) {
                    ++cur;
                } else {
                    visit[t] = false;
                    stack[cur]++;
                }
                //cout<<cur<<" "<<stack[cur]<<" "<<(int)graph.fgl[path[cur]]<<endl;
            }
            //cout<<cur<<" "<<path[cur]<<endl;
            //exit(0);
            if(cur > 0) {
                visit[path[cur]] = false;
                stack[cur] = 0;
                cur--;
                stack[cur]++;
            }
        }
    // }

        for(int j = 0; j < udlen; ++j) distance[update[j]] = 10;
        visit[i] = false;
    } 
    // timeval t;
    // gettimeofday(&t, NULL);
    // cout<<"thread "<<idx<<" findished!\t"<<1000000 * t.tv_sec + t.tv_usec<<endl;   
}




void checkCircle() {
    border[0] = 0;
    border[1] = 0;//25 * graph.pnum / 1000;
    border[2] = 0;//55 * graph.pnum / 1000;
    border[3] = 0;//90 * graph.pnum / 1000;
    border[4] = 0;//130 * graph.pnum / 1000;
    border[5] = 0;//175 * graph.pnum / 1000;
    border[6] = 0;//230 * graph.pnum / 1000;
    border[7] = 0;//350 * graph.pnum / 1000;
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

    string dataFileName = argv[1];
    string answerFileName = argv[2];
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
    //checkCircle();
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