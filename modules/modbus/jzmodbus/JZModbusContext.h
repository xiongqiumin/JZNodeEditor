#ifndef JZ_MODBUS_CONTEXT_H_
#define JZ_MODBUS_CONTEXT_H_

#include <inttypes.h>
#include <QByteArray>
#include <QVector>
#include "JZModbusDefine.h"

#define MODBUS_BROADCAST_ADDRESS    0

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 1 page 12)
* Quantity of Coils to read (2 bytes): 1 to 2000 (0x7D0)
* (chapter 6 section 11 page 29)
* Quantity of Coils to write (2 bytes): 1 to 1968 (0x7B0)
*/
#define MODBUS_MAX_READ_BITS              2000
#define MODBUS_MAX_WRITE_BITS             1968

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
* Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
* (chapter 6 section 12 page 31)
* Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
* (chapter 6 section 17 page 38)
* Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
*/
#define MODBUS_MAX_READ_REGISTERS          125
#define MODBUS_MAX_WRITE_REGISTERS         123
#define MODBUS_MAX_WR_WRITE_REGISTERS      121
#define MODBUS_MAX_WR_READ_REGISTERS       125

/* The size of the MODBUS PDU is limited by the size constraint inherited from
* the first MODBUS implementation on Serial Line network (max. RS485 ADU = 256
* bytes). Therefore, MODBUS PDU for serial line communication = 256 - Server
* address (1 byte) - CRC (2 bytes) = 253 bytes.
*/
#define MODBUS_MAX_PDU_LENGTH              253

/* Consequently:
* - RTU MODBUS ADU = 253 bytes + Server address (1 byte) + CRC (2 bytes) = 256
*   bytes.
* - TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes.
* so the maximum of both backend in 260 bytes. This size can used to allocate
* an array of bytes to store responses and it will be compatible with the two
* backends.
*/
#define MODBUS_MAX_ADU_LENGTH              260

/* It's not really the minimal length (the real one is report slave ID
* in RTU (4 bytes)) but it's a convenient size to use in RTU or TCP
* communications to read many values or write a single one.
* Maximum between :
* - HEADER_LENGTH_TCP (7) + function (1) + address (2) + number (2)
* - HEADER_LENGTH_RTU (1) + function (1) + address (2) + number (2) + CRC (2)
*/
#define _MIN_REQ_LENGTH 12
#define MAX_MESSAGE_LENGTH 260

/* Random number to avoid errno conflicts */
#define MODBUS_ENOBASE 112345678

/* Protocol exceptions */
enum {
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01,
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE,
    MODBUS_EXCEPTION_ACKNOWLEDGE,
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY,
    MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,
    MODBUS_EXCEPTION_MEMORY_PARITY,
    MODBUS_EXCEPTION_NOT_DEFINED,
    MODBUS_EXCEPTION_GATEWAY_PATH,
    MODBUS_EXCEPTION_GATEWAY_TARGET,
    MODBUS_EXCEPTION_MAX
};

#define EMBXILFUN  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_FUNCTION)
#define EMBXILADD  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS)
#define EMBXILVAL  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE)
#define EMBXSFAIL  (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE)
#define EMBXACK    (MODBUS_ENOBASE + MODBUS_EXCEPTION_ACKNOWLEDGE)
#define EMBXSBUSY  (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY)
#define EMBXNACK   (MODBUS_ENOBASE + MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE)
#define EMBXMEMPAR (MODBUS_ENOBASE + MODBUS_EXCEPTION_MEMORY_PARITY)
#define EMBXGPATH  (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_PATH)
#define EMBXGTAR   (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_TARGET)

/* Native libmodbus error codes */
#define EMBBADCRC  (EMBXGTAR + 1)
#define EMBBADDATA (EMBXGTAR + 2)
#define EMBBADEXC  (EMBXGTAR + 3)
#define EMBUNKEXC  (EMBXGTAR + 4)
#define EMBMDATA   (EMBXGTAR + 5)
#define EMBBADSLAVE (EMBXGTAR + 6)

#define MODBUS_WAIT_RECV 8000

typedef enum {
    _MODBUS_BACKEND_TYPE_RTU = 0,
    _MODBUS_BACKEND_TYPE_TCP
} modbus_backend_type_t;

typedef enum {
    /* Request message on the server side */
    MSG_INDICATION,
    /* Request message on the client side */
    MSG_CONFIRMATION
} msg_type_t;

/* This structure reduces the number of params in functions and so
* optimizes the speed of execution (~ 37%). */
typedef struct _sft {
    int slave;
    int function;
    int t_id;
} sft_t;

class JZModbusContext;
struct modbus_backend_t {
    unsigned int backend_type;
    unsigned int header_length;
    unsigned int checksum_length;
    unsigned int max_adu_length;
    int(*set_slave) (JZModbusContext *ctx, int slave);
    int(*build_request_basis) (JZModbusContext *ctx, int function, int addr,
        int nb, uint8_t *req);
    int(*build_response_basis) (sft_t *sft, uint8_t *rsp);
    int(*prepare_response_tid) (const uint8_t *req, int *req_length);
    int(*send_msg_pre) (uint8_t *req, int req_length);

    int(*check_integrity) (JZModbusContext *ctx, const uint8_t *msg,
        const int msg_length);
    int(*pre_check_confirmation) (JZModbusContext *ctx, const uint8_t *req,
        const uint8_t *rsp, int rsp_length);
};

struct MappingChanged
{
    int addr;
    int nb;
};

class JZModbusContext
{   
public:
    JZModbusContext();
    ~JZModbusContext();

    bool setSlave(int s);
    void setPlcMode(bool flag);

    int buildRequestBasis(int function, int addr, int nb, uint8_t *req);
    int sendMsgPre(uint8_t *req, int req_length);    
    int receiveMessage(msg_type_t msg_type, const QByteArray &res);    
    int checkConfirmation(const uint8_t *req, const uint8_t *rsp, int rsp_length);
    int reply(uint8_t *rsp, const uint8_t *req, int req_length, JZModbusMapping *mb_mapping, MappingChanged &chg_info);
    int response_exception(sft_t *sft,
        int exception_code, uint8_t *rsp,
        unsigned int to_flush,
        const char* temp, ...);

    void readIoStatus(int req_nb,const QByteArray &res,int res_rc, QVector<uint8_t> &dest);
    void readRegisters(const QByteArray &res, int res_rc, QVector<uint16_t> &dest);

    uint8_t compute_meta_length_after_function(int function, msg_type_t msg_type);
    int compute_data_length_after_meta(const uint8_t *msg, msg_type_t msg_type);
    unsigned int compute_response_length_from_request(const uint8_t *req);    
    
    int t_id;
    int slave;
    bool m_plcMode;
    const modbus_backend_t *backend;
};



#endif // !JZ_MODBUS_CLIENT_H_
