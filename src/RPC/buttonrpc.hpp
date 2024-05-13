#ifndef BUTTONRPC_HPP
#define BUTTONRPC_HPP

#include <string>
#include <map>
#include <sstream>
#include <functional>
#include <zmq.hpp>
#include "Serializer.hpp"

template <typename T>
struct type_xx{
    typedef T type;
};

//模版特例化，如果T是void类型，type就为int8_t
template<>
struct type_xx<void>{
  typedef int8_t type;  
};


class buttonrpc{
public:
    //定义rpc类型，是服务端还是客户端
    enum rpc_role{
        RPC_CLIENT,
        RPC_SERVER
    };

    enum rpc_err_code{
        RPC_ERR_SUCCESS = 0,    //成功
        RPC_ERR_FUNCTION_NOT_BIND = 1,  //函数未绑定
        RPC_ERR_RECV_TIMEOUT    //接受超时
    };

    template<typename T>
    class value_t{
    public:
        typedef typename type_xx<T>::type type;
        typedef std::string msg_type;
        typedef uint16_t code_type;     //可以换成rpc_err_code，因为后续就是用这个，但最好不要，因为后续有<<操作

        value_t(){ 
            code_ = 0;
            msg_.clear();
        }

        bool valid() { return code_==0; }
        int error_code() { return code_;} //返回错误码
        msg_type error_msg() { return msg_; }   //返回错误信息
        type val() { return val_; } //返回值

        void set_val(const type &val) { val_= val; }
        void set_code(code_type code) { code_ = code; }
        void set_msg(const msg_type &msg) { msg_ = msg; }

        //定义友元函数，重载Serializer针对value_t类型的>>运算符
        friend Serializer& operator >> (Serializer &in, value_t<T> &d){
            in>>d.code_>>d.msg_;
            if(d.code_==0){
                in>>d.val_;
            }
            return in;
        }

        //定义友元函数，重载Serializer针对value_t类型的<<运算符
        friend Serializer& operator << (Serializer &out, value_t<T> &d){
            out<<d.code_<<d.msg_<<d.val_;
            return out;
        }

    private:
        code_type code_;
        msg_type msg_;
        type val_;
    };

    buttonrpc();
    ~buttonrpc();

    //network
    void as_client(std::string ip, int port);
    void as_server(int port);
    void send(zmq::message_t &data);
    void recv(zmq::message_t &data);
    void set_timeout(uint32_t ms);
    void run();

public:
    //绑定普通函数
    template<typename F>
    void bind(std::string name, F func);
    
    //绑定类成员函数
    template<typename F, typename S>
    void bind(std::string name, F func, S *s);

    //client
    //客户端单参数调用
    template<typename R>
    value_t<R> call(std::string name);
    //一个参数
    template<typename R, typename P1>
    value_t<R> call(std::string name, P1);
    //两个参数
    template<typename R, typename P1, typename P2>
    value_t<R> call(std::string name, P1, P2);
    //三个参数
    template<typename R, typename P1, typename P2, typename P3>
    value_t<R> call(std::string name, P1, P2, P3);
    //四个参数
    template<typename R, typename P1, typename P2, typename P3, typename P4>
    value_t<R> call(std::string name, P1, P2, P3, P4);
    //五个参数
    template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
    value_t<R> call(std::string name, P1, P2, P3, P4, P5);

private:
    //服务端的函数调用，根据函数名字和对应的参数调用对应的函数
    Serializer* call_(std::string name, const char* data, int len);

    //客户端call实际调用net_call进行远程调用，接受返回结果
    template<typename R>
    value_t<R> net_call(Serializer &ds);

    //接受普通函数的统一接口函数
    template<typename F>
    void callproxy(F fun, Serializer *pr, const char *data, int len);

    //接受类成员函数的统一接口函数
    template<typename F, typename S>
    void callproxy(F fun, S *s, Serializer *pr, const char* data, int len);

    //接受普通函数的统一接口函数的辅助函数，即接受普通函数的callproxy里调用callproxy_
    template<typename R>
    void callproxy_(R(*func)(), Serializer *pr, const char *data, int len){
        callproxy_(std::function<R()>(func), pr, data, len);
    }

    template<typename R, typename P1>
    void callproxy_(R(*func)(P1), Serializer *pr, const char *data, int len){
        callproxy_(std::function<R(P1)>(func), pr, data, len);
    } 

    template<typename R, typename P1, typename P2>
	void callproxy_(R(*func)(P1, P2), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2)>(func), pr, data, len);
	}

	template<typename R, typename P1, typename P2, typename P3>
	void callproxy_(R(*func)(P1, P2, P3), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3)>(func), pr, data, len);
	}

    template<typename R, typename P1, typename P2, typename P3, typename P4>
	void callproxy_(R(*func)(P1, P2, P3, P4), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3, P4)>(func), pr, data, len);
	}

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
	void callproxy_(R(*func)(P1, P2, P3, P4, P5), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3, P4, P5)>(func), pr, data, len);
	}

    //接受类成员函数的统一接口函数的辅助函数，即接受类成员函数的callproxy里调用callproxy_
    //function不能包装类成员变量或函数，需要配合Bind,传入函数地址和类对象地址
    template<typename R, typename C, typename S>
    void callproxy_(R(C::*func)(), S *s, Serializer *pr, const char* data, int len){
        callproxy_(std::functional<R()>(std::bind(func,s)), pr, data, len);
    }

    template<typename R, typename C, typename S, typename P1>
    void callproxy_(R(C::*func(P1)), S *s, Serializer *pr, const char *data, int len){
        callproxy_(std::function<R(P1)>(std::bind(func,s,std::placeholders::1)), pr, data, len);
    }

    template<typename R, typename C, typename S, typename P1, typename P2>
	void callproxy_(R(C::* func)(P1, P2), S* s, Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2)>(std::bind(func, s, std::placeholders::_1, std::placeholders::_2)), pr, data, len);
	}

	template<typename R, typename C, typename S, typename P1, typename P2, typename P3>
	void callproxy_(R(C::* func)(P1, P2, P3), S* s, Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3)>(std::bind(func, s, 
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), pr, data, len);
	}

	template<typename R, typename C, typename S, typename P1, typename P2, typename P3, typename P4>
	void callproxy_(R(C::* func)(P1, P2, P3, P4), S* s, Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3, P4)>(std::bind(func, s,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)), pr, data, len);
	}

	template<typename R, typename C, typename S, typename P1, typename P2, typename P3, typename P4, typename P5>
	void callproxy_(R(C::* func)(P1, P2, P3, P4, P5), S* s, Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(P1, P2, P3, P4, P5)>(std::bind(func, s,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)), pr, data, len);
	}

    //PROXY FUNCTIONAL
    template<typename R>
    void callproxy_(std::function<R()>, Serializer *pr, const char *data, int len);

    template<typename R, typename P1>
	void callproxy_(std::function<R(P1)>, Serializer* pr, const char* data, int len);

    template<typename R, typename P1, typename P2>
	void callproxy_(std::function<R(P1, P2)>, Serializer* pr, const char* data, int len);

	template<typename R, typename P1, typename P2, typename P3>
	void callproxy_(std::function<R(P1, P2, P3)>, Serializer* pr, const char* data, int len);

	template<typename R, typename P1, typename P2, typename P3, typename P4>
	void callproxy_(std::function<R(P1, P2, P3, P4)>, Serializer* pr, const char* data, int len);

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
	void callproxy_(std::function<R(P1, P2, P3, P4, P5)>, Serializer* pr, const char* data, int len);

private:
    std::map<std::string, std::function<void(Serializer*, const char*, int)>> m_handlers; //函数映射表
    zmq::context_t m_context; //上下文
    zmq::socket_t *m_socket; //套接字
    rpc_err_code m_error_code; //错误码
    int m_role;
};

buttonrpc::buttonrpc() : m_context(1){
    m_error_code = RPC_ERR_SUCCESS;
}

buttonrpc::~buttonrpc(){
    m_socket->close();
    delete m_socket;
    m_context.close();
}

void buttonrpc::as_client(std::string ip, int port){
    m_role = RPC_CLIENT;
    m_socket = new zmq::socket_t(m_context, ZMQ_REQ);
    std::ostringstream os;
    os<<"tcp://"<<ip<<":"<<port;
    m_socket->connect(os.str());
}

void buttonrpc::as_server(int port){
    m_role = RPC_SERVER;
    m_socket = new zmq::socket_t(m_context, ZMQ_REP);
    std::ostringstream os;
    os<<"tcp://*"<<port;
    m_socket->bind(os.str());
}

void buttonrpc::send(zmq::message_t &data){
    m_socket->send(data);
}

void buttonrpc::recv(zmq::message_t &data){
    m_socket->recv(data);
}

inline void buttonrpc::set_timeout(uint32_t ms)
{
	// // only client can set
	// if (m_role == RPC_CLIENT) {
	// 	m_socket->setsockopt(ZMQ_RCVTIMEO, ms); //设置接收超时时间
	// }
}

void buttonrpc::run(){
    if(m_role != RPC_CLIENT)
        return;
    while(1){
        zmq::message_t data;
        recv(data);
        StreamBuffer iodev(static_cast<char*>(data.data()), data.size());
        Serializer ds(iodev);

        std::string funname;
        ds>>funname;    //读取函数名
        //可以优化，使用智能指针
        Serializer *r = call_(funname, ds.current(), ds.size()-funname.size());

        zmq::message_t retmsg(r->size());
        std::memcpy(retmsg.data(), r->data(), r->size());
        send(retmsg);
        delete r;
    }
}

//实现函数调用
Serializer* buttonrpc::call_(std::string name, const char* data, int len){
    Serializer *ds = new Serializer();
    if(m_handlers.find(name) == m_handlers.end()){
        (*ds)<<value_t<int>::code_type(RPC_ERR_FUNCTION_NOT_BIND);
        (*ds)<<value_t<int>::msg_type("function not bind: " + name); //设置错误信息
        return ds;
    }
    auto fun = m_handlers[name];
    fun(ds, data, len);
    ds->reset();
    return ds;
}

//绑定普通函数
template<typename F>
void buttonrpc::bind(std::string name, F func){
    m_handlers[name] = std::bind(&buttonrpc::callproxy<F>, this, func, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}
//绑定类成员函数
template<typename F, typename S>
inline void buttonrpc::bind(std::string name, F func, S* s)
{
	m_handlers[name] = std::bind(&buttonrpc::callproxy<F, S>, this, func, s, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

template<typename F>
inline void buttonrpc::callproxy(F fun, Serializer *pr, const char* data, int len){
    callproxy_(fun,pr,data,len);
}

template<typename F, typename S>
inline void buttonrpc::callproxy(F fun, S *s, Serializer *pr, const char *data, int len){
    callproxy_(fun, s, pr, data, len);
}

#pragma region 区分返回值
template<typename R, typename F>
typename std::enable_if<std::is_same<R,void>::value, typename type_xx<R>::type>::type call_helper(F f){
    f();
    return 0;
}

template<typename R, typename F>
typename std::enable_if<!std::is_same<R,void>::value, typename type_xx<R>::type>::type call_helper(F f){
    return f();
}
#pragma endregion

/**
 * @brief 重载callproxy_，具体绑定的函数调用的实现
 * @tparam R 返回值类型
 * @param func 绑定的函数
 * @param pr 函数运行结果保存到pr中
 * @param data 客户端传过来的字节流
 * @param len 字节流长度
*/
template<typename R>
void buttonrpc::callproxy_(std::function<R()>func, Serializer *pr, const char* data, int len){
    typename type_xx<R>::type r = call_helper<R>(std::bind(func));

    value_t<R> val;
    val.set_code(RPC_ERR_SUCCESS);
    val.set_val(r);
    (*pr)<<val;
}

template<typename R, typename P1>
void buttonrpc::callproxy_(std::function<R(P1)> func, Serializer *pr, const char *data, int len){
    Serializer* ds(StreamBuffer(data,len));
    P1 p1;
    ds>>p1;
    
    typename type_xx<R>::type r = call_helper<R>(std::bind(func,p1));

    value_t<R> val;
    val.set_code(RPC_ERR_SUCCESS);
    val.set_val(r);
    (*pr)<<val;
}

template<typename R, typename P1, typename P2>
void buttonrpc::callproxy_(std::function<R(P1,P2)> fuc, Serializer *pr, const char *data, int len){
    Serializer *ds(StreamBuffer(data,len));
    P1 p1;
    P2 p2;
    ds>>p1>>p2;
    typename type_xx<R>::type r = call_helper<R>(std::bind(func,p1,p2));

    value_t<R> val;
    val.set_code(RPC_ERR_SUCCESS);
    val.set_val(r);
    (*pr)<<val;
}

template<typename R, typename P1, typename P2, typename P3>
void buttonrpc::callproxy_(std::function<R(P1,P2,P3)> func, Serializer *pr, const char *data, int len){
    Serializer ds(StreamBuffer(data,len));
    P1 p1;
    P2 p2;
    P3 p3;
    ds>>p1>>p2>>p3;
    typename type_xx<R>::type r = call_helper<R>(std::bind(func,p1,p2,p3));

    value_t<R> val;
    val.set_code(RPC_ERR_SUCCESS);
    val.set_val(r);
    (*pr)<<val;
}

template<typename R, typename P1, typename P2, typename P3, typename P4>
void buttonrpc::callproxy_(std::function<R(P1, P2, P3, P4)> func, Serializer* pr, const char* data, int len)
{
	Serializer ds(StreamBuffer(data, len));
	P1 p1; P2 p2; P3 p3; P4 p4;
	ds >> p1 >> p2 >> p3 >> p4;
	typename type_xx<R>::type r = call_helper<R>(std::bind(func, p1, p2, p3, p4));
	value_t<R> val;
	val.set_code(RPC_ERR_SUCCESS);
	val.set_val(r);
	(*pr) << val;
}

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
void buttonrpc::callproxy_(std::function<R(P1, P2, P3, P4, P5)> func, Serializer* pr, const char* data, int len)
{
	Serializer ds(StreamBuffer(data, len));
	P1 p1; P2 p2; P3 p3; P4 p4; P5 p5;
	ds >> p1 >> p2 >> p3 >> p4 >> p5;
	typename type_xx<R>::type r = call_helper<R>(std::bind(func, p1, p2, p3, p4, p5));
	value_t<R> val;
	val.set_code(RPC_ERR_SUCCESS);
	val.set_val(r);
	(*pr) << val;
}

template<typename R>
inline buttonrpc::value_t<R> buttonrpc::net_call(Serializer &ds){
    zmq::message_t request(ds.size()+1);
    memcpy(request.data(), ds.data(), ds.size());
    if(m_error_code!=RPC_ERR_RECV_TIMEOUT){
        send(request);
    }
    zmq::message_t reply;
    recv(reply);
    value_t<R> val;
    if(reply.size()==0){
        m_error_code = RPC_ERR_RECV_TIMEOUT;
		val.set_code(RPC_ERR_RECV_TIMEOUT);
		val.set_msg("recv timeout");
		return val;
    }
    m_error_code = RPC_ERR_SUCCESS;
    ds.clear();
    ds.write_raw_data((char*)reple.data(), reply.size());
    ds.reset();

    ds>>val;
    return val;
}

template<typename R>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name){
    Serializer ds;
    ds<<name;
    return net_call<R>(ds);
}

template<typename R, typename P1>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name, P1 p1)
{
	Serializer ds;
	ds << name << p1;
	return net_call<R>(ds);
}

template<typename R, typename P1, typename P2>
inline buttonrpc::value_t<R> buttonrpc::call( std::string name, P1 p1, P2 p2 )
{
	Serializer ds;
	ds << name << p1 << p2;
	return net_call<R>(ds);
}

template<typename R, typename P1, typename P2, typename P3>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name, P1 p1, P2 p2, P3 p3)
{
	Serializer ds;
	ds << name << p1 << p2 << p3;
	return net_call<R>(ds);
}

template<typename R, typename P1, typename P2, typename P3, typename P4>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name, P1 p1, P2 p2, P3 p3, P4 p4)
{
	Serializer ds;
	ds << name << p1 << p2 << p3 << p4;
	return net_call<R>(ds);
}

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
inline buttonrpc::value_t<R> buttonrpc::call(std::string name, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
	Serializer ds;
	ds << name << p1 << p2 << p3 << p4 << p5;
	return net_call<R>(ds);
}


#endif