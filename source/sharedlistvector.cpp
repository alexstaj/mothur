/*
 *  sharedSharedListVector.cpp
 *  Mothur
 *
 *  Created by Sarah Westcott on 1/22/09.
 *  Copyright 2009 Schloss Lab UMASS Amherst. All rights reserved.
 *
 */

#include "sabundvector.hpp"
#include "rabundvector.hpp"
#include "ordervector.hpp"
#include "sharedlistvector.h"
#include "sharedordervector.h"
#include "sharedutilities.h"

/***********************************************************************/

SharedListVector::SharedListVector() : DataVector(), maxRank(0), numBins(0), numSeqs(0){ groupmap = NULL; }

/***********************************************************************/

SharedListVector::SharedListVector(int n):	DataVector(), data(n, "") , maxRank(0), numBins(0), numSeqs(0){ groupmap = NULL; }

/***********************************************************************/
SharedListVector::SharedListVector(ifstream& f) : DataVector(), maxRank(0), numBins(0), numSeqs(0) {
	try {
		//set up groupmap for later.
		groupmap = new GroupMap(m->getGroupFile());
		groupmap->readMap(); 

		int hold;
		string inputData;
		f >> label >> hold;
	
		data.assign(hold, "");
		
		for(int i=0;i<hold;i++){
			f >> inputData;
			set(i, inputData);
		}
		
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "SharedListVector");
		exit(1);
	}
}

/***********************************************************************/
void SharedListVector::set(int binNumber, string seqNames){
	try {
		int nNames_old = m->getNumNames(data[binNumber]);
		data[binNumber] = seqNames;
		int nNames_new = m->getNumNames(seqNames);
	
		if(nNames_old == 0)			{	numBins++;				}
		if(nNames_new == 0)			{	numBins--;				}
		if(nNames_new > maxRank)	{	maxRank = nNames_new;	}
	
		numSeqs += (nNames_new - nNames_old);
		
			 
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "set");
		exit(1);
	}
}

/***********************************************************************/

string SharedListVector::get(int index){
	return data[index];
}

/***********************************************************************/

void SharedListVector::push_back(string seqNames){
	try {
		data.push_back(seqNames);
		int nNames = m->getNumNames(seqNames);
	
		numBins++;
	
		if(nNames > maxRank)	{	maxRank = nNames;	}
	
		numSeqs += nNames;
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "push_back");
		exit(1);
	}
}

/***********************************************************************/

void SharedListVector::resize(int size){
	data.resize(size);		
}

/***********************************************************************/

int SharedListVector::size(){
	return data.size();
}
/***********************************************************************/

void SharedListVector::clear(){
	numBins = 0;
	maxRank = 0;
	numSeqs = 0;
	return data.clear();
	
}

/***********************************************************************/

void SharedListVector::print(ostream& output){
	try {
		output << label << '\t' << numBins << '\t';
	
		for(int i=0;i<data.size();i++){
			if(data[i] != ""){
				output << data[i] << '\t';
			}
		}
		output << endl;
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "print");
		exit(1);
	}
}


/***********************************************************************/

RAbundVector SharedListVector::getRAbundVector(){
	try {
		RAbundVector rav;
	
		for(int i=0;i<data.size();i++){
			int binSize = m->getNumNames(data[i]);
			rav.push_back(binSize);
		}
	
	//  This was here before to output data in a nice format, but it screws up the name mapping steps
	//	sort(rav.rbegin(), rav.rend());
	//	
	//	for(int i=data.size()-1;i>=0;i--){
	//		if(rav.get(i) == 0){	rav.pop_back();	}
	//		else{
	//			break;
	//		}
	//	}
		rav.setLabel(label);
	
		return rav;
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "getRAbundVector");
		exit(1);
	}
}

/***********************************************************************/

SAbundVector SharedListVector::getSAbundVector(){
	try {
		SAbundVector sav(maxRank+1);
	
		for(int i=0;i<data.size();i++){
			int binSize = m->getNumNames(data[i]);	
			sav.set(binSize, sav.get(binSize) + 1);	
		}
		sav.set(0, 0);
		sav.setLabel(label);
	
		return sav;
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "getSAbundVector");
		exit(1);
	}
}

/***********************************************************************/
SharedOrderVector* SharedListVector::getSharedOrderVector(){
	try {
		string groupName, names, name;
	
		SharedOrderVector* order = new SharedOrderVector();
		order->setLabel(label);
	
		for(int i=0;i<numBins;i++){
			int binSize = m->getNumNames(get(i));	//find number of individual in given bin	
			names = get(i);
			while (names.find_first_of(',') != -1) { 
				name = names.substr(0,names.find_first_of(','));
				names = names.substr(names.find_first_of(',')+1, names.length());
				groupName = groupmap->getGroup(name);
				
				if(groupName == "not found") {	m->mothurOut("Error: Sequence '" + name + "' was not found in the group file, please correct."); m->mothurOutEndLine();  exit(1); }
				
				order->push_back(i, binSize, groupName);  //i represents what bin you are in
			}
			//get last name
			groupName = groupmap->getGroup(names);
			if(groupName == "not found") {	m->mothurOut("Error: Sequence '" + names + "' was not found in the group file, please correct."); m->mothurOutEndLine();  exit(1); }
			order->push_back(i, binSize, groupName);
		}

		random_shuffle(order->begin(), order->end());
		order->updateStats();
		
		return order;
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "getSharedOrderVector");
		exit(1);
	}
}
/***********************************************************************/
SharedRAbundVector SharedListVector::getSharedRAbundVector(string groupName) {
	try {
		SharedRAbundVector rav(data.size());
		string group, names, name;
		
		for(int i=0;i<numBins;i++){
			names = get(i);  
			while (names.find_first_of(',') != -1) { 
				name = names.substr(0,names.find_first_of(','));
				names = names.substr(names.find_first_of(',')+1, names.length());
				group = groupmap->getGroup(name);
				if(group == "not found") {	m->mothurOut("Error: Sequence '" + name + "' was not found in the group file, please correct."); m->mothurOutEndLine();  exit(1); }
				if (group == groupName) { //this name is in the group you want the vector for.
					rav.set(i, rav.getAbundance(i) + 1, group);  //i represents what bin you are in
				}
			}
			
			//get last name
			groupName = groupmap->getGroup(names);
			if(groupName == "not found") {	m->mothurOut("Error: Sequence '" + names + "' was not found in the group file, please correct."); m->mothurOutEndLine();  exit(1); }
			if (group == groupName) { //this name is in the group you want the vector for.
					rav.set(i, rav.getAbundance(i) + 1, group);  //i represents what bin you are in
			}
		}
		
		rav.setLabel(label);
		rav.setGroup(groupName);

		return rav;
		
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "getSharedRAbundVector");
		exit(1);
	}
}
/***********************************************************************/
vector<SharedRAbundVector*> SharedListVector::getSharedRAbundVector() {
	try {
		SharedUtil* util;
		util = new SharedUtil();
		vector<SharedRAbundVector*> lookup;  //contains just the groups the user selected
		map<string, SharedRAbundVector*> finder;  //contains all groups in groupmap
		string group, names, name;
		
		vector<string> Groups = m->getGroups();
		vector<string> allGroups = groupmap->getNamesOfGroups();
		util->setGroups(Groups, allGroups);
		m->setGroups(Groups);
		delete util;

		for (int i = 0; i < allGroups.size(); i++) {
			SharedRAbundVector* temp = new SharedRAbundVector(data.size());
			finder[allGroups[i]] = temp;
			finder[allGroups[i]]->setLabel(label);
			finder[allGroups[i]]->setGroup(allGroups[i]);
			if (m->inUsersGroups(allGroups[i], m->getGroups())) {  //if this group is in user groups
				lookup.push_back(finder[allGroups[i]]);
			}
		}
	
		//fill vectors
		for(int i=0;i<numBins;i++){
			names = get(i);  
			int nameLength = names.size();
			string seqName = "";
			
			for(int j=0;j<nameLength;j++){
				if(names[j] == ','){
					group = groupmap->getGroup(seqName);
					if(group == "not found") {	m->mothurOut("Error: Sequence '" + seqName + "' was not found in the group file, please correct."); m->mothurOutEndLine();  exit(1); }
					finder[group]->set(i, finder[group]->getAbundance(i) + 1, group);  //i represents what bin you are in
					
					seqName = "";
				}
				else{
					seqName += names[j];
				}
			}
			group = groupmap->getGroup(seqName);
			if(group == "not found") {	m->mothurOut("Error: Sequence '" + seqName + "' was not found in the group file, please correct."); m->mothurOutEndLine();  exit(1); }
			finder[group]->set(i, finder[group]->getAbundance(i) + 1, group);  //i represents what bin you are in
			
			
			
//			while (names.find_first_of(',') != -1) { 
//				name = names.substr(0,names.find_first_of(','));
//				names = names.substr(names.find_first_of(',')+1, names.length());
//				group = groupmap->getGroup(name);
//				if(group == "not found") {	m->mothurOut("Error: Sequence '" + name + "' was not found in the group file, please correct."); m->mothurOutEndLine();  exit(1); }
//				finder[group]->set(i, finder[group]->getAbundance(i) + 1, group);  //i represents what bin you are in
//			}
			
			//get last name
//			group = groupmap->getGroup(names);
//			if(group == "not found") {	m->mothurOut("Error: Sequence '" + names + "' was not found in the group file, please correct."); m->mothurOutEndLine();  exit(1); }
//			finder[group]->set(i, finder[group]->getAbundance(i) + 1, group);  //i represents what bin you are in
			
		}

		return lookup;
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "getSharedRAbundVector");
		exit(1);
	}
}

/***********************************************************************/
SharedSAbundVector SharedListVector::getSharedSAbundVector(string groupName) {
	try { 
		SharedSAbundVector sav;
		SharedRAbundVector rav;
		
		rav = this->getSharedRAbundVector(groupName);
		sav = rav.getSharedSAbundVector();
		
		return sav;
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "getSharedSAbundVector");
		exit(1);
	}
}
/***********************************************************************/

OrderVector SharedListVector::getOrderVector(map<string,int>* orderMap = NULL){
	
	try {
		if(orderMap == NULL){
			OrderVector ov;
		
			for(int i=0;i<data.size();i++){
				int binSize = m->getNumNames(data[i]);		
				for(int j=0;j<binSize;j++){
					ov.push_back(i);
				}
			}
			random_shuffle(ov.begin(), ov.end());
			ov.setLabel(label);
			ov.getNumBins();
		
			return ov;
		
		}
		else{
			OrderVector ov(numSeqs);
		
			for(int i=0;i<data.size();i++){
				string listOTU = data[i];
				int length = listOTU.size();
				
				string seqName="";
			
				for(int j=0;j<length;j++){
				
					if(listOTU[j] != ','){
						seqName += listOTU[j];
					}
					else{
						if(orderMap->count(seqName) == 0){
							m->mothurOut(seqName + " not found, check *.names file\n");
							exit(1);
						}
					
						ov.set((*orderMap)[seqName], i);
						seqName = "";
					}						
				}
			
				if(orderMap->count(seqName) == 0){
					m->mothurOut(seqName + " not found, check *.names file\n");
					exit(1);
				}
				ov.set((*orderMap)[seqName], i);	
			}
		
			ov.setLabel(label);
			ov.getNumBins();
		
			return ov;		
		}
	}
	catch(exception& e) {
		m->errorOut(e, "SharedListVector", "getOrderVector");
		exit(1);
	}
}

/***********************************************************************/
