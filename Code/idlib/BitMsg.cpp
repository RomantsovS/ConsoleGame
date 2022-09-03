#include "precompiled.h"

void idBitMsg::InitWrite(boost::asio::streambuf& packetBuffer) {
	writer = std::make_unique<std::ostream>(&packetBuffer);
	osostream = std::make_unique<google::protobuf::io::OstreamOutputStream>(writer.get());
	coded_output = std::make_unique<google::protobuf::io::CodedOutputStream>(osostream.get());
}

void idBitMsg::InitRead(boost::asio::streambuf& packetBuffer, size_t length) {
	packetBuffer.commit(length);

	reader = std::make_unique<std::istream>(&packetBuffer);
	isistream = std::make_unique<google::protobuf::io::IstreamInputStream>(reader.get());
	coded_input = std::make_unique<google::protobuf::io::CodedInputStream>(isistream.get());
}

bool idBitMsg::ReadProtobufMessage(google::protobuf::Message* proto_msg) {
	// Read the size.
	size_t size = 0;
	if (!coded_input->ReadVarint64(&size)) {
		idLib::Warning("NET: error read proto size\n");
		return false;
	}
	// Tell the stream not to read beyond that size.
	auto limit = coded_input->PushLimit(size);

	// Parse the message.
	if (!proto_msg->ParseFromCodedStream(coded_input.get())) {
		idLib::Warning("NET: error parse from code stream\n");
		return false;
	}

	if (!coded_input->ConsumedEntireMessage()) {
		idLib::Warning("NET: error ConsumedEntireMessage\n");
		return false;
	}

	// Release the limit.
	coded_input->PopLimit(limit);

	return true;
}

bool idBitMsg::WriteProtobufMessage(google::protobuf::Message* proto_msg, int size) {
	uint8_t* buffer = coded_output->GetDirectBufferForNBytesAndAdvance(size);
	if (buffer != nullptr) {
		// Optimization:  The message fits in one buffer, so use the faster
		// direct-to-array serialization path.
		proto_msg->SerializeWithCachedSizesToArray(buffer);
	}
	else {
		// Slightly-slower path when the message is multiple buffers.
		proto_msg->SerializeWithCachedSizes(coded_output.get());
		if (coded_output->HadError())
			return false;
	}
	return true;
}

void idBitMsg::Flush() {
	coded_output = nullptr;
	osostream = nullptr;
	writer = nullptr;
}