#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <random>
#include <string>
#include <mutex>
#include <cstring>

#define MAX_SKIP_LIST_LEVEL 32
#define PROBABILITY_FACTOR 0.25
#define DELIMITER ":"
#define  SAVE_PATH "data_file"


//跳表节点
template <typename Key, typename Value>
struct SkipListNode{
    Key key;
    Value value;
    std::vector<std::shared_ptr<SkipListNode<Key,Value>>> forward;
    SkipListNode(Key k, Value v, int maxLevel=MAX_SKIP_LIST_LEVEL) : 
        key(k),value(v),forward(maxLevel, nullptr){}; 
}; 

template <typename Key, typename Value>
class SkipList{
public:
    SkipList();
    ~SkipList();
    bool addItem(const Key &key, const Value &value);   //增添节点
    bool modifyItem(const Key &key, const Value &value);    //修改节点
    std::shared_ptr<SkipListNode<Key,Value>> searchItem(const Key &key); //查找节点
    bool deleteItem(const Key &key); //删除节点
    void printList();   //打印跳表
    // void dumpFile(std::string save_path); //保存跳表到文件中;
    // void loadFile(std::string load_path); //从文件中加载到跳表中
    int size(); //返回跳表元素个数

public:
    int getCurrentLevel(){ return currentLevel; }       //获取当前层数
    std::shared_ptr<SkipListNode<Key,Value>> getHead(){ return head; }  //获取头节点

private:
    //随机生成需要插入节点的层数
    int randomLevel();
    // bool parseString(const std::string &line, std::string &key, std::string &value);
    // bool isVaildString(const std::string &line);

private:
    int currentLevel; //当前跳表的最大层数
    std::shared_ptr<SkipListNode<Key, Value>> head; //头节点
    std::mt19937 generator{std::random_device{}()}; //随机数生成器
    std::uniform_real_distribution<double> distribution; //随机数分布，限定随机数生成器生成的数字范围
    int elementNumber=0;
    std::ofstream writeFile; //文件输出流，即输出到文件中
    std::ifstream readFile; //文件输入留，即从文件中读取内容
    std::mutex mutex;       //互斥锁

};


template <typename Key, typename Value>
SkipList<Key,Value>::SkipList() : currentLevel(0), distribution(0,1) {
    Key key;
    Value value;
    head = std::make_shared<SkipListNode<Key, Value>>(key,value);
}

template <typename Key, typename Value>
SkipList<Key, Value>::~SkipList(){
    if(readFile)
        readFile.close();
    if(writeFile)
        writeFile.close();
}

template <typename Key, typename Value>
int SkipList<Key, Value>::randomLevel(){
    int level = 1;
    while(distribution(generator)<PROBABILITY_FACTOR && level < MAX_SKIP_LIST_LEVEL){
        ++level;
    }
    return level;
}

template <typename Key, typename Value>
int SkipList<Key, Value>::size(){
    mutex.lock();
    int ret = this->elementNumber;
    mutex.unlock();
    return ret;
}

//为跳表添加节点，注意，在插入钱没有判断是否存在，所以在使用addItem之前需要看一下是否存在，所以这里可以优化一下
template<typename Key, typename Value>
bool SkipList<Key, Value>::addItem(const Key& key, const Value &value){
    mutex.lock();
    auto currentNode = head;
    //记录每层需要更新的节点
    std::vector<std::shared_ptr<SkipListNode<Key, Value>>>update(MAX_SKIP_LIST_LEVEL,head);
    //寻找需要添加的节点位置，同时记录每层需要更新的节点
    for(int lv=currentLevel-1; lv>=0; lv--){
        while(currentNode->forward[lv] && currentNode->forward[lv]->key<key){
            currentNode = currentNode->forward[lv];
        }
        update[lv]=currentNode;
    }
    // 生成新节点的层数
    int newLevel = randomLevel();
    // 更新当前跳表的最大层数
    currentLevel = std::max(newLevel, currentLevel);
    // 只分配节点的层高
    std::shared_ptr<SkipListNode<Key,Value>> newNode = std::make_shared<SkipListNode<Key,Value>>(key,value,newLevel);
    for(int i=0; i<newLevel; i++){
        newNode->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = newNode;
    }
    elementNumber++;
    mutex.unlock();
    return true;
}

template <typename Key, typename Value>
std::shared_ptr<SkipListNode<Key,Value>> SkipList<Key,Value>::searchItem(const Key &key){
    mutex.lock();
    auto currentNode = head;
    if(!currentNode){
        mutex.unlock();
        return nullptr;
    }
    for(int i=currentLevel-1; i>=0; i++){
        while(currentNode->forward[i] && currentNode->forward[i]->key<key){
            currentNode = currentNode->forward[i];
        }
    }
    if(currentNode->forward[0]->key==key){
        //可以优化，不然先解锁后返回的是指针，怕指针内容被修改删除
        mutex.unlock();
        return currentNode->forward[0];
    }
    mutex.unlock();
    return nullptr;
}


template<typename Key, typename Value>
bool SkipList<Key, Value>::modifyItem(const Key &key, const Value &value){
    std::shared_ptr<SkipListNode<Key, Value>> targetNode = searchItem(key);
    mutex.lock();
    if(targetNode == nullptr){
        mutex.unlock();
        return false;
    }
    targetNode->value = value;
    mutex.unlock();
    return true;
}


template <typename Key, typename Value>
bool SkipList<Key,Value>::deleteItem(const Key &key){
    mutex.lock();
    std::shared_ptr<SkipListNode<Key,Value>> currentNode = head;
    std::vector<std::shared_ptr<SkipListNode<Key,Value>>> update(MAX_SKIP_LIST_LEVEL,nullptr);
    for(int i=currentLevel-1; i>=0; i--){
        while(currentNode->forward[i] && currentNode->forward[i]->key<key){
            currentNode = currentNode->forward[i];
        }
        update[i] = currentNode;
    }
    currentNode=currentNode->forward[0];
    //找不到当前需要删除的键,如果找不到，则直接退出
    if(!currentNode || currentNode->key!=key){
        mutex.unlock();
        return false;
    }
    
    for(int i=0; i<currentLevel; i++){
        //找到需要删除的键，删除它
        if(update[i]->forward[i]!=currentNode){
           break;
        }
        update[i]->forward[i] = currentNode->forward[i];
    }
    currentNode.reset();
    while(currentLevel>0 && head->forward[currentLevel-1]==nullptr){
        currentLevel--;
    }
    elementNumber--;
    mutex.unlock();
    return true;
}


//打印跳表
template <typename Key, typename Value>
void SkipList<Key, Value>::printList(){
    mutex.lock();
    for(int i=currentLevel-1; i>=0; i--){
        auto node = head->forward[i];
        std::cout<<"Level"<<i+1<<":";
        while(node!=nullptr){
            std::cout<<node->key<<DELIMITER<<node->value<<"; ";
            node=node->forward[i];
        }
        std::cout<<std::endl;
    }
    mutex.unlock();
}


// template <typename Key, typename Value>
// void SkipList<Key,Value>::dumpFile(std::string save_path){
//     mutex.lock();
//     writeFile.open(save_path);
//     auto node = head->forward[0];
//     while(!node){
//         //写入文件 dump()函数将value转换为字符串 整数 1->"1" 字符串 "hello"->"hello" 二进制数据 0x01 0x02 0x03->"0x01 0x02 0x03"
//         writeFile<<node->key<<DELIMITER<<node->value.dump()<<"\n";
//         node = node->forward[0];
//     }
//     //刷新缓冲区 写入文件 直接写入文件 不用等到文件关闭 
//     //输出的内容会先存入缓冲区,而flush()的作用正是强行将缓冲区的数据清空
//     writeFile.flush();
//     writeFile.close();
//     mutex.unlock();
// }


// template<typename Key, typename Value>
// void SkipList<Key, Value>::loadFile(std::string load_path){
//     readFile.open(load_path);
//     if(!readFile.is_open()){
//         return;
//     }
//     std::string line;
//     std::string key;
//     std::string value;
//     std::string err;
//     while(std::getline(readFile,line)){
//         if(parseString(line, key, value)){
//             //将字符串抓换为RedisValue对象
//             addItem(key, RedisValue::parse(value,err));
//         }
//     }
//     readFile.close();
// }

// template <typename Key, typename Value>
// bool SkipList<Key,Value>::isVaildString(const std::string &line){
//     if(line.empty()){
//         return false;
//     }
//     if(line.find(DELIMITER)==std::string::nops){
//         return false;
//     }
//     return true;
// }

// template <typename Key, typename Value>
// bool SkipList<Key,Value>::parseString(const std::string &line, std::string &key, std::string &value){
//     if(!isValidString(line)){
//         return fasle;
//     }
//     int index = line.find(DELIMITER);
//     key = line.substr(0,index);
//     value = line.substr(index+1);
//     return true;
// }


#endif