#include "types.hpp"

class cl_packet {
	private:
		t_index m_index;
		t_tag m_tag;
		t_way m_way;

	public:
		void traceReader();
		void parse(std::string l_transactionStr);
};