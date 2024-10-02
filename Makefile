CC=gcc
CFLAGS=-Wall
LIBS=-ljson-c  # Add this line to link the json-c library

# Default file to store extended functionality macros
MACRO_FILE=extended_functionality_macros_user.h

all: config torudo

config:
	@echo "Writing selected features to $(MACRO_FILE)..."
	@echo "/* Automatically generated macros for extended functionality */" > $(MACRO_FILE)
	@echo "#ifndef EXTENDED_FUNCTIONALITY_MACROS_USER_H" >> $(MACRO_FILE)
	@echo "#define EXTENDED_FUNCTIONALITY_MACROS_USER_H" >> $(MACRO_FILE)

	@echo "1. Verbose Logging (y/n): "; \
	read verbose; \
	if [ "$$verbose" = "y" ]; then \
		echo "#define ENABLE_VERBOSE_LOGGING 1" >> $(MACRO_FILE); \
	else \
		echo "#define ENABLE_VERBOSE_LOGGING 0" >> $(MACRO_FILE); \
	fi;

	@echo "2. Identity Manager (y/n): "; \
	read id_manager; \
	if [ "$$id_manager" = "y" ]; then \
		echo "#define ENABLE_IDENTITY_MANAGER 1" >> $(MACRO_FILE); \
	else \
		echo "#define ENABLE_IDENTITY_MANAGER 0" >> $(MACRO_FILE); \
	fi;

	@echo "3. Command Sequence (y/n): "; \
	read cmd_sequence; \
	if [ "$$cmd_sequence" = "y" ]; then \
		echo "#define ENABLE_COMMAND_SEQUENCE 1" >> $(MACRO_FILE); \
	else \
		echo "#define ENABLE_COMMAND_SEQUENCE 0" >> $(MACRO_FILE); \
	fi;

	@echo "4. Geolocation Filter (y/n): "; \
	read geo_filter; \
	if [ "$$geo_filter" = "y" ]; then \
		echo "#define ENABLE_GEOLOCATION_FILTER 1" >> $(MACRO_FILE); \
	else \
		echo "#define ENABLE_GEOLOCATION_FILTER 0" >> $(MACRO_FILE); \
	fi;

	@echo "5. Tor Monitor (y/n): "; \
	read tor_monitor; \
	if [ "$$tor_monitor" = "y" ]; then \
		echo "#define ENABLE_TOR_MONITOR 1" >> $(MACRO_FILE); \
	else \
		echo "#define ENABLE_TOR_MONITOR 0" >> $(MACRO_FILE); \
	fi;

	@echo "6. Circuit Assessment (y/n): "; \
	read circuit_assessment; \
	if [ "$$circuit_assessment" = "y" ]; then \
		echo "#define ENABLE_CIRCUIT_ASSESSMENT 1" >> $(MACRO_FILE); \
	else \
		echo "#define ENABLE_CIRCUIT_ASSESSMENT 0" >> $(MACRO_FILE); \
	fi;

	@echo "7. Custom Routing (y/n): "; \
	read custom_routing; \
	if [ "$$custom_routing" = "y" ]; then \
		echo "#define ENABLE_CUSTOM_ROUTING 1" >> $(MACRO_FILE); \
	else \
		echo "#define ENABLE_CUSTOM_ROUTING 0" >> $(MACRO_FILE); \
	fi;

	@echo "8. Packet Capturer (y/n): "; \
	read packet_capturer; \
	if [ "$$packet_capturer" = "y" ]; then \
		echo "#define ENABLE_PACKET_CAPTURER 1" >> $(MACRO_FILE); \
	else \
		echo "#define ENABLE_PACKET_CAPTURER 0" >> $(MACRO_FILE); \
	fi;

	@echo "#endif /* EXTENDED_FUNCTIONALITY_MACROS_USER_H */" >> $(MACRO_FILE)
	@echo "Configuration completed. Selected features have been written to $(MACRO_FILE)."

torudo: $(MACRO_FILE)
	$(CC) torudo.c -o torudo $(CFLAGS) $(LIBS)  # Link with json-c

clean:
	rm -f torudo
	rm -f $(MACRO_FILE)
