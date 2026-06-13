#ifndef _GPS_H_
#define _GPS_H_

#include <stdint.h>
#include <stdbool.h>

#define GPS_DEFAULT_BAUDRATE    9600

#pragma pack(push, 1)
typedef union {
    uint32_t raw;
    struct {
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        uint8_t padding;
    };
} PackedTime;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    uint32_t raw;
    struct {
        uint8_t  day;
        uint8_t  month;
        uint16_t year;
    };
} PackedDate;
#pragma pack(pop)

extern PackedTime gps_time;
extern PackedDate gps_date;
extern char       gps_latitude[];
extern char       gps_longitude[];
extern char       gps_n_s[];
extern char       gps_e_w[];
extern double     gps_latitude_double;
extern double     gps_longitude_double;
extern char       gps_locator[];
extern double     gps_msl_altitude;
extern double     gps_geoid_separation;
extern char       gps_hdop[];
extern uint8_t    num_sats;
extern uint32_t   gga_frames;

// GPS module models
typedef enum { GPS_MODEL_ATGM336H,  GPS_MODEL_NEO6M, GPS_MODEL_NEOM9N, GPS_MODEL_UNKNOWN } gps_model_type;

// Min and max values for time offset
#define GPS_MIN_TIME_OFFSET -14
#define GPS_MAX_TIME_OFFSET  14

extern int8_t   gps_time_offset;

// Last tiem a frame was received
extern uint32_t last_frame_receive_time;
extern uint32_t gps_invalid_frames;
extern uint32_t gps_fifo_overflow_gps;
extern uint32_t gps_fifo_overflow_comm;

void gps_start_it();
void gps_read();

int  gps_change_module_baudrate(uint32_t baudrate);
void gps_reconfigure_gps_uart(uint32_t baudrate);

#endif
