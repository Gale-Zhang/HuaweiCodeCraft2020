package com.gale.java.round1;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.Stack;
import java.util.TreeSet;

public class SolutionNoTarjan {
	private HashMap<Integer, Integer> id2idx;
	private HashMap<Integer, Integer> idx2id;
	private int[][] graph;
	private int[][] antigraph;
	
    private List<List<Integer>> circle;
    private LinkedList<Integer> path;
	
	private final boolean debug = false;
	private long startTime = System.currentTimeMillis();
    
    public void loadData(String fileName, boolean skipTitle) {
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new FileReader(fileName));
        } catch (FileNotFoundException exception) {
            System.err.println(fileName + " File Not Found");
            return;
        }
        String line = "";
        id2idx = new HashMap();
        idx2id = new HashMap();
        List<List<Integer>> graphF = new ArrayList();
        List<List<Integer>> graphB = new ArrayList();
        int idx = 0;
        try {
            if (skipTitle) {
                reader.readLine();
            }
            while ((line = reader.readLine()) != null) {
                String item[] = line.split(",");
                int start = Integer.parseInt(item[0]);
                if(!id2idx.containsKey(start)) {
                	id2idx.put(start, idx++);
                	idx2id.put(idx - 1, start);
                	start = idx - 1;
                } else {
                	start = id2idx.get(start);
                }
                int end = Integer.parseInt(item[1]);
                if(!id2idx.containsKey(end)) {
                	id2idx.put(end, idx++);
                	idx2id.put(idx - 1, end);
                	end = idx - 1;
                } else {
                	end = id2idx.get(end);
                }
                while(graphF.size() < idx) graphF.add(new ArrayList());
                graphF.get(start).add(end);
                while(graphB.size() < idx) graphB.add(new ArrayList());
                graphB.get(end).add(start);
            }
            graph = new int[idx][];
            antigraph = new int[idx][];
            int m = 0;
            for(List<Integer> list : graphF) {
            	Collections.sort(list, (a, b) -> idx2id.get(a) - idx2id.get(b));
            	graph[m++] = list.stream().mapToInt(Integer::intValue).toArray();
            }
            m = 0;
            for(List<Integer> list : graphB) {
            	Collections.sort(list, (a, b) -> idx2id.get(a) - idx2id.get(b));
            	antigraph[m++] = list.stream().mapToInt(Integer::intValue).toArray();
            }
            reader.close();
        } catch (IOException exception) {
            System.err.println(exception.getMessage());
        }
        if(debug) System.out.println("finish loading data! graph size : " + idx);
    }
    
    public void checkCircle() {
    	if(debug) System.out.println("checking circle...");
    	circle = new ArrayList();
    	path = new LinkedList();
    	boolean[] visit = new boolean[graph.length];
    	int[] reachable = new int[graph.length];
    	Arrays.fill(reachable, -1);
    	for(int start = 0; start < graph.length; start++) {
    		path.offer(start);
    		prune(graph, start, start, visit, reachable, 1);
    		prune(antigraph, start, start, visit, reachable, 1);
    		for(int i : antigraph[start]) {
    			reachable[i] = -2;
    		}
    		dfs(start, start, visit, reachable);
    		path.pollLast();
    		for(int i : antigraph[start]) {
    			reachable[i] = start;
    		}
    		if(debug) System.out.println("start point: " + idx2id.get(start) + " finished!");
    	}
    	Collections.sort(circle, (a, b) -> a.size() == b.size() ? idx2id.get(a.get(0)) - idx2id.get(b.get(0)) : a.size() - b.size());
    }
    
    public void prune(int[][] g, int cur, int target, boolean[] visit, int[] reachable, int distance) {
    	for(int i : g[cur]) {
    		if(idx2id.get(i) < idx2id.get(target) || visit[i]) continue;
    		reachable[i] = target;
    		if(distance == 3) continue;
    		visit[i] = true;
    		prune(g, i, target, visit, reachable, distance + 1);
    		visit[i] = false;
    	}
    }
    
    private void dfs(int cur, int target, boolean[] visit, int[] reachable) {
    	for(int i : graph[cur]) {
    		if(idx2id.get(i) < idx2id.get(target)) continue;
    		if(reachable[i] == -2 && !visit[i]) {
    			path.offer(i);
    			if(path.size() > 2) {
    				circle.add(new ArrayList(path));
    			}
    			path.pollLast();
    		}
    		if(visit[i] || (reachable[i] != target && reachable[i] != -2)) continue;
    		if(path.size() == 6 || i == target) continue;
    		visit[i] = true;
    		path.offer(i);
    		dfs(i, target, visit, reachable);
    		path.pollLast();
    		visit[i] = false;
    	}
    }
    
    public void saveAnswer(String answerFileName) {
    	if(debug) System.out.printf("saving result...");
        try {
            BufferedWriter out = new BufferedWriter(new FileWriter(answerFileName));
            out.write(circle.size() + "\n");
            for (List<Integer> cur : circle) {
            	out.write(idx2id.get(cur.get(0)) + "");
            	for(int i = 1; i < cur.size(); i++) {
            		out.write("," + idx2id.get(cur.get(i)));
            	}
            	out.write("\n");
            }
            out.close();
        } catch (IOException exception) {
            System.err.println(exception.getMessage());
        }
    	if(debug) System.out.println("OK");
    }	
}
