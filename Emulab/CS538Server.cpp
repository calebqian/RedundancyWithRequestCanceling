//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <list>
#include <queue>
#include <deque>
#include <time.h>
#include <unistd.h>
#define CANCELJOB 1
using boost::asio::ip::tcp;
typedef std::pair<std::list<std::pair<int, std::string> >*, boost::mutex*> ThreadSafeQueue;

#define BUFSIZE 4096
unsigned int maxqlen = 0;

class setConnection
{
public:
	setConnection(){

	}

	bool send_command(std::string command, std::string destIP)
	{
		try
		{
			std::string buffer;
			boost::asio::io_service io_service;

			tcp::resolver resolver(io_service);
			tcp::resolver::query query(destIP, "daytime");
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
			tcp::resolver::iterator end;

			tcp::socket socket(io_service);
			boost::system::error_code error = boost::asio::error::host_not_found;
			while (error && endpoint_iterator != end)
			{
				socket.close();
				socket.connect(*endpoint_iterator++, error);
			}
			if (error)
			{
				throw boost::system::system_error(error);
			}
			boost::asio::socket_base::message_flags flags;
			std::cerr<<"Try to send! Dest Node:" << destIP << std::endl;
			boost::asio::write(socket,boost::asio::buffer(command,command.size()));
			/*socket.send(boost::asio::buffer(command,command.size()), flags ,error);
			if(error){
				std::cerr<< "error..." << std::endl;
				throw boost::system::system_error(error);
			}*/
			socket.close();
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			while(1);
			return false;
		}
		return true;
	}

	void Connect(){
		std::size_t dot = boost::asio::ip::host_name().find_first_of('.');
		std::string myId = boost::asio::ip::host_name().substr(0,dot);
		std::string inBuffer;
		std::ifstream hosts;
		std::size_t dash;
		std::size_t space;
		hosts.open("/etc/hosts");
		if(hosts.is_open()){
			std::getline(hosts, inBuffer);
			std::cout<< "Get first line:" << inBuffer << std::endl;
			inBuffer.clear();
			while(1){
				std::getline(hosts,inBuffer);
				space=inBuffer.find_first_of('	');
				dash =inBuffer.find_first_of('-');
				if(hosts.eof()){
					break;
				}
				if(inBuffer.substr(space+1, dash-space-1) != myId)
				{
					connect2IP(inBuffer.substr(space + 1 , dash - space - 1));
					std::cout<< "next line: " << inBuffer.substr(space + 1, dash - space - 1) << std::endl;
				}
				inBuffer.clear();
			}
		}
		else{
			std::cerr<<"Hosts file not found!!" << std::endl;
		}
	}

	virtual ~setConnection(){

	}

private:
	bool connect2IP(std::string destIP){
		try
		{
			std::string buffer;
			boost::asio::io_service io_service;

			tcp::resolver resolver(io_service);
			tcp::resolver::query query(destIP, "daytime");
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
			tcp::resolver::iterator end;

			tcp::socket* socket = new tcp::socket(io_service);
			boost::system::error_code error = boost::asio::error::host_not_found;
			while (error && endpoint_iterator != end)
			{
				socket->close();
				socket->connect(*endpoint_iterator++, error);
			}
			if (error)
				throw boost::system::system_error(error);
			connectionMap[destIP]= socket;
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			return false;
		}
		return true;
	}
	std::map<std::string,tcp::socket*> connectionMap;
};

typedef struct{
	setConnection* myConnections;
	std::queue<std::pair<int, std::string> >* cancel;
	boost::mutex* mutex;
} CancelStruct;

class tcp_connection
: public boost::enable_shared_from_this<tcp_connection>
{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;

	static pointer create(boost::asio::io_service& io_service)
	{

		return pointer(new tcp_connection(io_service));
	}

	tcp::socket& socket()
	{
		return socket_;
	}
	void read(ThreadSafeQueue* myThreadSafeQueue)
	{
		socket_.async_read_some(boost::asio::buffer(data_, BUFSIZE),
				boost::bind(&tcp_connection::handle_read, shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred, myThreadSafeQueue));
	}
private:
	tcp_connection(boost::asio::io_service& io_service)
	: socket_(io_service)
	{
	}

	void handle_write(const boost::system::error_code& error,
			size_t bytes_transferred)
	{
		/*
		 * Job finish log
		 */
	}
	void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred, ThreadSafeQueue* myThreadSafeQueue)
	{
		try{
			if (!e && bytes_transferred)
			{
				/*				std::string metaData = message_.append(data_.data());
				message_.clear();
				std::string data;
				std::size_t loc1 = -1;
				std::size_t loc2 = metaData.find_first_of('!');
				if(loc2 == std::string::npos){
					std::cerr<<"Wrong data sequence got, need to read more data! Data is: " << metaData << std::endl;
					message_ = metaData;
					//data_.assign('\0');
					return;
				}
				for(data = metaData.substr(loc1 + 1,loc2 - loc1 -1); loc2 != std::string::npos;
						loc1 = loc2, loc2 = metaData.find_first_of('!', loc2 + 1), data = metaData.substr(loc1 + 1, loc2 - loc1 -1))
				{*/
				std::string data = data_.data();
				data[bytes_transferred] = '\0';
				//std::cout<< "Now the string is: "<<data << ", loc1 is :" << loc1 << ", loc2 is :" << loc2 <<std::endl;
				std::list<std::pair<int, std::string> >* queue = myThreadSafeQueue->first;
				boost::mutex *qmtx;
				qmtx = myThreadSafeQueue->second;
				std::cout << "I got a "<< bytes_transferred <<"bytes message: " << data << std::endl;
				int first_pos = data.find_first_of(' ');
				std::string type = data.substr(0, first_pos);
				if(type=="add")
				{
					int second_pos = data.find_first_of(' ', first_pos+1);
					std::string id = data.substr(first_pos+1, second_pos-first_pos);
					int third_pos = data.find_first_of(' ', second_pos+1);
					std::string other_node = data.substr(second_pos+1, third_pos-second_pos);
					int job_id = atoi(id.c_str());
					std::cout<<"I got a add command to add job "<< job_id<<", which is also assigned to "<< other_node <<std::endl;
					qmtx->lock();
					std::pair<int, std::string> another_job(job_id, other_node);
					queue->push_back(another_job);
					if(queue->size() > maxqlen)
					{
						std::size_t dot = boost::asio::ip::host_name().find_first_of('.');
						std::string myId = boost::asio::ip::host_name().substr(0,dot);
						maxqlen = queue->size();
						std::string filename("maxq");
						filename.append(myId);
						filename.append(".txt");
						std::ofstream ofs (filename.c_str(), std::ofstream::out);
						ofs << maxqlen<<std::endl;
						ofs.close();
					}
					qmtx->unlock();
				}
				else if (type=="cancel" && CANCELJOB)
				{
					int second_pos = data.find_first_of(' ', first_pos+1);
					std::string id = data.substr(first_pos+1, second_pos-first_pos);
					int job_id = atoi(id.c_str());
					std::cout<<"I got a cancel command to cancel job "<< job_id <<std::endl;
					std::list<std::pair<int, std::string> >::iterator it;
					qmtx->lock();
					for(it = queue->begin(); it!=queue->end(); it++)
					{
						if(it->first == job_id)
						{
							// job found
							queue->erase(it);
							break;
						}
						else
						{
							// do nothing, keep iterating
						}
					}
					qmtx->unlock();
				}
				else
				{
					std::cerr<< "Command not recognized: " << data<< std::endl;
				}

				/*
				 * Handle data
				 */
				this->read(myThreadSafeQueue);
				/*}
				if(metaData[loc1 + 1] != '\0'){
					message_ = metaData.substr(loc1 + 1);
					std::cout<< "leftover data is: " << message_ << std::endl;
				}*/
				data_.assign('\0');
			}
			else {
				throw boost::system::system_error(e);
			}
		}
		catch(std::exception & e)
		{
			std::cerr<<e.what()<<std::endl;
		}
	}
	void write_msg(std::string msg){
		boost::asio::async_write(socket_, boost::asio::buffer(msg),
				boost::bind(&tcp_connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
	tcp::socket socket_;
	std::string message_;
	boost::array<char, BUFSIZE> data_;
};

class tcp_server
{
public:
	tcp_server(boost::asio::io_service& io_service, ThreadSafeQueue * myThreadSafeQueue)
	: acceptor_(io_service, tcp::endpoint(tcp::v4(), 13)), myThreadSafeQueue(myThreadSafeQueue)
	{
		start_accept();
	}

private:
	void start_accept()
	{
		tcp_connection::pointer new_connection =
				tcp_connection::create(acceptor_.get_io_service());

		acceptor_.async_accept(new_connection->socket(),
				boost::bind(&tcp_server::handle_accept, this, new_connection,
						boost::asio::placeholders::error));
	}

	void handle_accept(tcp_connection::pointer new_connection,
			const boost::system::error_code& error)
	{
		if (!error)
		{
			// start to read messages
			std::cout<< "Get Connection!" << std::endl;
			new_connection->read(myThreadSafeQueue);
		}

		start_accept();
	}

	tcp::acceptor acceptor_;
	ThreadSafeQueue* myThreadSafeQueue;
};

void *listener(void* myThreadSafeQueue){
	try
	{
		boost::asio::io_service io_service;
		tcp_server server(io_service, (ThreadSafeQueue*) myThreadSafeQueue);
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}

void work_your_job()
{
	int duration = rand()%2000+500;
	usleep(duration * 1000);
}

void *cancelSender(void* cancelPara){
	while(1){
		CancelStruct* cancelMain = (CancelStruct*) cancelPara;
		cancelMain->mutex->lock();
		if(!cancelMain->cancel->empty()){
			std::pair<int, std::string> job = cancelMain->cancel->front();
			cancelMain->cancel->pop();
			cancelMain->mutex->unlock();
			std::ostringstream s;
			s << "cancel " << job.first;
			std::string command(s.str());
			/* send cancel command here*/
			std::cout << "Job done! Send cancel Command to :" << job.second << std::endl;
			cancelMain->myConnections->send_command(command, job.second + "-lan1");
		}
		else{
			cancelMain->mutex->unlock();
		}
	}
	return 0;
}

int main()
{
	pthread_t pthread1;
	pthread_t pthread2;
	setConnection myConnections;
	std::string garbage;
	std::string node0("node0");
	std::list<std::pair<int, std::string> > queue;
	boost::mutex qmtx;
	std::queue<std::pair<int,std::string> > cancel;
	boost::mutex cancelMutex;
	CancelStruct cancelPara;
	cancelPara.cancel = &cancel;
	cancelPara.mutex = &cancelMutex;
	cancelPara.myConnections = &myConnections;
	ThreadSafeQueue myThreadSafeQueue(&queue, &qmtx);
	int iret1 = pthread_create( &pthread1, NULL, listener, &myThreadSafeQueue);
	int iret2 = pthread_create( &pthread2, NULL, cancelSender, &cancelPara);
	//std::getline(std::cin,garbage);
	srand(time(NULL));
	//myConnections.Connect();
	while(1){
		qmtx.lock();
		if(queue.size()!=0)
		{
			std::pair<int, std::string> job = queue.front();
			queue.pop_front();
			qmtx.unlock();
			work_your_job();
			std::ostringstream s;
			if(CANCELJOB)
			{
				//s << "cancel " << job.first;
				//std::string command(s.str());
				/* send cancel command here*/
				//std::cout << "Job done! Send cancel Command to :" << job.second << std::endl;
				//myConnections.send_command(command, job.second + "-lan1");
				cancelMutex.lock();
				cancel.push(job);
				std::cout<< "try push cancel info"<< std::endl;
				cancelMutex.unlock();
			}
			std::ostringstream finish;
			finish<<"finish "<< job.first;
			std::string commandFinish(finish.str());
			myConnections.send_command(commandFinish,node0);
		}
		else{
			qmtx.unlock();
		}
	}
	/*
	 * While (1)
	 * 	break if key pressed
	 * Read /etc/hosts
	 * Find dest IP
	 * Try connect to thses IPs
	 *
	 */
	return 0;
}
