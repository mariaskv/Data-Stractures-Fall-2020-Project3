///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Graph μέσω λιστών γειτνίασης.
//
///////////////////////////////////////////////////////////

#include "ADTGraph.h"
#include "ADTList.h"
#include "ADTMap.h"
#include "ADTPriorityQueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Προς υλοποίηση

struct graph{
    Map map;
    int size;
    DestroyFunc destroy_value;
    CompareFunc compare;
};

struct neighbour{
    Pointer value;
    uint weight;
    Graph owner;
};

typedef struct neighbour* Neighbour; 

int compare_neighbour(Neighbour a, Neighbour b){
    return a->owner->compare(a->value,b->value);
}

uint* create_uint(uint value) {
	uint* p = malloc(sizeof(uint));
	*p = value;
	return p;
}

Graph graph_create(CompareFunc compare, DestroyFunc destroy_vertex){
    Graph graph = malloc(sizeof(*graph));
    graph->map = map_create(compare,free,NULL);
    graph->size = 0;
    graph->destroy_value = destroy_vertex;
    graph->compare = compare;

    return graph;
}

int graph_size(Graph graph){
    return graph->size;
};

void graph_insert_vertex(Graph graph, Pointer vertex){
    MapNode node = map_find_node(graph->map,vertex);
    if(node!=NULL)
        return;
    List adj = list_create(free);
    map_insert(graph->map, vertex, adj);
    graph->size++ ;
}

List graph_get_vertices(Graph graph){
    List list = list_create(NULL);
    for(MapNode node = map_first(graph->map); node!=MAP_EOF; node = map_next(graph->map,node)){
        list_insert_next(list,LIST_EOF,map_node_key(graph->map,node));
    }
    return list;
}


void graph_remove_vertex(Graph graph, Pointer vertex){
    MapNode node = map_find_node(graph->map,vertex);
    if(node == NULL)
        return;
    else{
        if(map_node_value(graph->map,node)!=NULL){
            list_destroy((List)map_node_value(graph->map,node));
        }
        map_remove(graph->map,vertex);
        graph->size--;
    }
}

void graph_insert_edge(Graph graph, Pointer vertex1, Pointer vertex2, uint weight){
    MapNode node1 = map_find_node(graph->map,vertex1);
    if(node1 == NULL){
        graph_insert_vertex(graph,vertex1);
        node1 = map_find_node(graph->map,vertex1);
    }    
    Neighbour neighbour1 = malloc(sizeof(*neighbour1));
    neighbour1->value = vertex2;
    neighbour1->weight = weight;
    neighbour1->owner = graph;
    list_insert_next(map_node_value(graph->map,node1),LIST_BOF,neighbour1);
 
    MapNode node2 = map_find_node(graph->map,vertex2);
    if(node2 == NULL){
        graph_insert_vertex(graph,vertex2);
        node2 = map_find_node(graph->map,vertex2);
    }    
    Neighbour neighbour2 = malloc(sizeof(*neighbour2));
    neighbour2->value = vertex1;
    neighbour2->weight = weight;
    neighbour2->owner = graph;
    list_insert_next(map_node_value(graph->map,node2),LIST_BOF,neighbour2);
}

void graph_remove_edge(Graph graph, Pointer vertex1, Pointer vertex2){
    MapNode node1 = map_find_node(graph->map,vertex1);
    MapNode node2 = map_find_node(graph->map,vertex2);
    if(node1 == NULL || node2 == NULL)
        return;

    struct neighbour search1 = {.value = vertex1,.owner = graph};
    ListNode l1 = list_find_node(map_node_value(graph->map,node2),&search1,(CompareFunc)(compare_neighbour));
    if(l1 != NULL)
        list_remove(map_node_value(graph->map,node2),l1);

    struct neighbour search2 = {.value = vertex2, .owner = graph};
    ListNode l2 = list_find_node(map_node_value(graph->map,node1),&search2,(CompareFunc)(compare_neighbour));
    if(l2 != NULL)
        list_remove(map_node_value(graph->map,node1),l2);
}

uint graph_get_weight(Graph graph, Pointer vertex1, Pointer vertex2){
    MapNode node1 = map_find_node(graph->map,vertex2);
    List list = map_node_value(graph->map,node1);
    struct neighbour  search = { .value = vertex1, .owner = graph};
    ListNode ln = list_find_node(list,&search,(CompareFunc)compare_neighbour);
    if(ln != NULL){
        Neighbour neighbour = list_node_value(list,ln);
        return neighbour->weight;
    }
    return UINT_MAX;
}

List graph_get_adjacent(Graph graph, Pointer vertex){
    List list;
    MapNode node = map_find_node(graph->map,vertex);
    list = map_node_value(graph->map,node);
       return list;
}

struct vertice_info{
    int visited;
    int distance;
    Pointer prev;
    Graph owner;
};

typedef struct vertice_info* VerticeInfo;

struct pq_node_info{
    Pointer vertex;
    int distance;
    Graph owner;
};

typedef struct pq_node_info* PqNodeInfo;

int compare_pq_nodes(PqNodeInfo a, PqNodeInfo b){
    return b->distance - a->distance;
}

List graph_shortest_path(Graph graph, Pointer source, Pointer target){
    Map MapInfo = map_create(graph->compare,NULL,free);
    map_set_hash_function(MapInfo,hash_int);

    PriorityQueue pqueue = pqueue_create((CompareFunc)compare_pq_nodes,NULL,NULL);
    List vertices = graph_get_vertices(graph);

    for(ListNode node = list_first(vertices); node!=LIST_EOF; node = list_next(vertices,node)){
        VerticeInfo vinfo = malloc(sizeof(*vinfo));
        PqNodeInfo pqinfo = malloc(sizeof(*pqinfo));

        vinfo->visited = 0;
        vinfo->prev = NULL;
        vinfo->owner = graph;

        pqinfo->vertex = list_node_value(vertices,node);
        pqinfo->owner = graph;

        if(!graph->compare(list_node_value(vertices,node),source)){
            vinfo->distance = 0;
            pqinfo->distance = 0;            
        }
        else{
            vinfo->distance = INT_MAX;
            pqinfo->distance = INT_MAX;
        }

        map_insert(MapInfo,list_node_value(vertices,node),vinfo);
        pqueue_insert(pqueue,pqinfo);
    }
    list_destroy(vertices);

    while(pqueue_size(pqueue)){
        PqNodeInfo w = pqueue_max(pqueue);
        pqueue_remove_max(pqueue);

        MapNode mapnode = map_find_node(MapInfo,w->vertex);
        VerticeInfo wInfo = map_node_value(MapInfo,mapnode);

        if(wInfo->visited){
            free(w);
            continue;
        }

        wInfo->visited = 1;

        if(!graph->compare(w->vertex,target)){
            free(w);
            break;
        }

        List neighbours = graph_get_adjacent(graph,w->vertex);
        for(ListNode node = list_first(neighbours); node!=LIST_EOF; node = list_next(neighbours,node)){
            Neighbour neighbour = list_node_value(vertices,node);

            MapNode mapnode = map_find_node(MapInfo,neighbour->value);
            VerticeInfo nInfo = map_node_value(MapInfo,mapnode);
            if(nInfo->visited)
                continue;

            int alt = wInfo->distance + neighbour->weight;

            if(nInfo->distance == INT_MAX || alt < nInfo->distance){
                nInfo->distance = alt;
                nInfo->prev = w->vertex;
                PqNodeInfo ninfoNode = malloc(sizeof(*ninfoNode));
                ninfoNode->vertex = neighbour->value;
                ninfoNode->distance = alt;
                ninfoNode->owner = graph;
                pqueue_insert(pqueue,ninfoNode);
            }    
        }
        free(w);
    }

    MapNode m = map_find_node(MapInfo,target);
    VerticeInfo info = map_node_value(MapInfo,m);

    List list = list_create(NULL);
    list_insert_next(list,LIST_EOF,target);

    while(info->prev != NULL){
        list_insert_next(list,LIST_EOF,info->prev);
        m = map_find_node(MapInfo,info->prev);
        info = map_node_value(MapInfo,m);
    }

    pqueue_set_destroy_value(pqueue,free);
    pqueue_destroy(pqueue);
    map_destroy(MapInfo);
    return list;
}

void graph_destroy(Graph graph){ 
for(MapNode node = map_first(graph->map); node!= NULL; node = map_next(graph->map,node)){ 
    list_destroy(map_node_value(graph->map,node)); 
} 
    map_destroy(graph->map); 
    free(graph);
}

void graph_set_hash_function(Graph graph, HashFunc hash_func){
    map_set_hash_function(graph->map,hash_func);
}