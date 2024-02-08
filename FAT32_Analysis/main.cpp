#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "byte_buffer.hpp"

using namespace std;

// trim from left 
inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
{
	s.erase(0, s.find_first_not_of(t));
	return s;
}
// trim from right 
inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
{
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

// trim from left & right 
inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v")
{
	return ltrim(rtrim(s, t), t);
}

class Superblock{

  protected:
    uint16_t bytesPecSec;
    uint8_t sectorPerCluster;
    uint16_t reservedSec;
    uint8_t numberOfFAT;
    uint32_t sectorOfFat;
    uint32_t rootDirCluster;
    int cluster_size;
    int fat_area_addr;
    int fat_area_size;
    int data_area_start;

  public:
    Superblock(char *buffer, int offset, int count);
    auto getBytesPerSec() const { return bytesPecSec; }
    auto getReservedSec() const { return reservedSec; }
    auto getSectorPerCluster() const { return (uint16_t) sectorPerCluster; }
    auto getNumberOfFAT() const { return (uint16_t) numberOfFAT; }
    auto getSectorOfFat() const { return sectorOfFat; }
    auto getRootDirCluster() const {return rootDirCluster; }
    auto get_fat_area_addr() const {return fat_area_addr; }
    auto get_fat_area_size() const {return fat_area_size; }
    auto get_data_area_start() const {return data_area_start; }
    auto get_cluster_size() const {return cluster_size;}
};

Superblock::Superblock(char *buffer, int offset, int count){

  sys::io::byte_buffer bb((uint8_t*)buffer, offset, count);

  bb.skip(0x0B);
  bytesPecSec = bb.get_int16_le();
  sectorPerCluster = bb.get_int8();
  reservedSec = bb.get_int16_le();
  numberOfFAT = bb.get_int8();
  bb.skip(0x13); //0x23

  sectorOfFat = bb.get_int32_le();
  bb.skip(0x04); //0x2C
  rootDirCluster = bb.get_int32_le();

  cluster_size = bytesPecSec * sectorPerCluster;
  fat_area_addr = reservedSec * bytesPecSec;
  fat_area_size = numberOfFAT * sectorOfFat * bytesPecSec;
  data_area_start = fat_area_size + fat_area_addr;

}

class FATArea {

  protected:
    vector<uint32_t> cluster;

  public:
    FATArea(char *buffer, int offset, int count);
    int getNumberOfCluster(int startcluster);

};

FATArea::FATArea(char *buffer, int offset, int count) {

  sys::io::byte_buffer bb((uint8_t*)buffer, offset, count);
  
  for (int i = 0 ; i < count/4 ; i++)
    cluster.push_back(bb.get_uint32_le());

}

int FATArea::getNumberOfCluster(int startcluster) {

  while ( cluster.at(startcluster++) != 0x0FFFFFFF) {}

  return startcluster;
}

class DirEntry {

  protected:
    uint8_t deletedOrNot;
    uint8_t attr;
    uint16_t high_cluster;
    uint16_t low_cluster;
    uint32_t cluster_no;
    uint32_t file_size;
    string file_name;
    string ext_name;

  public:
    DirEntry(char *buffer, int offset, int count);
    uint8_t getDeletedOrNot() const { return deletedOrNot; }
    uint8_t getAttr() const { return attr; }
    uint16_t getHighCluster() const { return high_cluster; }
    uint16_t getLowCluster() const { return low_cluster; }
    uint32_t getFileSize() const { return file_size; }
    uint32_t getCluster_no() const { return cluster_no; }
    string getFileName() const { return file_name; }
    string getExtName() const {return ext_name; }
    bool isFile();
    bool isDir();
    bool isDeleted();

};

bool DirEntry::isDeleted() {
  if (deletedOrNot == 0xE5) 
    return true;
  return false;
}

bool DirEntry::isDir() {
  if (attr == 0x10) 
    return true;
  return false;
}

bool DirEntry::isFile() {
  if (attr == 0x20)
    return true;
  return false;
}

DirEntry::DirEntry(char *buffer, int offset, int count) {

  sys::io::byte_buffer bb((uint8_t*)buffer, offset, count);

  deletedOrNot = bb.get_int8();

  string trim_temp;

  if ( deletedOrNot == 0xE5)
    trim_temp = bb.get_ascii(7);
  else {
    bb.reset();
    trim_temp = bb.get_ascii(8);
  }
  
  file_name = trim(trim_temp);
  trim_temp = bb.get_ascii(3);
  ext_name = trim(trim_temp);

  attr = bb.get_int8();

  bb.skip(0x08);
  high_cluster = bb.get_int16_le();

  bb.skip(0x04);
  low_cluster = bb.get_int16_le();

  cluster_no = (high_cluster << 16) | (low_cluster);
  file_size = bb.get_int32_le();

}

class Node {

  public:
    void export_to();
    int type;
    int deletedOrNot;
    string name;
    int filesize;
    fstream ifs;
    map<int, int> extents;
    bool isDeleted();
    bool isDir();
    bool isFile();
    vector <string> DirList;
  
};

bool Node::isDeleted() {
  if (deletedOrNot == 0xE5) 
    return true;
  return false;
}

bool Node::isDir() {
  if (type == 0x10) 
    return true;
  return false;
}

bool Node::isFile() {
  if (type == 0x20)
    return true;
  return false;
}

Node* to_node (DirEntry *dir, Superblock *superblock, FATArea *fatArea) {

  Node *node = new Node();

  if (dir->isFile() )
    node->name = dir->getFileName() + "." + dir->getExtName();
  else
    node->name = dir->getFileName();


  node->filesize = dir->getFileSize();
  node->type = dir->getAttr();
  node->deletedOrNot = dir->getDeletedOrNot();

  int physical_addr = superblock->get_data_area_start() + ((dir->getCluster_no() - 2) * superblock->get_cluster_size());

  if (dir->isDeleted())
    node->extents.insert( pair<int, int> (physical_addr, dir->getFileSize())) ;
  else if (dir->isFile())
    // node->extents.insert( pair<int, int> (physical_addr, fatArea->getNumberOfCluster(dir->getCluster_no()) * superblock->get_cluster_size() ));
    node->extents.insert( pair<int, int> (physical_addr, dir->getFileSize() ));
  else if (dir->isDir()) { 
    node->extents.insert( pair<int, int> (physical_addr, dir->getFileSize())) ;
    // node->DirList.push_back(node->name);
  }

  return node;
}

class Fat32{

  protected:
    Superblock *superblock;
    FATArea *fatArea;
    vector<string> dirList;
    
  public:
    Fat32(string filename);
    void processFile(Node* node, fstream &ifs) ;
    void processDirectoryRecursively(Node *node, fstream &ifs) ;
    void processDeleteFile(Node* node, fstream &ifs) ;
    void export_to(string filename, int dataLocation);

};

void Fat32::processDeleteFile(Node* node, fstream &ifs){

  int physical_addr;
  int size;
  map extents = node->extents;

  for (std::map<int, int>::iterator it = extents.begin(); it != extents.end(); ++it) {
    physical_addr = it->first;
    size = it->second;
  }

  ifs.seekg(physical_addr);

  char buf[size];
  ifs.read(buf, size);

  sys::io::byte_buffer bb((uint8_t*)buf, 0, size);

  ofstream hexFile(node->name);

  hexFile << bb.get_ascii(size) << endl;

  hexFile.close();

}
void Fat32::processDirectoryRecursively(Node *node, fstream &ifs) {

  int physical_addr;
  int size;
  map extents = node->extents;

  cout << node->name << endl;
  // dirList.push_back(dirList.back() + node->name);

  for (std::map<int, int>::iterator it = extents.begin(); it != extents.end(); ++it) {
    physical_addr = it->first;
    size = it->second;
  }

  for (int j = 0; j < superblock->get_cluster_size(); j += 0x20) {

      char dirBuf[0x20];
      ifs.seekg(physical_addr + j);
      ifs.read(dirBuf, 0x20);

      DirEntry *subDirEntry = new DirEntry(dirBuf, 0, 0x20);
      Node *node = to_node(subDirEntry, superblock, fatArea);

      if (node->isFile()) {
          processFile(node, ifs);
          delete subDirEntry;
      } 
      else if (node->isDir()) {
          if (node->deletedOrNot != 0x2E){ //heuristic (not starting with .)
            processDirectoryRecursively(node, ifs);
          } 
            
          delete subDirEntry;
      }
  }

}

void Fat32::processFile(Node* node, fstream &ifs) {

  int physical_addr;
    int size;
    map extents = node->extents;

 
    for (std::map<int, int>::iterator it = extents.begin(); it != extents.end(); ++it) {
      physical_addr = it->first;
      size = it->second;
    }

    size = node->filesize;

    char fileBUf[size];

    ifs.seekg(physical_addr);
    ifs.read(fileBUf, size);
    sys::io::byte_buffer bb((uint8_t*)fileBUf, 0, size);

    ofstream hexFile(node->name);

    hexFile << bb.get_ascii(size) << endl;

    hexFile.close();

}

Fat32::Fat32(string filename) {

  fstream ifs(filename);

  if (!ifs.good())
    cout << "error";

  char buffer[200] = {0};
  ifs.read(buffer, 200);

  superblock = new Superblock(buffer, 0, 200);

  char fat_buffer[superblock->get_fat_area_size()];
  ifs.seekg( superblock->get_fat_area_addr() );
  ifs.read( fat_buffer, superblock->get_fat_area_size() );

  fatArea = new FATArea(fat_buffer, 0, superblock->get_fat_area_size());

  // dirList.push_back("/");

  char root_dir[0x20];

  for (int i = 0 ; i < superblock->get_cluster_size() ; i+=0x20) {

    ifs.seekg(superblock->get_data_area_start() + i);
    ifs.read(root_dir, 0x20);
  
    DirEntry *dirEntry = new DirEntry(root_dir, 0, 0x20);

    Node *new_node = to_node(dirEntry, superblock, fatArea);

    if (new_node->isDir()) {
      processDirectoryRecursively(new_node, ifs);
    } else if (new_node->isFile() && new_node->isDeleted()) {
      
      processDeleteFile(new_node, ifs);
    }

    delete dirEntry;
  
  }
  
}

void Fat32::export_to(string filename, int dataLocation) {

}

int main(int argc, char* argv[])
{
  Fat32 *fat32 = new Fat32("FAT32_simple.mdf");


  return 0;
}
