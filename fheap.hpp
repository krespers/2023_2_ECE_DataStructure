#ifndef __FHEAP_H_
#define __FHEAP_H_

#include <iostream>
#include <initializer_list>
#include <optional>
#include <vector>
#include <cmath>
#include <memory>
#include <queue>

template <typename T>
class PriorityQueue {
    public:
        virtual void insert(const T& item) = 0;
        virtual std::optional<T> extract_min() = 0;
        virtual bool is_empty() const = 0;
};

template <typename T>
class FibonacciNode {
    public:
        // constructors
        FibonacciNode()
            :key(std::nullopt), degree(0), child(nullptr), right(nullptr) {}
        FibonacciNode(const T& item)
            :key(item), degree(0), child(nullptr), right(nullptr) {}

        // destructor
        ~FibonacciNode() = default;

        T key;
        size_t degree;

        std::shared_ptr<FibonacciNode<T>> right;
        std::shared_ptr<FibonacciNode<T>> child;
        // NOTE: If you set left/parent pointer to shared_ptr, circular reference may cause!
        // So, left/parent pointer should be set to weak_ptr.
        std::weak_ptr<FibonacciNode<T>> left;
        std::weak_ptr<FibonacciNode<T>> parent;
};

template <typename T>
class FibonacciHeap : public PriorityQueue<T> {
    public:
        // Default Constructor
        FibonacciHeap()
            : min_node(nullptr), size_(0) {}

        // Constructor with Value
        FibonacciHeap(const T& item)
            : min_node(nullptr), size_(0) { insert(item); }

        // Disable copy constructor.
        FibonacciHeap(const FibonacciHeap<T> &);

        // Constructor with initializer_list
        // ex) FibonacciHeap<int> fheap = {1, 2, 3, 4};
        FibonacciHeap(std::initializer_list<T> list): min_node(nullptr), size_(0) {
            for(const T& item : list) {
                insert(item);
            }
        }

        // Destructor
        ~FibonacciHeap();

        // Insert item into the heap.
        void insert(const T& item) override;

        // Return raw pointer of the min node.
        // If the heap is empty, return nullptr.
        FibonacciNode<T>* get_min_node() { return min_node.get(); }

        // Return minimum value of the min node.
        // If the heap is empty, return std::nullopt.
        std::optional<T> get_min() const;

        // 1. Return minimum value of the min node
        // 2. Remove the node which contains minimum value from the heap.
        // If the heap is empty, return std::nullopt;
        std::optional<T> extract_min() override;

        // Return true if the heap is empty, false if not.
        bool is_empty() const override { return !size_; }

        // Return the number of nodes the heap contains.
        size_t size() const { return size_; }


    private:
        // Points to the node which has minimum value.
        std::shared_ptr<FibonacciNode<T>> min_node;

        // Value that represents how many nodes the heap contains.
        size_t size_;

        void insert(std::shared_ptr<FibonacciNode<T>>& node);

        // After extract, clean up the heap.
        void consolidate();

        // Combine two nodes.
        void merge(std::shared_ptr<FibonacciNode<T>>& x, std::shared_ptr<FibonacciNode<T>>& y);
};

// ****** This is the new function(deletenode) I created. *********
// because it was difficult to put it in the destructor function.
template <typename T>
void deletenode(std::shared_ptr<FibonacciNode<T>>& node) {
	if(!node) {
        return;
    }//there is no node to remove

    auto current = node; //put into a node.
    while (current) { //if node exists for delete
        auto child = current->child; // access child by using pointer
        current->child = nullptr; // and change the value to nullptr!!!!!!!!!!!

        if (child) { //and child node exists,
            deletenode(child);//recursive-call
        }
        auto temp = current;//also for Kill the right pointer
        current = current->right;//recursive call
        temp->right = nullptr;
        temp.reset();//reset the node...
    }
    node.reset(); //reset the node...


}

template <typename T>
FibonacciHeap<T>::~FibonacciHeap() {
    //TODO
    // CALL THE NEW FUNCTION - FOR DESTRUCTOR
    deletenode(min_node);
}


template <typename T>
std::optional<T> FibonacciHeap<T>::get_min() const {
    //TODO
	if(min_node) 
		return min_node->key;
	else
		return std::nullopt;
}

template <typename T>
void FibonacciHeap<T>::insert(const T& item) {
    // TODO
	std::shared_ptr<FibonacciNode<T>> node = std::make_shared<FibonacciNode<T>>(item);
	insert(node);
}

// insert 
template <typename T>
void FibonacciHeap<T>::insert(std::shared_ptr<FibonacciNode<T>>& node) {
    // TODO
	/*
	so as to prepare for an next operation
extracting the minimum value, Fibonacci Heap has to locate the next
minimum value, which is comparatively slow. In order to make the
insertion operation fast, Fibonacci Heap delays this preparation
operation.
*/
	if(!min_node) {//no min_node (=empty) => insert the first node (like a deque insert)
			min_node = node;
			min_node->right = min_node;
			min_node->left = min_node;
		} else {
			std::shared_ptr<FibonacciNode<T>> leftlock = min_node->left.lock();//left.lock()
			// To access a pointer stored in weak_ptr, ("left")
			//you must convert it to shared_ptr. So I used lock() method of weak_ptr instance 
			leftlock->right = node;
			min_node->left = node;
			node->left = leftlock;
			node->right = min_node;
			//The new node is placed to the left of min_node. (Insertion like a deque insert)
			if(node->key < min_node->key) {
				min_node = node;
			}
		}
	size_++;
}

template <typename T>
std::optional<T> FibonacciHeap<T>::extract_min() {
    // TODO
	if(!min_node)
		return std::nullopt;//there is no node, and return null
	//7 is min_node, 9 is leftlock, 8 is firstchild, 13 is lastchild (like a images)
	//leftlock(9)-> right = firstchild
	//firstchild(8) ->left = leftlock 
	//lastchild 
	//
	if(min_node->child) {
		std::shared_ptr<FibonacciNode<T>> firstchild = min_node->child;
		std::shared_ptr<FibonacciNode<T>> lastchild = firstchild->left.lock();
		std::shared_ptr<FibonacciNode<T>> leftlock = min_node->left.lock();
		
		leftlock->right = firstchild;
		firstchild->left = leftlock;

		lastchild->right = min_node;
		min_node->left = lastchild;
	}
	//extracting the minimum value itself
	T min_val = min_node->key;//access
	if(min_node->right == min_node) {
		min_node->right = nullptr;
		min_node = nullptr;
		//If I use reset() function, it makes memory leak. 
	} else {
		std::shared_ptr<FibonacciNode<T>> leftlock = min_node->left.lock();
		std::shared_ptr<FibonacciNode<T>> minnoderight = min_node->right;
		min_node = minnoderight;
		minnoderight->left = leftlock;
		leftlock->right = minnoderight;

		//consolitation!!!!!!!
		consolidate();
	}
	size_--;
	return min_val;
}

template <typename T>
void FibonacciHeap<T>::consolidate() {
    // TODO
	/*
	The purpose of consolidate operation is to make the heap with the
	trees that have a **distinct degree**. The degree represents the
	number of children. You should perform: For each node in the root
	list, if two nodes have the same degree, make one to the child of
	x. Don't forget that the trees are **min heap ordered**.
	*/

	float phi = (1 + sqrt(5)) / 2.0;//cf. phi is golden ratio.
	int max_degree = int(log(size_) / log(phi)) + 1;//maximum degree of trees.

	//degree=node's child number.
	//making a heap (have a different degree)
	//1. About each root node, if two node have a same degree, one node will be the other node's child 
	//2. trees are min heap ordered
	
	std::vector<std::shared_ptr<FibonacciNode<T>>> degreearray(max_degree, nullptr); //1.array initialize
	// The reason why a vector is the only option is that, simultaneously acting as an array, 
	//the elements of that array serve the role of pointers in the data structure.
	
	// TRAVERSE THE ROOT LIST.
	auto last_root = min_node->left.lock();
	for(auto currentnode = min_node, temp = currentnode->right; ; currentnode = temp, temp = currentnode->right) {
		//2.Starting from the current node, it traverses the root list, inserting nodes into array A based on their degrees.
		//loop for next node...
		auto x = currentnode;
		size_t deg = x->degree;

		//3.If duplicate degrees are encountered, the two nodes are merged, and the corresponding entry in array A is emptied.
		while(degreearray[deg] && deg < max_degree) {
			auto y = degreearray[deg];
			if(x->key > y->key) {
				swap(x, y);//swap
				}
			merge(x, y);//y = x's child, y->key is bigger than x->key
			degreearray[deg] = nullptr;//the degreearray[deg] have to be empty.
			deg++;
		}
		degreearray[deg] = x;
		if(currentnode == last_root) {
			break;
		}
	}
	min_node = nullptr;
	//4.After processing all root nodes, the nodes stored in array A are reconstructed into the root list.

	for (auto &currentnode : degreearray) {
        if (currentnode) {
        if (!min_node || currentnode->key < min_node->key) {//no min node, and currentnode->key is smaller than min_node->key
            min_node = currentnode;	//5.The minimum value is found, and min_node is "updated".
        	}
    	}
	}
}

template <typename T>
void FibonacciHeap<T>::merge(std::shared_ptr<FibonacciNode<T>>& x, std::shared_ptr<FibonacciNode<T>>& y) {
    // TODO
	//this function's purpose : About each root node, if two node have a same degree, one node will be the other node's child 
	//y = x's child
	std::shared_ptr<FibonacciNode<T>> leftlock = y->left.lock();
	std::shared_ptr<FibonacciNode<T>> yright = y->right;
	yright->left = leftlock;
	leftlock->right = yright;
	//element y is removed

	if(x->child) {//if there is child - x.
		std::shared_ptr<FibonacciNode<T>> lastchild = x->child->left.lock();
		x->child->left = y;
		lastchild->right = y;
		y->parent = x;

		y->left = lastchild;
		y->right = x->child;

	} else {
		x->child = y;
		y->parent = x; //y is x's chlid.
		
		y->left = y;
		y->right = y; //like a deque...
	}
	x->degree++;
}

#endif // __FHEAP_H_
