/////////////////////////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Map μέσω Hash Table με open addressing (linear probing)
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "ADTMap.h"
#include "ADTList.h"

// Οι κόμβοι του map στην υλοποίηση με hash table, μπορούν να είναι σε 3 διαφορετικές καταστάσεις,
// ώστε αν διαγράψουμε κάποιον κόμβο, αυτός να μην είναι empty, ώστε να μην επηρεάζεται η αναζήτηση
// αλλά ούτε occupied, ώστε η εισαγωγή να μπορεί να το κάνει overwrite.

// Το μέγεθος του Hash Table ιδανικά θέλουμε να είναι πρώτος αριθμός σύμφωνα με την θεωρία.
// Η παρακάτω λίστα περιέχει πρώτους οι οποίοι έχουν αποδεδιγμένα καλή συμπεριφορά ως μεγέθη.
// Κάθε re-hash θα γίνεται βάσει αυτής της λίστας. Αν χρειάζονται παραπάνω απο 1610612741 στοχεία, τότε σε καθε rehash διπλασιάζουμε το μέγεθος.
int prime_sizes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
	786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};

// Χρησιμοποιούμε open addressing, οπότε σύμφωνα με την θεωρία, πρέπει πάντα να διατηρούμε
// τον load factor του  hash table μικρότερο ή ίσο του 0.5, για να έχουμε αποδoτικές πράξεις
#define MAX_LOAD_FACTOR 0.9

// Δομή του κάθε κόμβου που έχει το hash table (με το οποίο υλοιποιούμε το map)
struct map_node{
	Pointer key;		// Το κλειδί που χρησιμοποιείται για να hash-αρουμε
	Pointer value;  	// Η τιμή που αντισοιχίζεται στο παραπάνω κλειδί
	Map owner;
};

// Δομή του Map (περιέχει όλες τις πληροφορίες που χρεαζόμαστε για το HashTable)
struct map {
	List* array;
	int capacity;				// Πόσο χώρο έχουμε δεσμεύσει.
	int size;					// Πόσα στοιχεία έχουμε προσθέσει
	CompareFunc compare;		// Συνάρτηση για σύγκρηση δεικτών, που πρέπει να δίνεται απο τον χρήστη
	HashFunc hash_function;		// Συνάρτηση για να παίρνουμε το hash code του κάθε αντικειμένου.
	DestroyFunc destroy_key;	// Συναρτήσεις που καλούνται όταν διαγράφουμε έναν κόμβο απο το map.
	DestroyFunc destroy_value;
};

static int compare_map_nodes(MapNode a, MapNode b) {
	return a->owner->compare(a->key, b->key);
}

static void destroy_map_node(MapNode node) {
	if (node->owner->destroy_key != NULL)
		node->owner->destroy_key(node->key);

	if (node->owner->destroy_value != NULL)
		node->owner->destroy_value(node->value);

	free(node);
}

Map map_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value) {
	// Δεσμεύουμε κατάλληλα τον χώρο που χρειαζόμαστε για το hash table
	Map map = malloc(sizeof(*map));
	map->capacity = prime_sizes[0];

	map->array = (List*)malloc(map->capacity * sizeof(List));
    for(int i = 0; i< map->capacity; i++){
        map->array[i] = (List)list_create(free);
    }


	map->size = 0;
	map->compare = compare;
	map->destroy_key = destroy_key;
	map->destroy_value = destroy_value;

	return map;
}

// Επιστρέφει τον αριθμό των entries του map σε μία χρονική στιγμή.
int map_size(Map map) {
	return map->size;
}

// Συνάρτηση για την επέκταση του Hash Table σε περίπτωση που ο load factor μεγαλώσει πολύ.
static void rehash(Map map) {
	// Αποθήκευση των παλιών δεδομένων
	int old_capacity = map->capacity;
	List* old_array = map->array;

	// Βρίσκουμε τη νέα χωρητικότητα, διασχίζοντας τη λίστα των πρώτων ώστε να βρούμε τον επόμενο. 
	int prime_no = sizeof(prime_sizes) / sizeof(int);	// το μέγεθος του πίνακα
	for (int i = 0; i < prime_no; i++) {					// LCOV_EXCL_LINE
		if (prime_sizes[i] > old_capacity) {
			map->capacity = prime_sizes[i]; 
			break;
		}
	}
	// Αν έχουμε εξαντλήσει όλους τους πρώτους, διπλασιάζουμε
	if (map->capacity == old_capacity)					// LCOV_EXCL_LINE
		map->capacity *= 2;								// LCOV_EXCL_LINE

	// Δημιουργούμε ένα μεγαλύτερο hash table
	map->array = (List*)malloc(map->capacity * sizeof(List));
    for(int i = 0; i< map->capacity; i++){
        map->array[i] = (List)list_create(free);
    }

	// Τοποθετούμε ΜΟΝΟ τα entries που όντως περιέχουν ένα στοιχείο (το rehash είναι και μία ευκαιρία να ξεφορτωθούμε τα deleted nodes)
	map->size = 0;
	for (int i = 0; i < old_capacity; i++){
		for(ListNode new = list_first(old_array[i]); new!=LIST_EOF; new = list_next(old_array[i],new)){
			MapNode inserted = (MapNode)list_node_value(old_array[i],new);
			map_insert(map,inserted->key,inserted->value);
		}
	}
	for(int i = 0; i < old_capacity; i++)
		list_destroy(old_array[i]);

	free(old_array);
}

// Εισαγωγή στο hash table του ζευγαριού (key, item). Αν το key υπάρχει,
// ανανέωση του με ένα νέο value, και η συνάρτηση επιστρέφει true.

void map_insert(Map map, Pointer key, Pointer value) {

	uint pos = map->hash_function(key) % map->capacity;

	struct map_node search_node = { .key = key, .value = value, .owner = map};

	MapNode node = (MapNode)list_find_node(map->array[pos], &search_node, (CompareFunc)(compare_map_nodes));
	if(node == NULL){
		MapNode node1 = malloc(sizeof(*node1));
		node1->key = key;
		node1->value = value;
		node1->owner = map;
		ListNode test;
		ListNode temp;
		if(list_size(map->array[pos])>0 && compare_map_nodes((MapNode)list_node_value(map->array[pos],list_first(map->array[pos])),node1)>0){
			test=list_first(map->array[pos]); 
			while(test!=LIST_EOF){
				if(compare_map_nodes((MapNode)list_node_value(map->array[pos],list_first(map->array[pos])),node1)<=0){
					break;
				}
			temp=test;
			test=list_next(map->array[pos],test);
			}
		}
		else{
			temp=LIST_BOF;
		}
		list_insert_next(map->array[pos],temp,node1);
	}
	else{
		list_set_destroy_value(map->array[pos],(DestroyFunc)destroy_map_node);
		list_remove(map->array[pos],(ListNode)node);
		list_set_destroy_value(map->array[pos],free);
		MapNode node1 = malloc(sizeof(*node1));
		node1->key = key;
		node1->value = value;
		node1->owner = map;
		ListNode test;
		ListNode temp;
		if(list_size(map->array[pos])>0 && map->compare((MapNode)list_node_value(map->array[pos],list_first(map->array[pos])),node1)>0){
			test=list_first(map->array[pos]); 
			while(test!=LIST_EOF){
				if(map->compare((MapNode)list_node_value(map->array[pos],list_first(map->array[pos])),node1)<=0){
					break;
				}
			temp=test;
			test=list_next(map->array[pos],test);
			}
		}
		else{
			temp=LIST_BOF;
		}
		list_insert_next(map->array[pos],temp,node1);	}
    map->size ++;

	// Αν με την νέα εισαγωγή ξεπερνάμε το μέγιστο load factor, πρέπει να κάνουμε rehash
	float load_factor = (float)map->size / map->capacity;
	if (load_factor > MAX_LOAD_FACTOR)
		rehash(map);

}

// Διαργραφή απο το Hash Table του κλειδιού με τιμή key
bool map_remove(Map map, Pointer key) {
	uint pos = map->hash_function(key) % map->capacity;
	struct map_node search_node = {.key = key, .value = NULL, .owner = map};
	ListNode node = list_find_node(map->array[pos], &search_node, (CompareFunc)compare_map_nodes);
	if(node == NULL)
		return false;
	else{
		list_set_destroy_value(map->array[pos],(DestroyFunc)destroy_map_node);
		list_remove(map->array[pos],node);
		list_set_destroy_value(map->array[pos],free);
        map->size --;
        return true;
	}
}

// Αναζήτηση στο map, με σκοπό να επιστραφεί το value του κλειδιού που περνάμε σαν όρισμα.

Pointer map_find(Map map, Pointer key) {
	uint pos = map->hash_function(key) % map->capacity;
	struct map_node search_node = {.key = key, .value = NULL, .owner = map};
	ListNode node = list_find_node(map->array[pos], &search_node, (CompareFunc)compare_map_nodes);
    if(node == NULL)
        return NULL;
    else{
        MapNode new = (MapNode)list_node_value(map->array[pos], node);
        return new->value;
    }
}

DestroyFunc map_set_destroy_key(Map map, DestroyFunc destroy_key) {
	DestroyFunc old = map->destroy_key;
	map->destroy_key = destroy_key;
	return old;
}

DestroyFunc map_set_destroy_value(Map map, DestroyFunc destroy_value) {
	DestroyFunc old = map->destroy_value;
	map->destroy_value = destroy_value;
	return old;
}

// Απελευθέρωση μνήμης που δεσμεύει το map
void map_destroy(Map map) {

	for(int i =0 ; i < map->capacity; i++){
		list_set_destroy_value(map->array[i],(DestroyFunc)destroy_map_node);
		list_destroy(map->array[i]);
	}
	free(map->array);
	free(map);
}

/////////////////////// Διάσχιση του map μέσω κόμβων ///////////////////////////

MapNode map_first(Map map) {
	//Ξεκινάμε την επανάληψή μας απο το 1ο στοιχείο, μέχρι να βρούμε κάτι όντως τοποθετημένο
	for (int i = 0; i < map->capacity; i++)
		if (list_size(map->array[i])>0)
			return (MapNode)list_node_value(map->array[i],list_first(map->array[i]));

	return MAP_EOF;
}

MapNode map_next(Map map, MapNode node) {
	uint pos = map->hash_function(node->key) % map->capacity;
	uint pos_temp = pos;
	if(map->size == 1)
		return MAP_EOF;
	
	ListNode list_node = list_find_node(map->array[pos], node, (CompareFunc)compare_map_nodes);
	ListNode next_node = list_next(map->array[pos], list_node);
	if(next_node != NULL)
		return list_node_value(map->array[pos],next_node);
		
	else{
		ListNode lnode;
		pos = (pos + 1) % map->capacity;
		lnode = list_first(map->array[pos]);
		while(lnode == NULL && pos > pos_temp){
			pos = (pos + 1) % map->capacity;
			lnode = list_first(map->array[pos]);
		}
		if(pos <= pos_temp)
			return MAP_EOF;

		return list_node_value(map->array[pos],lnode);
	}
}

Pointer map_node_key(Map map, MapNode node) {
	return node->key;
}

Pointer map_node_value(Map map, MapNode node) {
	return node->value;
}

MapNode map_find_node(Map map, Pointer key) {
	uint pos = map->hash_function(key) % map->capacity;
	for(int i = pos; i < map->capacity; i++){
		for(ListNode node = list_first(map->array[i]); node!= LIST_EOF; node = list_next(map->array[i],node)){
			MapNode temp = (MapNode)list_node_value(map->array[i],node);
			if(map->compare(temp->key,key)==0)
				return temp;
		}
	}
	return MAP_EOF;
}

// Αρχικοποίηση της συνάρτησης κατακερματισμού του συγκεκριμένου map.
void map_set_hash_function(Map map, HashFunc func) {
	map->hash_function = func;
}

uint hash_string(Pointer value) {
	// djb2 hash function, απλή, γρήγορη, και σε γενικές γραμμές αποδοτική
    uint hash = 5381;
    for (char* s = value; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s;			// hash = (hash * 33) + *s. Το foo << 5 είναι γρηγορότερη εκδοχή του foo * 32.
    return hash;
}

uint hash_int(Pointer value) {
	return *(int*)value;
}

uint hash_pointer(Pointer value) {
	return (size_t)value;				// cast σε sizt_t, που έχει το ίδιο μήκος με έναν pointer
}