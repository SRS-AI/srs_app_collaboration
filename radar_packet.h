#ifndef RADAR_PACKET_H_
#define RADAR_PACKET_H_
#include <cstdint>

#define NETWORK_TX_HEADER_LENGTH           				(4)
#define MAX_BUF_SIZE                					(1920*1080*4)
#define RADAR_CMD_HEADER_LENGTH                         (36U)
#define RADAR_OUTPUT_MAGIC_WORD_LENGTH                  (8U)
#define NETWORK_PACKET_HEADER_LENGTH					(16)
#define MAX_NUM_POINTS_PER_FRAME						(5000)
#define FRAME_SIZE               						(NETWORK_PACKET_HEADER_LENGTH + (MAX_NUM_POINTS_PER_FRAME * 20))   // packet size : 100016 B
#define RADAR_DATA_RX_PORT     							(29172)

static uint8_t NETWORK_TX_HEADER[NETWORK_TX_HEADER_LENGTH] = {0x21, 0x43, 0xcd, 0xab};
static uint8_t radarMagicWord[RADAR_OUTPUT_MAGIC_WORD_LENGTH] = {1, 2, 3, 4, 5, 6, 7, 8};

inline uint32_t GetU32(const uint8_t *a) { return a[0] | ((uint32_t)a[1] << 8) | ((uint32_t)a[2] << 16) | ((uint32_t)a[3] << 24); }

typedef struct 
{
    /**< Number of buffers */
    unsigned int numBuf;

    /**< Header magic number NETWORK_RX_HEADER */
    unsigned int header;

    /**< Payload type NETWORK_RX_TYPE_* */
    unsigned int payloadType;

    /**< channel ID */
    unsigned int chNum;

    /**< Size of payload data in bytes */
    unsigned int dataSize = 0;

    /**< Width of video frame */
    unsigned int width;

    /**< Height of video frame */
    unsigned int height;

    /**< Pitch of video frame in bytes */
    unsigned int pitch[2];
} NetworkRx_CmdHeader;

typedef struct
{
    uint8_t magicWord[RADAR_OUTPUT_MAGIC_WORD_LENGTH];
    uint32_t frame_counter;
    uint32_t targetNumber;

} PACKET_HEADER;

typedef struct
{
    /* radar index */
    uint8_t position; 

    /* packet size */
    uint32_t size;

    /* TI network header */
    NetworkRx_CmdHeader cmdHeader;

    /* 1 frame data */
    union 
    {
        struct
        {
            /* packet header */
            PACKET_HEADER pkHeader;

            /* packet data */
            uint8_t data [(MAX_NUM_POINTS_PER_FRAME * 20)];
        };

        /* packet buffer */
        uint8_t buf[FRAME_SIZE];
    };

    /* buffer used */
    bool used;
} PACKET_BUFFER;

typedef struct 
{
    float x;
    float y;
    float z;
    float doppler;
    uint32_t power;
} POINT_DATA;

#endif
