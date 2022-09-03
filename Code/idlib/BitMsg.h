#ifndef __BITMSG_H__
#define __BITMSG_H__

#include <google/protobuf/util/delimited_message_util.h>
#include <google/protobuf/message.h>

class idBitMsg {
public:
	idBitMsg() = default;

	// both read & write
	void InitWrite(boost::asio::streambuf& packetBuffer);

	// read only
	void InitRead(boost::asio::streambuf& packetBuffer, size_t length);

	//------------------------
	// Write State
	//------------------------
	// 

	// save the read state
	void SaveReadState(int& c, int& b) const;

	// restore the read state
	void RestoreReadState(int c, int b);

	//------------------------
	// Reading
	//------------------------
	uint32_t ReadLong();
	uint64_t ReadLongLong();
	bool ReadProtobufMessage(google::protobuf::Message* proto_msg);

	//------------------------
	// Writing
	//------------------------
	void WriteLong(uint32_t c);
	void WriteLongLong(uint64_t c);
	bool WriteProtobufMessage(google::protobuf::Message* proto_msg, int size);

	void Flush();
private:
	std::unique_ptr<std::istream> reader;
	std::unique_ptr<google::protobuf::io::IstreamInputStream> isistream;
	std::unique_ptr<google::protobuf::io::CodedInputStream> coded_input;

	std::unique_ptr<std::ostream> writer;
	std::unique_ptr<google::protobuf::io::OstreamOutputStream> osostream;
	std::unique_ptr<google::protobuf::io::CodedOutputStream> coded_output;
};

/*
========================
idBitMsg::SaveReadState
========================
*/
inline void idBitMsg::SaveReadState(int& c, int& b) const {
	//c = input->CurrentPosition();
}

/*
========================
idBitMsg::RestoreReadState
========================
*/
inline void idBitMsg::RestoreReadState(int c, int b) {
	//input->
}

inline uint32_t idBitMsg::ReadLong() {
	uint32_t val;
	coded_input->ReadVarint32(&val);
	return val;
}

inline uint64_t idBitMsg::ReadLongLong() {
	uint64_t val;
	coded_input->ReadVarint64(&val);
	return val;
}

/*
========================
idBitMsg::WriteLong
========================
*/
inline void idBitMsg::WriteLong(uint32_t c) {
	coded_output->WriteVarint32(c);
}

/*
========================
idBitMsg::WriteLongLong
========================
*/
inline void idBitMsg::WriteLongLong(uint64_t c) {
	coded_output->WriteVarint64(c);
}

#endif