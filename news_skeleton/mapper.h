#ifndef _MAPPER_H_
#define _MAPPER_H_

#include <map>
#include <string>

template<typename T>
class StringMapper
{
public:
	StringMapper() 
	:root_ptr(NULL), prev_add(NULL), iterator(NULL), m_map_size(0)
	{  }
	~StringMapper() 
	{										//follow link list from first insert item to the last
		if (root_ptr == NULL) return;		//delete dynamic memory allocated
		bst_key_node *node;
		while (root_ptr != NULL)
		{
			node		= root_ptr;
			root_ptr	= root_ptr->nextItem;
			delete node->mapped_data;
			delete node;
		}
		m_map_size		= 0;
		iterator		= NULL;
	}
	
	void insert(std::string from, const T& to)
	{
		if (root_ptr == NULL)
		{
			bst_key_node* newItem = CreateBSTNode(from, to);
			root_ptr	= newItem;
			prev_add	= newItem;
			m_map_size++;
			return;
		}
		
		bst_key_node* ptr = root_ptr;
		while (ptr != NULL)
		{
			if (from == ptr->data) 
			{
				*ptr->mapped_data	= to;	//Do not change the string key, but change the value of mapped variable
				return;						//This extra implementation is in use for NewsAgg class
			}
			if (from < ptr->data)			// String value to be inserted to map is less than current string
			{
				if (ptr->left == NULL)		// Found the current location to insert 
				{
					bst_key_node* newItem	= CreateBSTNode(from, to);
					ptr->left				= newItem;
					prev_add->nextItem		= newItem;		//previous added node to current add node
					prev_add				= newItem;		//set as previous node
					break;
				}
				else		
					ptr = ptr->left;		//set pointer to left child tree
			}
			else if (from > ptr->data)		// String value to be inserted to map is greater than current string
			{
				if (ptr->right == NULL)
				{
					bst_key_node* newItem	= CreateBSTNode(from, to);
					ptr->right				= newItem;
					prev_add->nextItem		= newItem;		//previous added node to current add node
					prev_add				= newItem;		//set as previous node
					break;
				}
				else
					ptr	= ptr->right;		//set pointer to right child tree
			}	
		}
		m_map_size++;
		iterator = NULL;		//size of map changed, invalidate iterator pointer;	
	}
	
    bool find(std::string from, T& to) const
	{
		bst_key_node* temp_ptr = root_ptr;		//DO not use root_ptr, use a temp ptr
		
		while (temp_ptr != NULL)
		{
			if (from == temp_ptr->data)			
			{
				to = *(temp_ptr->mapped_data);	//Found the string in BST, return mapped T variable
				return true;
			}
			if (from < temp_ptr->data)
				temp_ptr	= temp_ptr->left;	//set pointer to left child tree
			else
				temp_ptr	= temp_ptr->right;	//set pointer to right child tree
		}
		return false;
	}
    bool getFirstPair(std::string& from, T& to)
	{
		iterator	= root_ptr;					//Start from root pointer
		return getNextPair(from, to);
	}
    bool getNextPair(std::string& from, T& to)
	{
		if (iterator == NULL) return false;
		from	= iterator->data;			
		to		= *(iterator->mapped_data);
		iterator= iterator->nextItem;			//Follow linked list created in the order node insertion
		return true;
	}
	
	int size() const { return m_map_size; }
	
private:
	
	struct bst_key_node 
	{	
		std::string data;					//String Mapper always contains a string
		bst_key_node *left;					//node pointing to left child tree
		bst_key_node *right;				//node pointing to right child tree
		bst_key_node *nextItem;				//node pointing to the next node that is inserted
		T* mapped_data;						//Mapped variable T
	};
	
	bst_key_node* CreateBSTNode(std::string val, T mapped_val) const
	{
		bst_key_node* newnode = new bst_key_node;		//Allocate memory for new node
		if( newnode == NULL) 
			return NULL;	
		newnode->data			= val;					//Copy value of val to string
		newnode->mapped_data	= new T;				//Allocate memory for Mapped Variable T
		*newnode->mapped_data	= mapped_val;			//Copy value of T to new node
		newnode->right = newnode->left	= NULL;
		newnode->nextItem	= NULL;
		return newnode;
	}
	
	bst_key_node*	root_ptr;			//head pointer of the BST
	bst_key_node*	prev_add;			//points to the previously inserted node
	bst_key_node*	iterator;			//iterator to iterate through all nodes
	int				m_map_size;			//size of the BST
};

#endif // _MAPPER_H_
