//
// implementation for managing the route/rule entries over kernel interface.
// REQUIRES ROOT PRIVILEGES
class IpRoute2 {
	public:
		// adds destination (in the form: "ip/mask") as route in the main table or a custom one available on interface 
		// if gateway and destination are valid IP's, route will be added as "via"
		// if gateway is valid IP and destination is nullptr, route will be added as "default via" (for all addresses)
		bool		AddRoute(const char *interface, const char *destination, unsigned int table = 0, const char *gateway = nullptr);
		//
		// removes destination (in the form: "ip/mask") as route in the main table or a custom available on interface 
		// if gateway and destination are valid IP's, route "via" will be removed
		// if gateway is valid IP and destination is nullptr, route "default via" will be removed
		bool		RemoveRoute(const char *interface, const char *destination, unsigned int table = 0, const char *gateway = nullptr);
		//
		// adds a rule with source from and lookup in table
		bool		AddRule(const char *from, unsigned int table);
		//
		// removes a rule with source from and lookup in table
		bool		RemoveRule(const char *from, unsigned int table);
		//
		// deletes a custom table as long as the reference id < 256:
		bool		DeleteRuleTable(unsigned int table);
		//
		// extract the id of table from the routing file; -1 if table is not found
		static int GetIdFromTableName(const char *table);
		
	private:
		struct 		Request;
		
		int 		addattr_l(Request *request, int type, void *data, int alen);
		void 		add_source(Request *request, const char *destination);
		void 		add_destination(Request *request, const char *destination);
		void 		add_gateway(Request *request, const char *gateway);
		void 		add_oif(Request *request, const char *interface);
		bool 		manage_route(int action, const char *interface, const char *destination, unsigned int table, const char *gateway);
		bool 		manage_rule(int action, const char *from, unsigned int table);
		
		static int 	open_socket();
		static void	close_socket(int descriptor);
		static int	send_request(const Request *request, int descriptor);		
};
