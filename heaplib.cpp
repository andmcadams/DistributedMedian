#include <vector>

// // Swap element i and j or array arr. Do not check for bounds.
// template <size_t T>
void swap_elems(std::vector<int> *v, int i, int j) {
	int temp = (*v)[i];
	(*v)[i] = (*v)[j];
	(*v)[j] = temp;
}

// Puts the element at index n in it's correct spot.
// Recursively pushes it down the "heap". 
// *v is a pointer to the vector to order.
// n is the length of the array to sort.
// root is the index of the element to check for swapping
void heapify(std::vector<int> *v, int n, int root) {
	int largest = root;
	// Get the root's left and right child nodes.
	int l = 2*root + 1;
	int r = 2*root + 2;

	// If the left child node exists and is larger than the parent node, the largest index is set to l.
	if (l < n && (*v)[l] > (*v)[largest]) {
		largest = l;
	}

	// If the right child node exists and is larger than the parent node and the left child node, the largest index is r.
	if (r < n && (*v)[r] > (*v)[largest]) {
		largest = r;
	}

	// If the elements need to be swapped, swap them and continue looking for the right location of index largest (was the root)
	if (largest != root) {
		swap_elems(v, root, largest);
		heapify(v, n, largest);
	}
}

// Destructive heap-sort algorithm
void heap_sort(std::vector<int> *v) {
	int size = (*v).size();

	// Create a max heap
	// Iterate through all the nodes on level d-1 that have children nodes and all nodes on level d-2, d-3,..., 0.
		// (Heapify)
		// Swap them with their largest child, if such a child exists.
		// Push down the node to its correct location.
	// Once (Heapify) has been run on all such nodes, v will be in order of a max heap.
	for(int i = size/2-1; i >= 0; i--)
		heapify(v, size, i);

	// By construction of the max heap, index position 0 is the largest number.
	// Swap it with the last index involved in the max heap.
	// Then create a new max heap using Heapify with one less element. This keeps our sorted element "safe".
	// Note that since our max heap was swapped so that the only number that could be out of place is now the root.
	// This is index position 0, so run Heapify with size i on index 0. This will put v[0] in it's correct place,
	// resulting in a new max heap.
	for(int i = size-1; i >= 0; i--) {
		swap_elems(v, 0, i);

		heapify(v, i, 0);
	}
}