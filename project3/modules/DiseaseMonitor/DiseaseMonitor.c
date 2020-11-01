
#include "DiseaseMonitor.h"
#include "ADTMap.h"
#include "ADTPriorityQueue.h"
#include "ADTSet.h"
#include "ADTList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Map countries = NULL;
static Map diseases = NULL;
static Map ids = NULL;
static Set disease_monitor = NULL;
static Map top = NULL;
static PriorityQueue pqueue = NULL;

static int compare_countries(String a, String b){
    return strcmp(a,b);
}

static int compare_diseases(String a, String b){
    return strcmp(a,b);
}

int compare_ids(Pointer a, Pointer b) {
	return *(int*)a - *(int*)b;
}

int compare_date(Record a, Record b){
    if(strcmp(a->date,b->date)!=0)
        return strcmp(a->date,b->date);

    return a->id - b->id;
}

static int compare(Record a, Record b){
    if(strcmp(a->disease,b->disease)!=0)
        return strcmp(a->disease,b->disease);

    else if(strcmp(a->country,b->country)!=0)
        return strcmp(a->country,b->country);

    else if(strcmp(a->name,b->name)!=0)
        return strcmp(a->name,b->name);

    return a->id - b->id;
}

struct top_node{
    String disease;
    int counter;
};

typedef struct top_node* TopNode;

static int compare_top(TopNode a, TopNode b){
    if(a->counter != b->counter)
        return a->counter - b->counter;
    else
        return strcmp(a->disease,b->disease);        
}

void dm_init(){
    countries = map_create((CompareFunc)compare_countries,NULL,NULL);
    map_set_hash_function(countries,hash_string);

    diseases = map_create((CompareFunc)compare_diseases,NULL,NULL);
    map_set_hash_function(diseases,hash_string);

    ids = map_create((CompareFunc)compare_ids,NULL,NULL);
    map_set_hash_function(ids,hash_int);

    disease_monitor = set_create((CompareFunc)compare,NULL); 

    top = map_create((CompareFunc)compare_countries,NULL,NULL);
    map_set_hash_function(top,hash_string);

    pqueue = pqueue_create((CompareFunc)compare_top,NULL,NULL);
}

void dm_destroy(){
    for(MapNode node = map_first(ids); node != MAP_EOF; node = map_next(ids,node))
        set_destroy(map_node_value(ids,node));

    for(MapNode node = map_first(countries); node != MAP_EOF; node = map_next(countries,node))
        set_destroy(map_node_value(countries,node));

    for(MapNode node = map_first(diseases); node != MAP_EOF; node = map_next(diseases,node))
        set_destroy(map_node_value(diseases,node));    

    for(MapNode node = map_first(top); node != MAP_EOF; node = map_next(top,node))
        set_destroy(map_node_value(top,node));             

    map_destroy(top);
    map_destroy(countries);
    map_destroy(diseases);
    map_destroy(ids);
    set_destroy(disease_monitor);
    pqueue_set_destroy_value(pqueue,free);
    pqueue_destroy(pqueue);
}

List dm_get_records(String disease, String country, Date date_from, Date date_to){

    List list = list_create(NULL); 

    if(disease != NULL){
        MapNode DisNode = map_find_node(diseases,disease);
        if(DisNode == NULL)
            return list;

        Set DisSet = map_node_value(diseases,DisNode);
        for(SetNode node = set_first(DisSet); node != SET_EOF; node = set_next(DisSet,node)){
            Record record = set_node_value(DisSet,node);
            if((date_from != NULL && strcmp(record->date,date_from) < 0) || (date_to != NULL && strcmp(record->date,date_to) > 0) ){
                continue;
            }
            else{
                if(country == NULL || strcmp(record->country,country)==0){
                    list_insert_next(list,LIST_EOF,record);
                }
            }
        }
    }
    else if(country != NULL){
        MapNode CNode = map_find_node(countries,country);
        if(CNode == NULL)
            return list;
        Set CSet = map_node_value(countries,CNode);
        for(SetNode node = set_first(CSet); node != SET_EOF; node = set_next(CSet,node)){
            Record record = set_node_value(CSet,node);
            if((date_from != NULL && strcmp(record->date,date_from)<0) || (date_to != NULL && strcmp(record->date,date_to) > 0) ){
                continue;
            }
            else{
                list_insert_next(list,LIST_EOF,record);
            }
        }
    }
    else{
        for(SetNode node = set_first(disease_monitor);node!=SET_EOF;node=set_next(disease_monitor,node)){
            Record r = set_node_value(disease_monitor,node);
            if((disease == NULL || strcmp(disease,r->disease)==0) && (country == NULL || strcmp(country,r->country)==0) && (date_from == NULL || strcmp(r->date,date_from)>=0) && (date_to == NULL || strcmp(r->date,date_to)<=0)){
                list_insert_next(list,LIST_EOF,set_node_value(disease_monitor,node));
            }   
        }
    }

        // printf("new\n");
        // for(MapNode node = map_first(countries);node!=MAP_EOF;node = map_next(countries,node)){
        // Set set = map_node_value(countries,node);
        // for(SetNode nod = set_first(set); nod!=SET_EOF; nod = set_next(set,nod)){
        //     Record r = set_node_value(set,nod);
        //     printf("%s %s %s %d %s\n",r->country,r->name,r->disease,r->id,r->date);
        // }
        // printf("stop\n");
        // }

    return list;
}

int dm_count_records(String disease, String country, Date date_from, Date date_to){
    List list = dm_get_records(disease,country,date_from,date_to);
    int size = list_size(list);
    list_destroy(list);
    return size;
}

List dm_top_diseases(int k, String country){
    List list = list_create(NULL);
    if(country != NULL){
        MapNode node = map_find_node(top,country);
        if(node == NULL)
            return list;
    
        Set set = map_node_value(top,node);
        SetNode Snode = set_first(set);
        for(int i = 0; i < k; i++){
            TopNode node = set_node_value(set,Snode);
            list_insert_next(list,LIST_EOF,node);
            Snode = set_next(set,Snode);
        }
    }
    else{
        List temp = list_create(NULL);

        for(int i = 1; i <= k; i++){
        //printf("size: %d, k:%d\n",pqueue_size(pqueue),k);  
            PriorityQueueNode PQnode = pqueue_max(pqueue);
            TopNode node = pqueue_node_value(pqueue,PQnode);
            list_insert_next(list,LIST_EOF,node);
            list_insert_next(temp,LIST_EOF,PQnode);
            pqueue_remove_max(pqueue);
        }        
        for(ListNode ln = list_first(temp); ln != LIST_EOF; ln = list_next(temp,ln)){
            pqueue_insert(pqueue,list_node_value(temp,ln));
        }
        list_destroy(temp);
    }
    return list;
}

bool dm_insert_record(Record record){
    set_insert(disease_monitor,record);

    bool flag1 = true;
    bool flag2 = true;
    bool flag3 = true;

    MapNode Mtop = map_find_node(top,record->country);
    if(Mtop == NULL){
        Set TopSet = set_create((CompareFunc)compare_top,NULL);
        map_insert(top,record->country,TopSet);
        Mtop = map_find_node(top,record->country);
    }
        Set InSet = map_node_value(top,Mtop);
        struct top_node search = {.disease = record->disease};
        SetNode find = set_find_node(InSet,&search);
        if(find != NULL){
            TopNode node = set_node_value(InSet,find);
            pqueue_remove_node(pqueue,(PriorityQueueNode)node);
            node->counter ++;
            set_insert(InSet,node);
            pqueue_insert(pqueue,node);
        }
        else{
            TopNode node = malloc(sizeof(node));
            node->counter = 1;
            node->disease = record->disease;
            set_insert(InSet,node);
            pqueue_insert(pqueue,node);
        }

    MapNode node1 = map_find_node(ids,&(record->id));
    if(node1 == NULL){
        flag1 = false;
        Set set = set_create((CompareFunc)compare_ids,NULL);
        map_insert(ids,&(record->id),set);
        node1 = map_find_node(ids,&(record->id));    
    }
        Set set1 = map_node_value(ids,node1);
        SetNode n1 = set_find_node(set1,record);
        if(n1 == NULL)
            flag1 = false;        
        set_insert(set1,record);

    MapNode node2 = map_find_node(countries,record->country);
    if(node2 == NULL){
        flag2 = false;
        Set set2 = set_create((CompareFunc)compare_date,NULL);
        map_insert(countries,record->country,set2);
        node2 = map_find_node(countries,record->country);
    }
        Set set3 = map_node_value(countries,node2);
        SetNode n2 = set_find_node(set3,record);
        if(n2 == NULL)
            flag2 = false;        
        set_insert(set3,record);

    MapNode node3 = map_find_node(diseases,record->disease);
    if(node3 == NULL){
        flag3 = false;
        Set set4 = set_create((CompareFunc)compare_date,NULL);
        map_insert(diseases,record->disease,set4);
        node3 = map_find_node(diseases,record->disease);
    }
        Set set5 = map_node_value(diseases,node3);
        SetNode n3 = set_find_node(set5,record);
        if(n3 == NULL)
            flag3 = false;

        set_insert(set5,record); 

    if(flag1 == false && flag2 == false && flag3 == false)
        return false;
    else
        return true;
}

bool dm_remove_record(int id){

    // for(MapNode node = map_first(ids);node!=MAP_EOF;node = map_next(ids,node)){
    //     printf("%d\n",*(int*)map_node_key(ids,node));
    // }
    // printf("new\n");

    MapNode node = map_find_node(ids,&id);      
    if(node == NULL)
        return false;

    Set set = map_node_value(ids,node);

    struct record search = { .id = id};
    SetNode find = set_find_node(set,&search);

    if(find == NULL)
        return false;

    Record record = set_node_value(set,find);
    set_remove(set,record);  

    MapNode Mnode = map_find_node(top,record->country);
    if(Mnode != NULL){

        Set OutSet = map_node_value(top,Mnode);
        struct top_node search = {.disease = record->disease};
        SetNode find = set_find_node(OutSet,&search);

        if(find != NULL){
            TopNode node = set_node_value(OutSet,find);
            node->counter --;
            set_insert(OutSet,node);
            pqueue_remove_node(pqueue,(PriorityQueueNode)node);
            if(node->counter != 0)
                pqueue_insert(pqueue,node);
        }     
    } 

    if(set_size(set) == 0){
        set_destroy(set);
        map_remove(ids,&id);
    }


    MapNode node1 = map_find_node(countries,record->country);
    if(node1 == NULL)
        return false; 

    Set set1 = map_node_value(countries,node1);   
    set_remove(set1,record);   
    if(set_size(set1) == 0){
        set_destroy(set1);
        map_remove(countries,record->country);
    }    

    MapNode node2 = map_find_node(diseases,record->disease);
    if(node2 == NULL)
        return false; 

    Set set2 = map_node_value(diseases,node2);    
    if(set_size(set2) == 0){
        set_destroy(set2);
        map_remove(diseases,record->disease);
    }           
    set_remove(set2,record);    

    set_remove(disease_monitor,record);
    return true;   

}


