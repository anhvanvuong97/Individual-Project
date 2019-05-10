#pragma once

#include <map>      
#include <string>   
#include <iostream> 
#include <math.h> 	
#include <sstream> 

using namespace cv;
using namespace std;


template<typename T>
float calculate_certainty(vector<T> &vec, int certainty_spread)
{
	
	//Extract BNN probability values
	vector<int> vec2;
	for(auto const &elem : vec)
	{
		vec2.push_back(elem);
	}
	
	sort(vec2.begin(), vec2.end());
	
	int sum_of_elems = 0;
	for (auto& n : vec2)
	{
		sum_of_elems += n;
	}
	
	int sum_of_threelargestelems = vec2[vec2.size() -1] + vec2[vec2.size() -2] + vec2[vec2.size() -3] + vec2[vec2.size() -4] + vec2[vec2.size() -5];
	int max_elem = vec2[vec2.size() -1];
	int min_elem = vec2[0];
	int gap_large = vec2[vec2.size() -1] - vec2[vec2.size() -10];
	int gap_small = vec2[vec2.size() -1] - vec2[vec2.size() -1];
	//int max_elem = *max_element(vec2.begin(), vec2.end());
	//float certainty;
	
	//certainty = max_elem/sum_of_elems;
	return (float)gap_small/(float)gap_large;
	
}