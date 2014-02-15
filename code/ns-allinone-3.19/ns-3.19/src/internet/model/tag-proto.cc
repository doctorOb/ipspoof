#include "tag-proto.h"



TagTableEntry::TagTableEntry(unsigned long long tag, uint32_t interface) {
	m_tag = tag;
	m_interface = interface;
}

unsigned long long
TagTableEntry::GetTag(void) {
	return m_tag;
}

uint32_t
TagTableEntry::GetInterface(void) {
	return m_interface;
}