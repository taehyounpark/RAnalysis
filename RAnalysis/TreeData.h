#pragma once

#include <string>
#include <vector>
#include <memory>

#include <ROOT/RVec.hxx>

#include "TROOT.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"

#include "ana/input.h"
#include "ana/reader.h"

class TreeData : public ana::input::dataset<TreeData>
{

public:
	class Reader;

	template <typename T>
	class Branch;

public:
	TreeData(const std::string& treeName, std::initializer_list<std::string> allFiles);
	TreeData(const std::string& treeName, const std::vector<std::string>& allFiles);
	~TreeData() = default;

	ana::input::partition allocate();
	std::shared_ptr<Reader> open(const ana::input::range& part) const;

protected:
	std::string m_treeName;
	std::vector<std::string> m_allFiles;
	std::vector<std::string> m_goodFiles;

};

class TreeData::Reader : public ana::input::reader<Reader>
{
public:
	Reader(const ana::input::range& part, std::unique_ptr<TTree> tree);
	~Reader() = default;

	template <typename U>
	std::shared_ptr<Branch<U>> read(const std::string& branchName) const;

	virtual void begin() override;
	virtual bool next() override;

protected:
	std::unique_ptr<TTree>       m_tree; 
	std::unique_ptr<TTreeReader> m_treeReader;  

};

template <typename T>
class TreeData::Branch : public ana::column::reader<T>
{

public:
	Branch(const std::string& branchName, TTreeReader& treeReader) :
		ana::column::reader<T>(branchName),
		m_branchName(branchName),
		m_treeReader(&treeReader)
	{}
	~Branch() = default;

	virtual void initialize() override
	{
		m_treeReaderValue = std::make_unique<TTreeReaderValue<T>>(*m_treeReader,this->m_branchName.c_str());
	}

	virtual void execute() override
	{
		this->read(**m_treeReaderValue);
	}

protected:
  std::string m_branchName;
	TTreeReader* m_treeReader;
  std::unique_ptr<TTreeReaderValue<T>> m_treeReaderValue;  

};

template <typename T>
class TreeData::Branch<ROOT::RVec<T>> : public ana::column::reader<ROOT::RVec<T>>
{

public:
	Branch(const std::string& branchName, TTreeReader& treeReader) :
		ana::column::reader<ROOT::RVec<T>>(branchName),
		m_branchName(branchName),
		m_treeReader(&treeReader)
	{}
	~Branch() = default;

	virtual void initialize() override
	{
		m_treeReaderArray = std::make_unique<TTreeReaderArray<T>>(*m_treeReader,this->m_branchName.c_str());
	}

	virtual void execute() override
	{
    if (auto arraySize = m_treeReaderArray->GetSize()) {
      ROOT::RVec<T> readArray(&m_treeReaderArray->At(0), arraySize);
      std::swap(m_readArray,readArray);
    } else {
      ROOT::RVec<T> emptyVector{};
      std::swap(m_readArray,emptyVector);
    }
		this->read(m_readArray);
	}

protected:
  std::string m_branchName;
	TTreeReader* m_treeReader;
  std::unique_ptr<TTreeReaderArray<T>> m_treeReaderArray;
	mutable ROOT::RVec<T> m_readArray;

};

template <>
class TreeData::Branch<ROOT::RVec<bool>> : public ana::column::reader<ROOT::RVec<bool>>
{

public:
	Branch(const std::string& branchName, TTreeReader& treeReader) :
		ana::column::reader<ROOT::RVec<bool>>(branchName),
		m_branchName(branchName),
		m_treeReader(&treeReader)
	{}
	~Branch() = default;

	virtual void initialize() override
	{
		m_treeReaderArray = std::make_unique<TTreeReaderArray<bool>>(*m_treeReader,this->m_branchName.c_str());
	}

	virtual void execute() override
	{
    if (m_treeReaderArray->GetSize()) {
      ROOT::RVec<bool> readArray(m_treeReaderArray->begin(), m_treeReaderArray->end());
      std::swap(m_readArray,readArray);
    } else {
      ROOT::RVec<bool> emptyVector{};
      std::swap(m_readArray,emptyVector);
    }
		this->read(m_readArray);
	}

protected:
  std::string m_branchName;
	TTreeReader* m_treeReader;
  std::unique_ptr<TTreeReaderArray<bool>> m_treeReaderArray;
	mutable ROOT::RVec<bool> m_readArray;

};


template <typename U>
std::shared_ptr<TreeData::Branch<U>> TreeData::Reader::read(const std::string& branchName) const
{
	return std::make_shared<Branch<U>>(branchName,*m_treeReader);
}