///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Priority Queue μέσω σωρού.
//
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <assert.h>

#include "ADTPriorityQueue.h"
#include "ADTVector.h"			// Η υλοποίηση του PriorityQueue χρησιμοποιεί Vector

// Ενα PriorityQueue είναι pointer σε αυτό το struct
struct priority_queue {
	Vector vector;				// Τα δεδομένα, σε Vector ώστε να έχουμε μεταβλητό μέγεθος χωρίς κόπο
	CompareFunc compare;		// Η διάταξη
	DestroyFunc destroy_value;	// Συνάρτηση που καταστρέφει ένα στοιχείο του vector.
};

struct priority_queue_node{
    Pointer value;
	PriorityQueue owner;
	int pos;
};

typedef struct priority_queue_node* PriorityQueueNode;

static void destroy_priority_queue_node(PriorityQueueNode node) { //destroy function for pqnodes(uses the given destroy value)
 
	if (node->owner->destroy_value != NULL)
		node->owner->destroy_value(node->value);

	free(node);
}

// Βοηθητικές συναρτήσεις ////////////////////////////////////////////////////////////////////////////

// Προσοχή: στην αναπαράσταση ενός complete binary tree με πίνακα, είναι βολικό τα ids των κόμβων να
// ξεκινάνε από το 1 (ρίζα), το οποίο απλοποιεί τις φόρμουλες για εύρεση πατέρα/παιδιών. Οι θέσεις
// ενός vector όμως ξεκινάνε από το 0. Θα μπορούσαμε απλά να αφήσουμε μία θέση κενή, αλλά δεν είναι ανάγκη,
// μπορούμε απλά να αφαιρούμε 1 όταν διαβάζουμε/γράφουμε στο vector. Για απλοποίηση του κώδικα, η
// πρόσβαση στα στοιχεία του vector γίνεται από τις παρακάτω 2 βοηθητικές συναρτήσεις.

// Επιστρέφει την τιμή του κόμβου node_id

static Pointer node_value(PriorityQueue pqueue, int node_id) {
	// τα node_ids είναι 1-based, το node_id αποθηκεύεται στη θέση node_id - 1
	PriorityQueueNode node = (PriorityQueueNode)vector_get_at(pqueue->vector, node_id - 1);
	return node->value;
}


// Ανταλλάσει τις τιμές των κόμβων node_id1 και node_id2

static void node_swap(PriorityQueue pqueue, int node_id1, int node_id2) {
	// τα node_ids είναι 1-based, το node_id αποθηκεύεται στη θέση node_id - 1

	PriorityQueueNode value1 = (PriorityQueueNode)vector_get_at(pqueue->vector, node_id1 - 1);
	PriorityQueueNode value2 = (PriorityQueueNode)vector_get_at(pqueue->vector, node_id2 - 1);

	int temp = value1->pos; //swap nodes id's
	value1->pos = value2->pos;
	value2->pos = temp;

	vector_set_at(pqueue->vector, node_id1 - 1, value2);
	vector_set_at(pqueue->vector, node_id2 - 1, value1);

}


// Αποκαθιστά την ιδιότητα του σωρού.
// Πριν: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού, εκτός από
//       τον node_id που μπορεί να είναι _μεγαλύτερος_ από τον πατέρα του.
// Μετά: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού.

static void bubble_up(PriorityQueue pqueue, int node_id) {
	// Αν φτάσαμε στη ρίζα, σταματάμε
	if (node_id == 1)
		return;

	int parent = node_id / 2;		// Ο πατέρας του κόμβου. Τα node_ids είναι 1-based

	// Αν ο πατέρας έχει μικρότερη τιμή από τον κόμβο, swap και συνεχίζουμε αναδρομικά προς τα πάνω
	if (pqueue->compare(node_value(pqueue, parent), node_value(pqueue, node_id)) < 0) {
		node_swap(pqueue, parent, node_id);
		bubble_up(pqueue, parent);
	}
}

// Αποκαθιστά την ιδιότητα του σωρού.
// Πριν: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού, εκτός από τον
//       node_id που μπορεί να είναι _μικρότερος_ από κάποιο από τα παιδιά του.
// Μετά: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού.

static void bubble_down(PriorityQueue pqueue, int node_id) {
	// βρίσκουμε τα παιδιά του κόμβου (αν δεν υπάρχουν σταματάμε)
	int left_child = 2 * node_id;
	int right_child = left_child + 1;

	int size = pqueue_size(pqueue);
	if (left_child > size)
		return;

	// βρίσκουμε το μέγιστο από τα 2 παιδιά
	int max_child = left_child;
	if (right_child <= size && pqueue->compare(node_value(pqueue, left_child), node_value(pqueue, right_child)) < 0)
			max_child = right_child;

	// Αν ο κόμβος είναι μικρότερος από το μέγιστο παιδί, swap και συνεχίζουμε προς τα κάτω
	if (pqueue->compare(node_value(pqueue, node_id), node_value(pqueue, max_child)) < 0) {
		node_swap(pqueue, node_id, max_child);
		bubble_down(pqueue, max_child);
	}
}

// Αρχικοποιεί το σωρό από τα στοιχεία του vector values.

static void efficient_heapify(PriorityQueue pqueue, Vector values) {
	for(int i=0;i<vector_size(values);i++){
	
		PriorityQueueNode node = malloc(sizeof(*node));
		node->value = vector_get_at(values,i);
		node->owner = pqueue;
		node->pos = pqueue_size(pqueue) + 1;

		vector_insert_last(pqueue->vector,node);
	}
	bubble_down(pqueue,1);
}

// Συναρτήσεις του ADTPriorityQueue //////////////////////////////////////////////////

PriorityQueue pqueue_create(CompareFunc compare, DestroyFunc destroy_value, Vector values) {
	assert(compare != NULL);	// LCOV_EXCL_LINE

	PriorityQueue pqueue = malloc(sizeof(*pqueue));
	pqueue->compare = compare;
	pqueue->destroy_value = destroy_value;

	// Δημιουργία του vector που αποθηκεύει τα στοιχεία.
	// ΠΡΟΣΟΧΗ: ΔΕΝ περνάμε την destroy_value στο vector!
	// Αν την περάσουμε θα καλείται όταν κάνουμε swap 2 στοιχεία, το οποίο δεν το επιθυμούμε.
	pqueue->vector = vector_create(0, NULL);

	// Αν values != NULL, αρχικοποιούμε το σωρό.
	if (values != NULL)
		efficient_heapify(pqueue, values);

	return pqueue;
}

int pqueue_size(PriorityQueue pqueue) {
	return vector_size(pqueue->vector);
}

Pointer pqueue_max(PriorityQueue pqueue) {
	return node_value(pqueue,1);
}

PriorityQueueNode pqueue_insert(PriorityQueue pqueue, Pointer value) {

		PriorityQueueNode node = malloc(sizeof(*node));
		node->value = value;
		node->owner = pqueue;
		node->pos = pqueue_size(pqueue) + 1;

	// Προσθέτουμε την τιμή στο τέλος το σωρού
	vector_insert_last(pqueue->vector, node);

 	// Ολοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού εκτός από τον τελευταίο, που μπορεί να είναι
	// μεγαλύτερος από τον πατέρα του. Αρα μπορούμε να επαναφέρουμε την ιδιότητα του σωρού καλώντας
	// τη bubble_up γα τον τελευταίο κόμβο (του οποίου το 1-based id ισούται με το νέο μέγεθος του σωρού).
	bubble_up(pqueue, pqueue_size(pqueue));

	// TODO: υλοποίηση κόμβων και επιστροφή.

	return node;
}

void pqueue_remove_max(PriorityQueue pqueue) {
	int last_node = pqueue_size(pqueue);
	assert(last_node != 0);		// LCOV_EXCL_LINE

	// Destroy την τιμή που αφαιρείται
	if (pqueue->destroy_value != NULL)
		pqueue->destroy_value(pqueue_max(pqueue));

	// Αντικαθιστούμε τον πρώτο κόμβο με τον τελευταίο και αφαιρούμε τον τελευταίο
	node_swap(pqueue, 1, last_node);
	DestroyFunc old = vector_set_destroy_value(pqueue->vector,free);
	vector_remove_last(pqueue->vector);
	vector_set_destroy_value(pqueue->vector,old);

 	// Ολοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού εκτός από τη νέα ρίζα
 	// που μπορεί να είναι μικρότερη από κάποιο παιδί της. Αρα μπορούμε να
 	// επαναφέρουμε την ιδιότητα του σωρού καλώντας τη bubble_down για τη ρίζα.
	bubble_down(pqueue, 1);
}

DestroyFunc pqueue_set_destroy_value(PriorityQueue pqueue, DestroyFunc destroy_value) {
	DestroyFunc old = pqueue->destroy_value;
	pqueue->destroy_value = destroy_value;
	return old;
}

void pqueue_destroy(PriorityQueue pqueue) {
	// Αντί να κάνουμε εμείς destroy τα στοιχεία, είναι απλούστερο να
	// προσθέσουμε τη destroy_value στο vector ώστε να κληθεί κατά το vector_destroy.
	vector_set_destroy_value(pqueue->vector, (DestroyFunc)destroy_priority_queue_node);
	vector_destroy(pqueue->vector);

	free(pqueue);
}

// Νέες συναρτήσεις για την εργασία 2 //////////////////////////////////////////

Pointer pqueue_node_value(PriorityQueue set, PriorityQueueNode node) {
	return node->value;
}

void pqueue_remove_node(PriorityQueue pqueue, PriorityQueueNode node) {
	int i = node->pos;
	if(pqueue->destroy_value != NULL)
		pqueue->destroy_value(node_value(pqueue,i));
	node_swap(pqueue,i,vector_size(pqueue->vector)); //put the node at the end of the heap
	DestroyFunc old = vector_set_destroy_value(pqueue->vector,free); //set vector's destroy function
	vector_remove_last(pqueue->vector); //and remove the element
	vector_set_destroy_value(pqueue->vector,old); //reset vector's destroy value
	int parent = i/2;
	int left = 2*i;
	int right = 2*i+1;
	int size = pqueue_size(pqueue);
 
//check nodes relationship with its parent and childrean and bubble it up or down

	if (left<=size) {
		if(pqueue->compare(node_value(pqueue, left), node_value(pqueue, i)) > 0) {
			bubble_down(pqueue,i);
			return;	
		}
	}

	if (right<=size) {
		if(pqueue->compare(node_value(pqueue, right), node_value(pqueue, i)) > 0) {
			bubble_down(pqueue,i);
			return;	
		}
	}
	if (parent>=1) {
		if(pqueue->compare(node_value(pqueue, parent), node_value(pqueue, i)) < 0) {
			bubble_up(pqueue,i);
			return;	
		}
	}
}

void pqueue_update_order(PriorityQueue pqueue, PriorityQueueNode node) { //same as remove node without removing and swap
	int i = node->pos;
	int parent = i/2;
	int left = 2*i;
	int right = 2*i+1;
	int size = vector_size(pqueue->vector);

	if (left<=size) {
		if(pqueue->compare(node_value(pqueue, left), node_value(pqueue, i)) > 0) {
			bubble_down(pqueue,i);
			return;	
		}
	}

	if (right<=size) {
		if(pqueue->compare(node_value(pqueue, right), node_value(pqueue, i)) > 0) {
			bubble_down(pqueue,i);
			return;	
		}
	}
	if (parent>=1) {
		if(pqueue->compare(node_value(pqueue, parent), node_value(pqueue, i)) < 0) {
			bubble_up(pqueue,i);
			return;	
		}
	}
}