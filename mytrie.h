/*
 * mytrie.h
 *
 *  Created on: 29 Aug 2016
 *      Author: thachln
 */

#ifndef MYTRIE_H_
#define MYTRIE_H_

#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <map>



#define ALPHABET_SIZE (26)






class Trie {
private:
	struct TNode
	{
		struct TNode *children[ALPHABET_SIZE];
		char key;
		std::string pattern;
		bool isLeaf;
		double val;
		int size;
		int id;
	} ;
	struct TNode *root;
	unsigned int next_id = 1;
	unsigned int trie_size = 0;

	void print(struct TNode *node){

		std::cout << node->key << ":" << node->pattern << ":" << node->val;

		if(node->size > 0){
			std::cout << " [ ";
			for (int i = 0; i < node->size;i++){
				print(node->children[i]);
			}
			std::cout << " ] ";
		}
	}

	void deleteBranch(struct TNode *node){
		if(node->size > 0){
			for (int i = 0; i < node->size;i++){
				deleteBranch(node->children[i]);
			}
		}
		delete node;
	}

	// calculate the distance between two keys
	double distance(char a, char b){
		return 1.0*std::abs(a - b);
	}

	double compute_xi(double d, double max_distance){

		if (d <= max_distance){
//			return 1.0;
			return 1.0 - d/(max_distance + 1);
		}
		return 0.0;
	}

	bool isExitNode(struct TNode *node){
		return (node->id > 0);
	}

public:
	Trie(){
		root = new struct TNode();
		root->key = ' ';
		root->val = 0.0;
		root->size = 0;
		root->pattern = "";
		root->id = -1;

	}
	~Trie(){
		deleteBranch(root);
	}

	int getSize(){
		return trie_size;
	}

	// insert new pattern to the trie
	bool insert(std::string pattern, double score){
		struct TNode *currentNode = root;


		for (int i = 0; i < pattern.length();i++){
			bool not_found = true;
			for (int j = 0; j < currentNode->size;j++){
				if (currentNode->children[j]->key == pattern[i]){
					currentNode = currentNode->children[j];
					not_found = false;
					break;
				}
			}
			if (not_found){
				if (currentNode->size >= ALPHABET_SIZE){
					return false;
				} else {
					struct TNode *newNode = new struct TNode();
					newNode->key = pattern[i];
					newNode->isLeaf = false;
					newNode->val = 0.0;
					newNode->size = 0;
					newNode->pattern = "";
					newNode->id = -1;

					currentNode->children[currentNode->size] = newNode;
					currentNode->size++;

					currentNode = newNode;
					trie_size++;
				}

			}
		}

		currentNode->val = score;
		currentNode->pattern = pattern;
		currentNode->id = next_id;
		next_id++;
		return true;
	}

	// for testing: print the whole trie
	void printTrie(){
		std::cout << std::endl;
		print(root);
		std::cout << std::endl;
	}

	void printPatterns(){
		std::cout << std::endl;
		std::vector<struct TNode *> expanding_nodes;
		expanding_nodes.push_back(root);
		struct TNode * cur_node;
		while(!expanding_nodes.empty()){
			cur_node = expanding_nodes.back();
			expanding_nodes.pop_back();
			if (isExitNode(cur_node)){
				std::cout << cur_node->pattern << " " << cur_node->id << " " << cur_node->val << std::endl;
			}
			for (int i = 0; i < cur_node->size;i++){
				expanding_nodes.push_back(cur_node->children[i]);
			}

		}
		std::cout << std::endl;
	}

	bool load_seql_trie (const char *file){
		std::ifstream infile(file);
		std::string s;
		std::string v;
		double val;
		// skip the first line
		if (!(infile >> s)){
			std::cout << "Wrong input" << std::endl;
			return false;
		}
		while(infile >> v >> s){
			insert(s,std::stod(v));
		}
		return true;
	}

	double naive_search(const char *line, double max_distance, std::vector<int> &output){


		unsigned int len = strlen (line);

		double r = 0.0;

		std::vector<struct TNode *> expanding_nodes;
		std::vector<int> next_positions;
		std::vector<double> distances;

		std::map<struct TNode *,double> results;
		//int st_pos = 0;

		for (unsigned int st_pos = 0; st_pos < len; st_pos++){
			//while (line[st_pos] != '\0' && line[st_pos] !=  NULL){
			expanding_nodes.push_back(root);
			next_positions.push_back(st_pos);
			distances.push_back(0.0);

			while(!expanding_nodes.empty()){
				// fetch unvisited node
				unsigned int cur_pos = next_positions.back(); // advance to the next position
				double cur_distance = distances.back();
				struct TNode * cur_node = expanding_nodes.back();
				next_positions.pop_back();
				distances.pop_back();
				expanding_nodes.pop_back();

				//if (line[st_pos] != '\0' && line[st_pos] !=  NULL){
				if (cur_pos < len && !isspace(line[cur_pos])){
					char next_char = line[cur_pos];
					for (int i = 0; i < cur_node->size;i++){
						double new_dist = distance(cur_node->children[i]->key,next_char);
						if (cur_distance + new_dist <= max_distance){
							// insert unvisited node
							expanding_nodes.push_back(cur_node->children[i]);
							distances.push_back(cur_distance + new_dist);
							next_positions.push_back(cur_pos + 1);

							if (isExitNode(cur_node->children[i])){ // found a pattern
								// update results
								if (results.count(cur_node->children[i]) == 0 || results[cur_node->children[i]] > cur_distance + new_dist){
									results[cur_node->children[i]] = cur_distance + new_dist;
									//std::cout << cur_node->children[i]->id << ":" << cur_distance + new_dist << ":"<< cur_node->children[i]->val << std::endl;
								}

								//std::cout << cur_node->children[i]->pattern << ":" << cur_distance + new_dist << ":"<< cur_node->children[i]->val << std::endl;
							}
						}

					}
				}

			}
		}

		for(auto const &ent1 : results) {
			// ent1.first is the first key
			//std::cout << ent1.first->pattern << ":" << ent1.first->val << std::endl;

			output.push_back(ent1.first->id);
			r += ent1.first->val * compute_xi(ent1.second,max_distance);
		}

		return r;


	}



};

#endif /* MYTRIE_H_ */
