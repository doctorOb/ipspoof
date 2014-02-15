#ifndef TAG_PROTOCOL
#define TAG_PROTOCOL

class TagTableEntry {

public:

	TagTableEntry(unsigned long long tag, uint32_t interface);

	unsigned long long GetTag():

	uint32_t GetInterface();

private:

	unsigned long long m_tag;
	uint32_t m_interface;
};



#endif