//////////////////////////////////////////////////////////////////
//
// Unit tests για τον ADT Graph.
// Οποιαδήποτε υλοποίηση οφείλει να περνάει όλα τα tests.
//
//////////////////////////////////////////////////////////////////

#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing
#include <limits.h>
#include "ADTGraph.h"
#include "ADTList.h"


int compare_ints(Pointer a, Pointer b) {
	return *(int*)a - *(int*)b;
}

int* create_int(int value) {
	int* p = malloc(sizeof(int));
	*p = value;
	return p;
}

void test_create(void) {

	// Δημιουργούμε μια κενή λίστα (χωρίς αυτόματο free)
	Graph graph = graph_create(compare_ints, free);
	graph_set_hash_function(graph, hash_int);
	// Ελέγχουμε ότι δεν απέτυχε η malloc στην λίστα, και ότι
	// αρχικοποιείται με μέγεθος 0 (δηλαδή χωρίς κόμβους)
	TEST_ASSERT(graph != NULL);
	TEST_ASSERT(graph_size(graph) == 0);

	graph_destroy(graph);
}

void test_insert(void) {
	Graph graph = graph_create(compare_ints,NULL);
	graph_set_hash_function(graph, hash_int);

	// Θα προσθέτουμε, μέσω της insert, δείκτες ως προς τα στοιχεία του π΄ίνακα
	int N = 1000;					

	for (int i = 0; i < N; i++) {
		graph_insert_vertex(graph,create_int(i));
		
		TEST_ASSERT(graph_size(graph) == (i + 1));	

	}

	graph_destroy(graph);
}

void test_get_vertices(void){
	Graph graph = graph_create(compare_ints,NULL);
	graph_set_hash_function(graph, hash_int);

	int N = 1000;

	for (int i = 0; i < N; i++) {
		graph_insert_vertex(graph,create_int(i));
		List list = graph_get_vertices(graph);
		TEST_ASSERT(list_size(list) == graph_size(graph));
		TEST_ASSERT(*(int*)list_node_value(list,list_first(list)) == i);
		list_destroy(list);
	}
	graph_destroy(graph);
}

void test_remove_vertex(void) {

	Graph graph = graph_create(compare_ints, free);
	graph_set_hash_function(graph, hash_int);

	int N = 1000;
	int** key_array = malloc(N * sizeof(*key_array));

	for (int i = 0; i < N; i++) {
		key_array[i] = create_int(i);

		graph_insert_vertex(graph, key_array[i]);
	}

	for (int i = 0; i < N; i++) {
		
		TEST_ASSERT(graph_size(graph) == N-i);
		graph_remove_vertex(graph, key_array[i]);
	}

	// Δοκιμάζουμε, πριν διαγράψουμε κανονικά τους κόμβους, ότι η map_remove
	// διαχειρίζεται σωστά ένα κλειδί που δεν υπάρχει στο Map
	int not_exists = 2000;
	graph_remove_vertex(graph, &not_exists);
	TEST_ASSERT(graph_size(graph) == 0);

	graph_destroy(graph);
	free(key_array);
}

void test_edges(void){
	Graph graph = graph_create(compare_ints,NULL);
	graph_set_hash_function(graph, hash_int);

	int N = 1000;
	int** array = malloc(N*sizeof(int*));
	for(int i = 0; i <= N; i++){
		array[i] = create_int(i);
	}
	for (uint i = 0; i < N; i++) {
		graph_insert_vertex(graph,array[i]);
	}

	for (uint i = 0; i < N/2; i++) {
		graph_insert_edge(graph,array[i],array[N-i],i);
	}

	for(uint i = 0; i < N/2; i++){
		List list  = graph_get_adjacent(graph,array[i]);
		TEST_ASSERT(list_size(list) == 1);
		uint weight = graph_get_weight(graph,array[i],array[N-i]);
		TEST_ASSERT(weight == i);

	}

	for(int i = 0; i < N; i++){
		graph_remove_edge(graph,array[i],array[N-i]);
		List list = graph_get_adjacent(graph,array[i]);
		TEST_ASSERT(list_size(list) == 0);
	}	
	graph_destroy(graph);
	free(array);
}

void test_shortest_path(void){
	Graph graph = graph_create(compare_ints,NULL);
	graph_set_hash_function(graph,hash_int);

	int N = 6;
	int** array = malloc(N*sizeof(int*));

	for(int i = 0; i <= N; i++){
		array[i] = create_int(i+1);
	}

	for (uint i = 0; i <= N; i++) {
		graph_insert_vertex(graph,array[i]);
	}

	graph_insert_edge(graph,array[0],array[1],3);
	graph_insert_edge(graph,array[0],array[5],5);

	graph_insert_edge(graph,array[1],array[2],7);
	graph_insert_edge(graph,array[1],array[5],10);

	graph_insert_edge(graph,array[2],array[4],1);
	graph_insert_edge(graph,array[2],array[5],8);
	graph_insert_edge(graph,array[2],array[3],5);

	graph_insert_edge(graph,array[3],array[5],2);
	graph_insert_edge(graph,array[3],array[4],6);
	
	graph_insert_edge(graph,array[5],array[4],7);

	List list = graph_shortest_path(graph,array[0],array[3]);
	TEST_ASSERT(list_size(list)==3);
	list_destroy(list);

	list = graph_shortest_path(graph,array[1],array[4]);
	TEST_ASSERT(list_size(list)==3);
	list_destroy(list);

	list = graph_shortest_path(graph,array[3],array[5]);
	TEST_ASSERT(list_size(list)==2);
	list_destroy(list);

	list = graph_shortest_path(graph,array[1],array[0]);
	TEST_ASSERT(list_size(list)==2);
	list_destroy(list);

	list = graph_shortest_path(graph,array[0],array[6]);
	TEST_ASSERT(list_size(list)==1);	
	list_destroy(list);

	list = graph_shortest_path(graph,array[2],array[0]);
	TEST_ASSERT(list_size(list)==3);
	list_destroy(list);
	
	graph_destroy(graph);
	free(array);

}

// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {

	{ "graph_create", test_create },
	{ "graph_insert", test_insert },
	{ "graph_get_vertices", test_get_vertices},
	{ "graph_remove_vertises", test_remove_vertex},
	{ "graph_edges", test_edges},
	{ "graph_shortest_path", test_shortest_path},


	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};