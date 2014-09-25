#include "DBMgr.h"
#include <iostream>
#include <stdio.h>

static DBMgr dbMgr;

DBMgr* DBMgr::getInstance() 
{
	static DBMgr *pInstance = NULL;
	if(pInstance == NULL)
	{
		pInstance = &dbMgr;
	}
	return pInstance;
}

void DBMgr::writeItem(FILE *fp, std::string module, std::string item)
{
	char *p = (char*)item.c_str();
	std::string id(item);
	auto idPosBegin = item.find_first_of(":");
	auto idPosEnd = item.find_first_of(",");
	if(!(idPosBegin <= item.length() && idPosEnd <= item.length()))
	{
		printf("can't find id\n");
		exit(-1);
	}
	id = id.substr(idPosBegin+1, idPosEnd-idPosBegin-1);
	id = std::string("[") + id + std::string("]");
	/*
	if(*(id.c_str()) == "\"")
	{
		id = std::string("[
	}
	else
	{

	}
	*/
	while(1)
	{
		auto pos = item.find_first_of("\\");
		if(pos <= item.length())
		{
			item.erase(pos, 1);
		}
		else
		{
			break;
		}
	}
	while(1)
	{
		auto pos = item.find_first_of(":");
		if(pos <= item.length())
		{
			*(p+pos) = '=';
			item.erase(pos-1, 1);
			auto tmp = item.substr(0, pos);
			pos = tmp.find_last_of("\"");
			item.erase(pos, 1);
		}
		else
		{
			break;
		}
	}
	m_itemTables.clear();
	while(1)
	{
		//auto tabPos = item.find_first_of("_tab=");
		auto tabPos = strstr(p, "_tab");
		if(tabPos)
		{
			
			auto len_ = tabPos - p;
			auto tab1 = item.substr(0, len_);
			auto begin = tab1.find_last_of(",")+1;
			auto key = item.substr(begin-1);
			key.erase(0, 1);
			key.erase(key.find_first_of("_tab"));
			auto tab2 = item.substr(len_+5);
			//auto tabLen = tab2.find_first_of("}")+1;
			auto ss = item.substr(begin);
			ss.erase(strstr(ss.c_str(), "_tab")-ss.c_str(), 4);
			ss.erase(ss.find_first_of("\""), 1);
			ss.erase(ss.find_first_of("}")+1);
			item.erase(begin, item.find_first_of("}")-begin+3);
			
			//std::map<std::string, std::string> m;
			//m[key] = ss;
			//m_itemTables.push_back(m);
			m_itemTables.push_back(ss);
		}
		else
		{
			break;
		}
	}

	auto idBegin = item.find_first_of("\"");
	
	//auto idEnd = id.find_first_of("\"");
	//id = id.substr(0, idEnd);
	auto moduleTab = std::string("\n") + module + id + "=" + item + "\n";
	fwrite(moduleTab.c_str(), moduleTab.length(), 1, fp);
	for(auto it = std::begin(m_itemTables); it != std::end(m_itemTables); ++it)
	{
		auto subTab = module + id + "." + *it + "\n";
		fwrite(subTab.c_str(), subTab.length(), 1, fp);
	}
}

void DBMgr::parseFileToItems(std::string file)
{
	FILE *fp = fopen(file.c_str(), "rb");
	std::cout << file << std::endl;
	if(!fp)
	{
		printf("open file failed: %s!\n", file.c_str());
		return;
	}

	fseek(fp,0,SEEK_SET);
	fseek(fp,0,SEEK_END);

	int len = ftell(fp);

	char *buf = (char*)malloc(len+1);
	if(!buf)
	{
		printf("malloc 1m failed: %m!\n");
		return;
	}

	memset(buf, 0, len+1);
	fseek(fp,0,SEEK_SET);
	fread(buf, 1, len, fp);
	fclose(fp);
	buf[len] = 0;
	std::string s((const char*)buf);

	auto fileNameBegin = file.find_first_of("_");
	if(fileNameBegin > file.length())
	{
		fileNameBegin = 0;
	}
	else
	{
		fileNameBegin += 1;
	}
	auto fileNameEnd   = file.find_first_of(".");
	auto file2 = file.substr(fileNameBegin, fileNameEnd-fileNameBegin);
	
	FILE *fp2 = fopen((file2+std::string(".lua")).c_str(), "wb+");
	if(!fp2)
	{
		printf("open file2 failed: %s!\n", file.c_str());
		return;
	}
	auto module = file2;
	auto wmodule = std::string("local ") + file2 + std::string("={}\n");
	fwrite(wmodule.c_str(), wmodule.length(), 1, fp2);

	const char *head = "{\"id\":";
	const unsigned int headLen = strlen(head);
	char *begin = 0;
	char *headEnd = 0;
	char *end = 0;
	char *sBegin = (char *)s.c_str();
	unsigned int currLen = 0;

	char *cp = (char*)s.c_str();
	
	int idx = 1;
	while(1)
	{
		begin = strstr(cp, head);
		auto pos2 = s.find(std::string(head));
		if(!begin)
		{
			break;
		}
		headEnd = begin + headLen;
		end = strstr(headEnd, head);
		if(end)
		{
			 end -= 1;
			 m_itemCache.push_back(s.substr(begin-sBegin, end-begin));
			 auto ss = s.substr(begin-sBegin, end-begin);
			 writeItem(fp2, module, ss);
			 cp += (end-begin+1);
		}
		else
		{
			auto tmps = s.substr(begin-sBegin);
			auto pos = tmps.find_last_of("}");
			m_itemCache.push_back(tmps.substr(0, pos+1));
			auto ss = tmps.substr(0, pos+1);
			writeItem(fp2, module, ss);
			break;
		}
		idx++;
	}

	wmodule = "\nreturn " + module;
	fwrite(wmodule.c_str(), wmodule.length(), 1, fp2);

	fclose(fp2);
}

void DBMgr::addFile(std::string file)
{
	//auto path = FileUtils::getInstance()->fullPathForFilename(file);
	auto pos = file.find_last_of("/");
	if(pos < file.length())
	{
		file = file.substr(pos+1);
	}
	pos = file.find_last_of("\\");
	if(pos < file.length())
	{
		file = file.substr(pos+1);
	}
	m_fileCache.push_back(file);
}

void DBMgr::asyncLoadAll()
{
	for (auto it = std::begin(m_fileCache); it != std::end(m_fileCache); ++it)
	{
		parseFileToItems(*it);
		
		int i = 1;
		std::cout << "------ file: " << *it << std::endl;
		for(auto itemIt = std::begin(m_itemCache); itemIt != std::end(m_itemCache); ++itemIt, i++)
		{
			std::cout << i << ": " << *itemIt << std::endl;
		}
		
	}
	m_allLoaded = true;
}

void DBMgr::loadAll()
{
	m_allLoaded = false;
	//auto t = new std::thread(&DBMgr::asyncLoadAll, this);
	//t->detach();
	asyncLoadAll();
}

std::string  DBMgr::getAndRemoveOneItem()
{
	if(!m_itemCache.empty())
	{
		std::string s = m_itemCache.back();
		m_itemCache.pop_back();
		return s;
	}
	return "";
}
