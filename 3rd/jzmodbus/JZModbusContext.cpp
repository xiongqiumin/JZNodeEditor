#include "JZModbusContext.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef OFF
#define OFF 0
#endif

#ifndef ON
#define ON 1
#endif

#define MSG_LENGTH_UNDEFINED -1

#define _REPORT_SLAVE_ID 180
#define LIBMODBUS_VERSION_STRING "3.1.4"

/* 3 steps are used to parse the query */
typedef enum {
    _STEP_FUNCTION,
    _STEP_META,
    _STEP_DATA
} _step_t;


static int response_io_status(uint8_t *tab_io_status,
    int address, int nb,
    uint8_t *rsp, int offset)
{
    int shift = 0;
    /* Instead of byte (not allowed in Win32) */
    int one_byte = 0;
    int i;

    for (i = address; i < address + nb; i++) {
        one_byte |= tab_io_status[i] << shift;
        if (shift == 7) {
            /* Byte is full */
            rsp[offset++] = one_byte;
            one_byte = shift = 0;
        }
        else {
            shift++;
        }
    }

    if (shift != 0)
        rsp[offset++] = one_byte;

    return offset;
}

/* Sets many bits from a table of bytes (only the bits between idx and
idx + nb_bits are set) */
static void modbus_set_bits_from_bytes(uint8_t *dest, int idx, unsigned int nb_bits,
    const uint8_t *tab_byte)
{
    unsigned int i;
    int shift = 0;

    for (i = idx; i < idx + nb_bits; i++) {
        dest[i] = tab_byte[(i - idx) / 8] & (1 << shift) ? 1 : 0;
        /* gcc doesn't like: shift = (++shift) % 8; */
        shift++;
        shift %= 8;
    }
}

//JZModbusContext
JZModbusContext::JZModbusContext()
{
    t_id = 0;
    backend = 0;
    slave = -1;
    m_plcMode = false;
}

JZModbusContext::~JZModbusContext()
{
}

bool JZModbusContext::setSlave(int s)
{
    if (backend)
        return (backend->set_slave(this, s) == 0);
    else
    {
        this->slave = s;
        return true;
    }
}

void JZModbusContext::setPlcMode(bool flag)
{
    m_plcMode = flag;
}

int JZModbusContext::buildRequestBasis(int function, int addr, int nb, uint8_t *req)
{
    return backend->build_request_basis(this, function, addr, nb, req);
}

int JZModbusContext::sendMsgPre(uint8_t *req, int req_length)
{
    return backend->send_msg_pre(req, req_length);
}

int JZModbusContext::receiveMessage(msg_type_t msg_type, const QByteArray &res)
{
    int rc;
    int length_to_read;
    int msg_length = 0;
    _step_t step;

    const uint8_t *msg = (const uint8_t *)res.data();
    /* We need to analyse the message step by step.  At the first step, we want
    * to reach the function code because all packets contain this
    * information. */
    step = _STEP_FUNCTION;
    length_to_read = backend->header_length + 1;

    while (length_to_read != 0) {
        if (msg_length + length_to_read > res.size())
            return MODBUS_WAIT_RECV;

        int rc = length_to_read;
        /* Sums bytes received */
        msg_length += rc;
        /* Computes remaining bytes */
        length_to_read -= rc;

        if (length_to_read == 0) {
            switch (step) {
            case _STEP_FUNCTION:
                /* Function code position */
                length_to_read = compute_meta_length_after_function(
                    msg[backend->header_length],
                    msg_type);
                if (length_to_read != 0) {
                    step = _STEP_META;
                    break;
                } /* else switches straight to the next step */
            case _STEP_META:
                length_to_read = compute_data_length_after_meta(msg, msg_type);
                if ((msg_length + length_to_read) > (int)backend->max_adu_length) {
                    errno = EMBBADDATA;                    
                    return -1;
                }
                step = _STEP_DATA;
                break;
            default:
                break;
            }
        }
    }

    return backend->check_integrity(this, msg, msg_length);
}

int JZModbusContext::checkConfirmation(const uint8_t *req,const uint8_t *rsp, int rsp_length)
{
    int rc;
    int rsp_length_computed;
    const int offset = backend->header_length;
    const int function = rsp[offset];

    if (backend->pre_check_confirmation) {
        rc = backend->pre_check_confirmation(this, req, rsp, rsp_length);
        if (rc == -1) {
            return -1;
        }
    }

    rsp_length_computed = compute_response_length_from_request(req);

    /* Exception code */
    if (function >= 0x80) {
        if (rsp_length == (offset + 2 + (int)backend->checksum_length) &&
            req[offset] == (rsp[offset] - 0x80)) {
            /* Valid exception code received */

            int exception_code = rsp[offset + 1];
            if (exception_code < MODBUS_EXCEPTION_MAX) {
                errno = MODBUS_ENOBASE + exception_code;
            }
            else {
                errno = EMBBADEXC;
            }            
            return -1;
        }
        else {
            errno = EMBBADEXC;            
            return -1;
        }
    }

    /* Check length */
    if ((rsp_length == rsp_length_computed ||
        rsp_length_computed == MSG_LENGTH_UNDEFINED) &&
        function < 0x80) {
        int req_nb_value;
        int rsp_nb_value;

        /* Check function code */
        if (function != req[offset]) {            
            errno = EMBBADDATA;
            return -1;
        }

        /* Check the number of values is corresponding to the request */
        switch (function) {
        case MODBUS_FC_READ_COILS:
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            /* Read functions, 8 values in a byte (nb
            * of values in the request and byte count in
            * the response. */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            req_nb_value = (req_nb_value / 8) + ((req_nb_value % 8) ? 1 : 0);
            rsp_nb_value = rsp[offset + 1];
            break;
        case MODBUS_FC_WRITE_AND_READ_REGISTERS:
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_READ_INPUT_REGISTERS:
            /* Read functions 1 value = 2 bytes */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            rsp_nb_value = (rsp[offset + 1] / 2);
            break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            /* N Write functions */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            rsp_nb_value = (rsp[offset + 3] << 8) | rsp[offset + 4];
            break;
        case MODBUS_FC_REPORT_SLAVE_ID:
            /* Report slave ID (bytes received) */
            req_nb_value = rsp_nb_value = rsp[offset + 1];
            break;
        default:
            /* 1 Write functions & others */
            req_nb_value = rsp_nb_value = 1;
        }

        if (req_nb_value == rsp_nb_value) {
            rc = rsp_nb_value;
        }
        else {
            errno = EMBBADDATA;
            rc = -1;
        }
    }
    else {
        errno = EMBBADDATA;
        rc = -1;
    }

    return rc;
}

void JZModbusContext::readIoStatus(int req_nb, const QByteArray &res,int res_rc, QVector<uint8_t> &dest)
{    
    const uint8_t *rsp = (const uint8_t *)res.data();
    int rc = res_rc; 
    dest.clear();

    int temp, bit;
    int pos = 0;
    int offset = backend->header_length + 2;    
    int offset_end = offset + rc;    
    int nb = req_nb;
    for (int i = offset; i < offset_end; i++) {
        /* Shift reg hi_byte to temp */
        temp = rsp[i];

        for (bit = 0x01; (bit & 0xff) && (pos < nb);) {
            dest.push_back((temp & bit) ? true : false);
            pos++;
            bit = bit << 1;
        }
    }
}

void JZModbusContext::readRegisters(const QByteArray &res, int res_rc, QVector<uint16_t> &dest)
{
    const uint8_t *rsp = (const uint8_t *)res.data();
    int rc = res_rc;
    dest.resize(rc);

    int offset = backend->header_length;
    for (int i = 0; i < rc; i++) {
        /* shift reg hi_byte to temp OR with lo_byte */
        dest[i] = (rsp[offset + 2 + (i << 1)] << 8) |
            rsp[offset + 3 + (i << 1)];
    }
}

/* Send a response to the received request.
Analyses the request and constructs a response.

If an error occurs, this function construct the response
accordingly.
*/
int JZModbusContext::reply(uint8_t *rsp,const uint8_t *req, int req_length, JZModbusMapping *mb_mapping, MappingChanged &chg_info)
{
    int offset;
    int slave;
    int function;
    uint16_t address;    
    int rsp_length = 0;
    sft_t sft;

    chg_info.addr = -1;
    chg_info.nb = 0;

    offset = backend->header_length;
    slave = req[offset - 1];
    function = req[offset];
    address = (req[offset + 1] << 8) + req[offset + 2];

    sft.slave = slave;
    sft.function = function;
    sft.t_id = backend->prepare_response_tid(req, &req_length);

    /* Data are flushed on illegal number of values errors. */
    switch (function) {
    case MODBUS_FC_READ_COILS:
    case MODBUS_FC_READ_DISCRETE_INPUTS: {
        unsigned int is_input = (function == MODBUS_FC_READ_DISCRETE_INPUTS);
        int start_bits = is_input ? mb_mapping->start_input_bits : mb_mapping->start_bits;
        int nb_bits = is_input ? mb_mapping->nb_input_bits : mb_mapping->nb_bits;
        uint8_t *tab_bits = is_input ? mb_mapping->tab_input_bits : mb_mapping->tab_bits;
        const char * const name = is_input ? "read_input_bits" : "read_bits";
        int nb = (req[offset + 3] << 8) + req[offset + 4];
        /* The mapping can be shifted to reduce memory consumption and it
        doesn't always start at address zero. */
        int mapping_address = address - start_bits;

        if (nb < 1 || MODBUS_MAX_READ_BITS < nb) {
            rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp, TRUE,
                "Illegal nb of values %d in %s (max %d)\n",
                nb, name, MODBUS_MAX_READ_BITS);
        }
        else if (mapping_address < 0 || (mapping_address + nb) > nb_bits) {
            rsp_length = response_exception(&sft,MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp, FALSE,
                "Illegal data address 0x%0X in %s\n",
                mapping_address < 0 ? address : address + nb, name);
        }
        else {
            rsp_length = backend->build_response_basis(&sft, rsp);
            rsp[rsp_length++] = (nb / 8) + ((nb % 8) ? 1 : 0);
            rsp_length = response_io_status(tab_bits, mapping_address, nb,
                rsp, rsp_length);
        }
    }
                                         break;
    case MODBUS_FC_READ_HOLDING_REGISTERS:
    case MODBUS_FC_READ_INPUT_REGISTERS: {
        unsigned int is_input = (function == MODBUS_FC_READ_INPUT_REGISTERS);
        int start_registers = is_input ? mb_mapping->start_input_registers : mb_mapping->start_registers;
        int nb_registers = is_input ? mb_mapping->nb_input_registers : mb_mapping->nb_registers;
        uint16_t *tab_registers = is_input ? mb_mapping->tab_input_registers : mb_mapping->tab_registers;
        const char * const name = is_input ? "read_input_registers" : "read_registers";
        int nb = (req[offset + 3] << 8) + req[offset + 4];
        /* The mapping can be shifted to reduce memory consumption and it
        doesn't always start at address zero. */
        int mapping_address = address - start_registers;

        if (nb < 1 || MODBUS_MAX_READ_REGISTERS < nb) {
            rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp, TRUE,
                "Illegal nb of values %d in %s (max %d)\n",
                nb, name, MODBUS_MAX_READ_REGISTERS);
        }
        else if (mapping_address < 0 || (mapping_address + nb) > nb_registers) {
            rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp, FALSE,
                "Illegal data address 0x%0X in %s\n",
                mapping_address < 0 ? address : address + nb, name);
        }
        else {
            int i;

            rsp_length = backend->build_response_basis(&sft, rsp);
            rsp[rsp_length++] = nb << 1;
            for (i = mapping_address; i < mapping_address + nb; i++) {
                rsp[rsp_length++] = tab_registers[i] >> 8;
                rsp[rsp_length++] = tab_registers[i] & 0xFF;
            }
        }
    }
                                         break;
    case MODBUS_FC_WRITE_SINGLE_COIL: {
        int mapping_address = address - mb_mapping->start_bits;

        if (mapping_address < 0 || mapping_address >= mb_mapping->nb_bits) {
            rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp, FALSE,
                "Illegal data address 0x%0X in write_bit\n",
                address);
        }
        else {
            int data = (req[offset + 3] << 8) + req[offset + 4];

            if (data == 0xFF00 || data == 0x0) {
                mb_mapping->tab_bits[mapping_address] = data ? ON : OFF;
                memcpy(rsp, req, req_length);
                rsp_length = req_length;

                chg_info.addr = address;
                chg_info.nb = 1;
            }
            else {
                rsp_length = response_exception(&sft,
                    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp, FALSE,
                    "Illegal data value 0x%0X in write_bit request at address %0X\n",
                    data, address);
            }
        }
    }
                                      break;
    case MODBUS_FC_WRITE_SINGLE_REGISTER: {
        int mapping_address = address - mb_mapping->start_registers;

        if (mapping_address < 0 || mapping_address >= mb_mapping->nb_registers) {
            rsp_length = response_exception(&sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp, FALSE,
                "Illegal data address 0x%0X in write_register\n",
                address);
        }
        else {
            int data = (req[offset + 3] << 8) + req[offset + 4];

            mb_mapping->tab_registers[mapping_address] = data;
            memcpy(rsp, req, req_length);
            rsp_length = req_length;

            chg_info.addr = address;
            chg_info.nb = 1;
        }
    }
                                          break;
    case MODBUS_FC_WRITE_MULTIPLE_COILS: {
        int nb = (req[offset + 3] << 8) + req[offset + 4];
        int mapping_address = address - mb_mapping->start_bits;

        if (nb < 1 || MODBUS_MAX_WRITE_BITS < nb) {
            /* May be the indication has been truncated on reading because of
            * invalid address (eg. nb is 0 but the request contains values to
            * write) so it's necessary to flush. */
            rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp, TRUE,
                "Illegal number of values %d in write_bits (max %d)\n",
                nb, MODBUS_MAX_WRITE_BITS);
        }
        else if (mapping_address < 0 ||
            (mapping_address + nb) > mb_mapping->nb_bits) {
            rsp_length = response_exception(&sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp, FALSE,
                "Illegal data address 0x%0X in write_bits\n",
                mapping_address < 0 ? address : address + nb);
        }
        else {
            /* 6 = byte count */
            modbus_set_bits_from_bytes(mb_mapping->tab_bits, mapping_address, nb,
                &req[offset + 6]);

            rsp_length = backend->build_response_basis(&sft, rsp);
            /* 4 to copy the bit address (2) and the quantity of bits */
            memcpy(rsp + rsp_length, req + rsp_length, 4);
            rsp_length += 4;

            chg_info.addr = address;
            chg_info.nb = nb;
        }
    }
                                         break;
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS: {
        int nb = (req[offset + 3] << 8) + req[offset + 4];
        int mapping_address = address - mb_mapping->start_registers;

        if (nb < 1 || MODBUS_MAX_WRITE_REGISTERS < nb) {
            rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp, TRUE,
                "Illegal number of values %d in write_registers (max %d)\n",
                nb, MODBUS_MAX_WRITE_REGISTERS);
        }
        else if (mapping_address < 0 ||
            (mapping_address + nb) > mb_mapping->nb_registers) {
            rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp, FALSE,
                "Illegal data address 0x%0X in write_registers\n",
                mapping_address < 0 ? address : address + nb);
        }
        else {
            int i, j;
            for (i = mapping_address, j = 6; i < mapping_address + nb; i++, j += 2) {
                /* 6 and 7 = first value */
                mb_mapping->tab_registers[i] =
                    (req[offset + j] << 8) + req[offset + j + 1];
            }

            rsp_length = backend->build_response_basis(&sft, rsp);
            /* 4 to copy the address (2) and the no. of registers */
            memcpy(rsp + rsp_length, req + rsp_length, 4);
            rsp_length += 4;

            chg_info.addr = address;
            chg_info.nb = nb;
        }
    }
                                             break;
    case MODBUS_FC_REPORT_SLAVE_ID: {
        int str_len;
        int byte_count_pos;

        rsp_length = backend->build_response_basis(&sft, rsp);
        /* Skip byte count for now */
        byte_count_pos = rsp_length++;
        rsp[rsp_length++] = _REPORT_SLAVE_ID;
        /* Run indicator status to ON */
        rsp[rsp_length++] = 0xFF;
        /* LMB + length of LIBMODBUS_VERSION_STRING */
        str_len = 3 + strlen(LIBMODBUS_VERSION_STRING);
        memcpy(rsp + rsp_length, "LMB" LIBMODBUS_VERSION_STRING, str_len);
        rsp_length += str_len;
        rsp[byte_count_pos] = rsp_length - byte_count_pos - 1;
    }
                                    break;
    case MODBUS_FC_READ_EXCEPTION_STATUS:        
        errno = ENOPROTOOPT;
        return -1;
        break;
    case MODBUS_FC_MASK_WRITE_REGISTER: {
        int mapping_address = address - mb_mapping->start_registers;

        if (mapping_address < 0 || mapping_address >= mb_mapping->nb_registers) {
            rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp, FALSE,
                "Illegal data address 0x%0X in write_register\n",
                address);
        }
        else {
            uint16_t data = mb_mapping->tab_registers[mapping_address];
            uint16_t and_i = (req[offset + 3] << 8) + req[offset + 4];
            uint16_t or_i = (req[offset + 5] << 8) + req[offset + 6];

            data = (data & and_i) | (or_i &(~and_i));
            mb_mapping->tab_registers[mapping_address] = data;
            memcpy(rsp, req, req_length);
            rsp_length = req_length;
        }
    }
                                        break;
    case MODBUS_FC_WRITE_AND_READ_REGISTERS: {
        int nb = (req[offset + 3] << 8) + req[offset + 4];
        uint16_t address_write = (req[offset + 5] << 8) + req[offset + 6];
        int nb_write = (req[offset + 7] << 8) + req[offset + 8];
        int nb_write_bytes = req[offset + 9];
        int mapping_address = address - mb_mapping->start_registers;
        int mapping_address_write = address_write - mb_mapping->start_registers;

        if (nb_write < 1 || MODBUS_MAX_WR_WRITE_REGISTERS < nb_write ||
            nb < 1 || MODBUS_MAX_WR_READ_REGISTERS < nb ||
            nb_write_bytes != nb_write * 2) {
            rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp, TRUE,
                "Illegal nb of values (W%d, R%d) in write_and_read_registers (max W%d, R%d)\n",
                nb_write, nb, MODBUS_MAX_WR_WRITE_REGISTERS, MODBUS_MAX_WR_READ_REGISTERS);
        }
        else if (mapping_address < 0 ||
            (mapping_address + nb) > mb_mapping->nb_registers ||
            mapping_address < 0 ||
            (mapping_address_write + nb_write) > mb_mapping->nb_registers) {
            rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp, FALSE,
                "Illegal data read address 0x%0X or write address 0x%0X write_and_read_registers\n",
                mapping_address < 0 ? address : address + nb,
                mapping_address_write < 0 ? address_write : address_write + nb_write);
        }
        else {
            int i, j;
            rsp_length = backend->build_response_basis(&sft, rsp);
            rsp[rsp_length++] = nb << 1;

            /* Write first.
            10 and 11 are the offset of the first values to write */
            for (i = mapping_address_write, j = 10;
                i < mapping_address_write + nb_write; i++, j += 2) {
                mb_mapping->tab_registers[i] =
                    (req[offset + j] << 8) + req[offset + j + 1];
            }

            /* and read the data for the response */
            for (i = mapping_address; i < mapping_address + nb; i++) {
                rsp[rsp_length++] = mb_mapping->tab_registers[i] >> 8;
                rsp[rsp_length++] = mb_mapping->tab_registers[i] & 0xFF;
            }
        }
    }
                                             break;

    default:
        rsp_length = response_exception(&sft, MODBUS_EXCEPTION_ILLEGAL_FUNCTION, rsp, TRUE,
            "Unknown Modbus function code: 0x%0X\n", function);
        break;
    }

    /* Suppress any responses when the request was a broadcast */
    return (backend->backend_type == _MODBUS_BACKEND_TYPE_RTU &&
        slave == MODBUS_BROADCAST_ADDRESS) ? 0 : rsp_length;
}

/* Build the exception response */
int JZModbusContext::response_exception(sft_t *sft,
    int exception_code, uint8_t *rsp,
    unsigned int to_flush,
    const char* temp, ...)
{
    int rsp_length;

    /* Build exception response */
    sft->function = sft->function + 0x80;
    rsp_length = backend->build_response_basis(sft, rsp);
    rsp[rsp_length++] = exception_code;

    return rsp_length;
}

/*
*  ---------- Request     Indication ----------
*  | Client | ---------------------->| Server |
*  ---------- Confirmation  Response ----------
*/

/* Computes the length to read after the function received */

//JZModbusContext
uint8_t JZModbusContext::compute_meta_length_after_function(int function,
    msg_type_t msg_type)
{
    int length;

    if (msg_type == MSG_INDICATION) {
        if (function <= MODBUS_FC_WRITE_SINGLE_REGISTER) {
            length = 4;
        }
        else if (function == MODBUS_FC_WRITE_MULTIPLE_COILS ||
            function == MODBUS_FC_WRITE_MULTIPLE_REGISTERS) {
            length = 5;
        }
        else if (function == MODBUS_FC_MASK_WRITE_REGISTER) {
            length = 6;
        }
        else if (function == MODBUS_FC_WRITE_AND_READ_REGISTERS) {
            length = 9;
        }
        else {
            /* MODBUS_FC_READ_EXCEPTION_STATUS, MODBUS_FC_REPORT_SLAVE_ID */
            length = 0;
        }
    }
    else {
        /* MSG_CONFIRMATION */
        switch (function) {
        case MODBUS_FC_WRITE_SINGLE_COIL:
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            length = 4;
            break;
        case MODBUS_FC_MASK_WRITE_REGISTER:
            length = 6;
            break;
        default:
            length = 1;
        }
    }

    return length;
}

/* Computes the length to read after the meta information (address, count, etc) */
int JZModbusContext::compute_data_length_after_meta(const uint8_t *msg,
    msg_type_t msg_type)
{
    int function = msg[backend->header_length];
    int length;

    if (msg_type == MSG_INDICATION) {
        switch (function) {
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            length = msg[backend->header_length + 5];
            break;
        case MODBUS_FC_WRITE_AND_READ_REGISTERS:
            length = msg[backend->header_length + 9];
            break;
        default:
            length = 0;
        }
    }
    else {
        /* MSG_CONFIRMATION */
        if (function <= MODBUS_FC_READ_INPUT_REGISTERS ||
            function == MODBUS_FC_REPORT_SLAVE_ID ||
            function == MODBUS_FC_WRITE_AND_READ_REGISTERS) {
            length = msg[backend->header_length + 1];
        }
        else {
            length = 0;
        }
    }

    length += backend->checksum_length;

    return length;
}

/* Computes the length of the expected response */
unsigned int JZModbusContext::compute_response_length_from_request(const uint8_t *req)
{
    int length;
    const int offset = backend->header_length;

    switch (req[offset]) {
    case MODBUS_FC_READ_COILS:
    case MODBUS_FC_READ_DISCRETE_INPUTS: {
        /* Header + nb values (code from write_bits) */
        int nb = (req[offset + 3] << 8) | req[offset + 4];
        length = 2 + (nb / 8) + ((nb % 8) ? 1 : 0);
    }
                                         break;
    case MODBUS_FC_WRITE_AND_READ_REGISTERS:
    case MODBUS_FC_READ_HOLDING_REGISTERS:
    case MODBUS_FC_READ_INPUT_REGISTERS:
        /* Header + 2 * nb values */
        length = 2 + 2 * (req[offset + 3] << 8 | req[offset + 4]);
        break;
    case MODBUS_FC_READ_EXCEPTION_STATUS:
        length = 3;
        break;
    case MODBUS_FC_REPORT_SLAVE_ID:
        /* The response is device specific (the header provides the
        length) */
        return MSG_LENGTH_UNDEFINED;
    case MODBUS_FC_MASK_WRITE_REGISTER:
        length = 7;
        break;
    default:
        length = 5;
    }

    return offset + length + backend->checksum_length;
}