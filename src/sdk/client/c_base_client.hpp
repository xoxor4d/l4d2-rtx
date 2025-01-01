#pragma once
#define CLIENT_INTERFACE_VERSION "VClient016"

namespace sdk
{
	class d_variant;
	class recv_table;
	class recv_prop;
	class c_recv_proxy_data;

	class c_client_class;
	class i_client_networkable;
	class client_mode;

	typedef i_client_networkable* (*create_client_class_fn)(int ent_number, int serial_number);
	typedef i_client_networkable* (*create_event_fn)();

	using recv_var_proxy_fn = void(*)(const c_recv_proxy_data* data, void* struct_ptr, void* out_ptr);
	using array_length_recv_proxy_fn = void(*)(void* struct_ptr, int object_id, int current_array_length);
	using data_table_recv_var_proxy_fn = void(*)(const recv_prop* prop, void** out_ptr, void* data_ptr, int object_id);

	enum send_prop_type
	{
		_int = 0,
		_float,
		_vec,
		_vec_xy,
		_string,
		_array,
		_data_table,
		_int_64,
	};

	class d_variant
	{
	public:
		union {
			float m_float;
			long m_int;
			char* m_string;
			void* m_data;
			float m_vector[3];
			__int64 m_int64;
		};
		send_prop_type type;
	};

	class c_recv_proxy_data
	{
	public:
		const recv_prop* recv_prop;
		d_variant value;
		int element_index;
		int object_id;
	};

	class recv_prop
	{
	public:
		char* prop_name;
		send_prop_type prop_type;
		int prop_flags;
		int buffer_size;
		int is_inside_of_array;
		const void* extra_data_ptr;
		recv_prop* array_prop;
		array_length_recv_proxy_fn array_length_proxy;
		recv_var_proxy_fn proxy_fn;
		data_table_recv_var_proxy_fn data_table_proxy_fn;
		recv_table* data_table;
		int offset;
		int element_stride;
		int elements_count;
		const char* parent_array_prop_name;
	};

	class recv_table
	{
	public:
		recv_prop* props;
		int props_count;
		void* decoder_ptr;
		char* table_name;
		bool is_initialized;
		bool is_in_main_list;
	};

	class c_client_class
	{
	public:
		create_client_class_fn create_fn;
		create_event_fn create_event_fn;
		char* network_name;
		recv_table* recvtable_ptr;
		c_client_class* next_ptr;
		int class_id;
	};

	class base_client
	{
	public:
		c_client_class* get_client_class();
	};
}
