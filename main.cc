#include <iostream>
#include "tensorflow/lite/c/c_api.h"
#include "radar_socket.h"
#include "tfmodel.h"

#define	TARGET_CPU_CORE1		(3)
#define	TARGET_CPU_CORE2		(2)

int8_t receive(Socket &sock)
{
    PACKET_BUFFER packetBuffer;
	int readBytes = 0;

	uint32_t frame_number, nPoints;
	
    readBytes = sock.readData((uint8_t *)&packetBuffer.cmdHeader, RADAR_CMD_HEADER_LENGTH, true);
    if(readBytes == RADAR_CMD_HEADER_LENGTH) {
        if(memcmp(&packetBuffer.cmdHeader.header, NETWORK_TX_HEADER, NETWORK_TX_HEADER_LENGTH) != 0) {
            std::cerr << "Not match with the TI header magic number !!!" << "\n";
            return 0;
        }

        if(packetBuffer.cmdHeader.dataSize > MAX_BUF_SIZE) {
            std::cerr << "Greater than max buffer size !" << "\n";
            return 0;
        }
    } else {
        std::cerr << "Read bytes(" << readBytes << ") is not matching the data size !!!" << "\n";
		return 0;
    }

    readBytes = sock.readData((uint8_t *)&packetBuffer.buf, FRAME_SIZE, true);
    if (readBytes == FRAME_SIZE)
    {
        if(memcmp(packetBuffer.pkHeader.magicWord, radarMagicWord, RADAR_OUTPUT_MAGIC_WORD_LENGTH) != 0)
        {
            std::cerr << "Magic Word is not matched !!!" << "\n";
            return 0;
        }

        // Frame Number
        frame_number = packetBuffer.pkHeader.frame_counter;
		std::cout << "Frame Counter : " << frame_number << "\n";
			
        // The number of points
        nPoints = packetBuffer.pkHeader.targetNumber;
		std::cout << "Total number of points : " << nPoints << "\n";

        for (uint32_t i = 0; i < nPoints; i++)
        {
            POINT_DATA *point = (POINT_DATA *)(packetBuffer.data + (sizeof(POINT_DATA) * i));

            // just for debugging 
            //std::cout << "[" << i << "] (" << point->x << ", " << point->y << ", " << point->z << "\n";
			//[TODO]
        }
	}

    return 1;
}

int main(int argc, char **argv)
{
	int numTry = 0;
    Socket m_socket;
	std::string ip_addr;

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(TARGET_CPU_CORE1, &mask);
	if( TARGET_CPU_CORE2 >= 0)
	{
		CPU_SET(TARGET_CPU_CORE2, &mask);
	}
	
	if( sched_setaffinity(0, sizeof(mask), &mask) < 0)
	{
		//SRS_LOGE("cannot fix cpu core: 0X%02X", mask);
		return -1;
	}

	ip_addr = "192.168.0.124";
	
	while (!m_socket.connectSocket(ip_addr.c_str(), RADAR_DATA_RX_PORT)) {
		numTry++;
		if (numTry <= 20) { // 20 tries
			usleep(100000);			
		}
		else {
			std::cerr << "Tried 20 times, but couldn't connect. Bye !\n";
			return 0;
		}
	}

	TfLiteModel* model = TfLiteModelCreate(model_tflite, model_tflite_len);
	TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();
	TfLiteInterpreterOptionsSetNumThreads(options, 2);
	TfLiteInterpreter* interpreter = TfLiteInterpreterCreate(model, options);
	TfLiteInterpreterAllocateTensors(interpreter);
	TfLiteTensor* input_tensor = TfLiteInterpreterGetInputTensor(interpreter, 0);

	float input[4] = { 5.1f, 3.5f, 1.4f, 0.2 };

	while (1) {
		if(!receive(m_socket)) {
			std::cerr << "failed to receive the packet data !!!" << "\n";
            continue;
		} 

		TfLiteTensorCopyFromBuffer(input_tensor, input, 4 * sizeof(float));

		TfLiteInterpreterInvoke(interpreter);

		const TfLiteTensor* output_tensor = TfLiteInterpreterGetOutputTensor(interpreter, 0);

        float output[3];
        TfLiteTensorCopyToBuffer(output_tensor, output, 3 * sizeof(float));

		printf("TensorFlow C library active %s %f %f %f\n", TfLiteVersion(), output[0], output[1], output[2]);
        usleep(10000);
	}

	TfLiteInterpreterDelete(interpreter);
	TfLiteInterpreterOptionsDelete(options);
	TfLiteModelDelete(model);
	
}