# Κάνοντας compile το <foo>_test.c με μια υλοποίηση <foo>.c του
# συγκεκριμένου τύπου, παράγουμε ένα tets για την υλοποίηση αυτή.

# Υλοποιήσεις μέσω HashTable: ADTMap
#
UsingHashTable_ADTMap_test_OBJS	= ADTMap_test.o $(MODULES)/UsingHashTable/ADTMap.o $(MODULES)/UsingLinkedList/ADTList.o

# ADTGraph
#
UsingAdjacencyLists_ADTGraph_test_OBJS = ADTGraph_test.o $(MODULES)/UsingHashTable/ADTMap.o $(MODULES)/UsingLinkedList/ADTList.o $(MODULES)/UsingAdjacencyLists/ADTGraph.o $(MODULES)/UsingHeap/ADTPriorityQueue.o $(MODULES)/UsingDynamicArray/ADTVector.o

# DiseaseMonitor
#
DiseaseMonitor_test_OBJS = DiseaseMonitor_test.o $(MODULES)/DiseaseMonitor/DiseaseMonitor.o $(MODULES)/UsingLinkedList/ADTList.o  $(MODULES)/UsingAVL/ADTSet.o $(MODULES)/UsingHashTable/ADTMap.o $(MODULES)/UsingHeap/ADTPriorityQueue.o $(MODULES)/UsingDynamicArray/ADTVector.o

# Ο βασικός κορμός του Makefile
include ../common.mk