#include <cstdlib>
#include <cstdio>
#include <stdint.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>
#include "cbor-cpp/src/cbor.h"

/* settings section */

/* sensor settings for check humidity program */
#define DHT_PIN	"18"
#define DHT_TYPE "DHT11"

/* path settings */
#define PATH_TO_DHT_PROG "/root/checkHumidity/checkHumidity " DHT_PIN " " DHT_TYPE
#define PATH_TO_LORASEND "/root/lorawan_send"

/* lora settings */
#define LORA_SF (9)
/* ABP device keys here */
#define LORA_DEVICEADDR "REMOVED"
#define LORA_NWSKEY "REMOVED"
#define LORA_APPSKEY "REMOVED"

/* try sending data every 15 seconds */
#define TX_INTERVAL_SECS	(15)

/* output data buffer */
uint8_t dataOutputBuf[256];
size_t dataOutputLen = 0;

/* lora packet counter. we manage this ourselves. */
unsigned long packetCounter = 0;

void hexDump(const char *desc, const void *addr, size_t len) {
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*) addr;

	// Output description if given.
	if (desc != NULL)
		printf("%s:\n", desc);

	if (len == 0) {
		printf("  ZERO LENGTH\n");
		return;
	}

	// Process every byte in the data.
	for (i = 0; i < (int) len; i++) {
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
				printf("  %s\n", buff);

			// Output the offset.
			printf("  %04x ", i);
		}

		// Now the hex code for the specific character.
		printf(" %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	// And print the final ASCII bit.
	printf("  %s\n", buff);
}

constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f' };

std::string hexStr(unsigned char *data, int len) {
	std::string s(len * 2, ' ');
	for (int i = 0; i < len; ++i) {
		s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
		s[2 * i + 1] = hexmap[data[i] & 0x0F];
	}
	return s;
}

void encodeCBOR(int temp, int humidity) {
	cbor::output_dynamic output;
	cbor::encoder encoder(output);
	encoder.write_map(2);
	encoder.write_string("t", 1);
	encoder.write_int(temp);
	encoder.write_string("h", 1);
	encoder.write_int(humidity);

	uint8_t* data = output.data();
	size_t len = output.size();

	if (len <= sizeof(dataOutputBuf)) {
		memcpy(dataOutputBuf, data, len);
		dataOutputLen = len;
	} else {
		printf("Overflow!!\n");
		return;
	}

	hexDump("CBOR encoded data", data, len);

	//only for debug: decode again
#if 1
	cbor::input input(output.data(), output.size());
	cbor::listener_debug listener;
	cbor::decoder decoder(input, listener);
	decoder.run();
#endif
}

bool getTemp(int& tmp, int& humidity) {
	FILE *handle = popen(PATH_TO_DHT_PROG, "r");

	if (handle == NULL) {
		printf("Failed to start program: %s\n", PATH_TO_DHT_PROG);
		return false;
	}
	std::string programOutput = "";
	char buf[64] = {0};
	size_t readn = 0;
	while ((readn = fread(buf, 1, sizeof(buf), handle)) > 0) {
		//fwrite(buf, 1, readn, stdout);
		programOutput.append(buf, readn);
	}
	pclose(handle);

	printf("PROGRAM OUTPUT:\n%s\n", programOutput.c_str());
	hexDump("output", programOutput.c_str(), programOutput.length());

    std::stringstream iss(programOutput);

    int numLine = 0;
    while(iss.good())
    {
        std::string SingleLine;
        std::getline(iss,SingleLine,'\n');

        if(SingleLine.empty())
        	continue;

        printf("Line: %s\n", SingleLine.c_str());

        int val = (int) std::stof(SingleLine.c_str(), nullptr);

        if(numLine == 0)
        	humidity = val;
        else if(numLine == 1)
        	tmp = val;
        numLine++;
    }

	return true;
}

/* sends the output buffer via lora */
bool send_over_lora() {
	std::string hexOutput = hexStr(dataOutputBuf, dataOutputLen);
	char programInvoke[256];

	snprintf(programInvoke, sizeof(programInvoke),
			"%s --payload \"%s\""
			" --method \"ABP\""
			" --dev-adr \"%s\" --nws-key \"%s\""
			" --apps-key \"%s\""
			" --format \"hex\""
			" -s %d"
			" --up-counter %d",
			PATH_TO_LORASEND,
			hexOutput.c_str(),
			LORA_DEVICEADDR,
			LORA_NWSKEY,
			LORA_APPSKEY,
			LORA_SF,
			(int)((uint16_t) packetCounter)
	);
	packetCounter++;

	printf("Invoking now: \"%s\"\n", programInvoke);
	int retCode = std::system(programInvoke);
	printf("Ret Code: %d\n", retCode);

	return retCode == 0;
}

int main() {

	while (true ) {
		int temp = 0, humidity = 0;
		if(getTemp(temp, humidity)) {
		    printf("Temp: %d°C Humidity %d%%\n", temp, humidity);
			encodeCBOR(temp, humidity);
			send_over_lora();
		}
		sleep(TX_INTERVAL_SECS);
	}
	return 0;
}
