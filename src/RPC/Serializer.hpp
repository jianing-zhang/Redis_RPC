#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstring>

using std::vector;


/*
    其实是为了能够移动自定义指针，可以记录解析的位置
    采用vector<char> 因为是一个动态数组，可以存储任意数量的 char 元素，、
    并且可以动态地增加或减少元素。这使得 std::vector<char> 非常适合用作字节流的缓冲区

    然而，std::vector<char> 并没有提供一些字节流操作所需要的功能，
    例如移动当前位置、查找特定的字节、检查是否已经到达末尾等。
    因此，StreamBuffer 类继承自 std::vector<char>，并添加了这些功能。
*/
class StreamBuffer : public vector<char>{
public:
    StreamBuffer() : m_curpos(0){}

    StreamBuffer(const char* in, size_t len){
        m_curpos=0;
        insert(begin(),in,in+len);
    }

    ~StreamBuffer(){};

    void reset(){ m_curpos = 0; }

    //获取缓冲区的数据，返回数组头指针
    const char* data() { return &(*this)[0]; }

    //返回当前移动到的位置
    const char* current(){ return &(*this)[m_curpos]; }

    //移动当前指向的元素
    void offset(int offset) { m_curpos += offset; }

    //检查是否已经到达末尾
    bool is_eof(){ return m_curpos >= size(); }

    //往末尾添加数据
    bool input(const char* in, size_t len){ insert(end(), in, in+len); }

    //在缓冲区中查找特定的字节
    int findc(char c){
        iterator itr = std::find(begin()+m_curpos, end(), c);
        if(itr != end()){
            return itr - (begin()+m_curpos);
        }
        return -1;
    }

private:
    unsigned int m_curpos;
};

/*
    序列号和反序列号的类
    用于将数据序列化为字节流，或者将字节流反序列化为数据

    存储数据：写入数据长度和数据本身
    读取数据：读取数据长度，然后读取数据本身
*/
class Serializer{
public:
    //定义字节序枚举类型
    enum ByteOrder{
        BigEndian = 0,
        LittleEndian = 1
    };

    Serializer() : m_byteorder(LittleEndian){
    }

    Serializer(StreamBuffer dev, ByteOrder byteorder = LittleEndian) 
    : m_byteorder(byteorder), m_iodevice(dev) {
    }

    //字节流指针重置
    void reset(){
        m_iodevice.reset();
    }
    //返回字节流大小
    int size(){
        return m_iodevice.size();
    }
    //字节流指针前进k步
    void skip_raw_date(int k){
        m_iodevice.offset(k);
    }
    //返回当前字节流的数组头指针，即返回字节流
    const char* data(){
        return m_iodevice.data();
    }

    //转换为小端序
    void byte_orser(char* in, int len){
        if(m_byteorder == BigEndian){
            std::reverse(in, in+len);
        }
    }

    /**
     * @brief 将指定的数据写入字节流中
     * @param in 输入的字符串数组
     * @param len 输入字符数组的长度
    */
    void write_raw_data(char* in, int len){
        m_iodevice.input(in, len);
        m_iodevice.offset(len);
    }

    //返回字节流当前指向的指针
    const char* current(){
        return m_iodevice.current();
    }

    /**
     * @brief 清空序列化容器的数据
    */
    void clear(){
        m_iodevice.clear();
        reset();
    }

    /**
	 * @brief 输出指定类型的数据
	 * @tparam T 要输出的数据类型
	 * @param t 输出数据到t
	 */
    template<typename T>
    void output_type(T& t);

    /**
	 * @brief 输入指定类型的数据
	 * @tparam T 要输入的数据类型
	 * @param t 要输入的数据
	 */
    template<typename T>
    void input_type(T t);

    /**
	 * @brief 重载运算符>>，用于从序列化器中读取数据。
	 * @tparam T 要读取的数据类型。
	 * @param t 用于存储读取结果的变量。
	 * @return 当前序列化器对象的引用。
	 */
    template<typename T>
    Serializer& operator>> (T &i){
        output_type(i);
        return *this;
    }

    /**
	 * @brief 重载运算符<<，用于向序列化器中写入数据。
	 * @tparam T 要写入的数据类型。
	 * @param i 要写入的数据。
	 * @return 当前序列化器对象的引用。
	 */
    template<typename T>
	Serializer &operator << (const T &i){
		input_type(i);
		return *this;
	}

private:
    ByteOrder m_byteorder; //字节序
    StreamBuffer m_iodevice;    //字节流缓冲区
};

template<typename T>
inline void Serializer::output_type(T &t){
    int len = sizeof(T);
    char *d = new char[len];
    if(!m_iodevice.is_eof()){
        memcpy(d,m_iodevice.current,len);
        m_iodevice.offset(len);
        byte_orser(d,len);
        t = *reinterpret_cast<T*>(&d[0]);
    }
}


template<>
inline void Serializer::output_type(std::string &in){
    //针对string类型，先取出长度，后根据长度存入到string中
    int marklen = sizeof(uint16_t); //unsigned short类型
    char *d = new char[marklen];
    memcpy(d, m_iodevice.current(),marklen);
    byte_orser(d,marklen);
    m_iodevice.offset(marklen); //将字节流的位置向后移动两个字节
    int len = *reinterpret_cast<uint16_t*>(&d[0]);  //取出字符串长度
    delete[] d;
    if(len==0) return;

    in.insert(in.begin(), m_iodevice.current(), m_iodevice.current()+len);
    m_iodevice.offset(len);
}


/**
 * @brief 针对所有类型的数据，通用版本，进行输入
*/
template<typename T>
inline void Serializer::input_type(T t){
    int len = sizeof(T);
    char *d = new char[len];
    const char *p = reinterpret_cast<const char*>(&t);
    memcpy(d,p,len);
    byte_orser(d,len);
    m_iodevice.input(d,len);
    delete[] d;
}

/**
 * @brief 将string输入到字节流中。
 * @param in 要输入的字符串。
 */
template<>
inline void Serializer::input_type(std::string in){
    //先将字符串的长度输入到字节流中
    uint16_t len = in.size();
    char *p = reinterpret_cast<char*>(&len);
    byte_orser(p,sizeof(uint16_t));
    m_iodevice.input(p,sizeof(uint16_t));
    if(len==0) return;

    //再将字符串的数据输入到字节流中
    char *d = new char[len];
    memcpy(d,in.c_str(),len);
    m_iodevice.input(d,len);
    delete[] d;
}


/**
 * @brief Inputs a null-terminated string into the serializer.
 * @param in The null-terminated string to input.
 */
template<>
inline void Serializer::input_type(const char* in)
{
	input_type<std::string>(std::string(in)); //调用input_type<std::string>函数
}


#endif
