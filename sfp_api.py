import ctypes
import os
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from typing import List, Optional
from pydantic import BaseModel

# Library Load
# Assuming libsfp.so is in the parent directory
LIB_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "libsfp.so"))
if not os.path.exists(LIB_PATH):
    raise RuntimeError(f"Shared library {LIB_PATH} not found. Run 'make lib' in the root directory first.")

libsfp = ctypes.CDLL(LIB_PATH)

# Constants
SFP_A0_BASE_SIZE = 64
SFP_I2C_ADDR_A0 = 0x50

# C Types Definitions (Matching structures in sfp_8472.h)
class SfpA0hBase(ctypes.Structure):
    _fields_ = [
        ("identifier", ctypes.c_int),
        ("ext_identifier", ctypes.c_uint8),
        ("connector", ctypes.c_uint8),
        ("encoding", ctypes.c_int),
        ("nominal_rate", ctypes.c_uint8),
        ("rate_identifier", ctypes.c_uint8),
        ("smf_length_m", ctypes.c_uint16),
        ("smf_status", ctypes.c_int),
        ("om2_length_m", ctypes.c_uint16),
        ("om2_status", ctypes.c_int),
        ("om1_length_m", ctypes.c_uint16),
        ("om1_status", ctypes.c_int),
        ("om4_or_copper_length_m", ctypes.c_uint16),
        ("om4_or_copper_status", ctypes.c_int),
        ("vendor_name", ctypes.c_char * 17),
        ("ext_compliance", ctypes.c_int),
        ("vendor_oui", ctypes.c_uint8 * 3),
        ("vendor_pn", ctypes.c_char * 17),
        ("vendor_rev", ctypes.c_char * 5),
        # Union media_info omitted for simplicity if not used immediately, 
        # or can be added as bytes if needed.
        ("dummy_media_info", ctypes.c_uint8 * 2), 
        ("fc_speed2", ctypes.c_uint8),
        ("cc_base", ctypes.c_uint8),
        ("cc_base_is_valid", ctypes.c_bool),
    ]

# Function Signatures
libsfp.sfp_i2c_init.argtypes = [ctypes.c_char_p]
libsfp.sfp_i2c_init.restype = ctypes.c_int

libsfp.sfp_i2c_close.argtypes = [ctypes.c_int]
libsfp.sfp_i2c_close.restype = None

libsfp.sfp_read_block.argtypes = [ctypes.c_int, ctypes.c_uint8, ctypes.c_uint8, ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t]
libsfp.sfp_read_block.restype = ctypes.c_bool

libsfp.sfp_parse_a0_base_identifier.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(SfpA0hBase)]
libsfp.sfp_parse_a0_base_ext_identifier.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(SfpA0hBase)]
libsfp.sfp_parse_a0_base_connector.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(SfpA0hBase)]
libsfp.sfp_parse_a0_base_encoding.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(SfpA0hBase)]
libsfp.sfp_parse_a0_base_smf.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(SfpA0hBase)]
libsfp.sfp_parse_a0_base_om1.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(SfpA0hBase)]
libsfp.sfp_parse_a0_base_om2.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(SfpA0hBase)]
libsfp.sfp_parse_a0_base_om4_or_copper.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(SfpA0hBase)]
libsfp.sfp_parse_a0_base_ext_compliance.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(SfpA0hBase)]
libsfp.sfp_parse_a0_base_cc_base.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(SfpA0hBase)]

# FastAPI App
app = FastAPI(title="SFP Interface API")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

class SfpData(BaseModel):
    identifier: int
    ext_identifier: int
    connector: int
    encoding: int
    vendor_name: str
    vendor_pn: str
    vendor_rev: str
    smf_length_m: int
    om1_length_m: int
    om2_length_m: int
    om4_or_copper_length_m: int
    cc_base_valid: bool

@app.get("/sfp/data", response_model=SfpData)
def get_sfp_data(device: str = "/dev/i2c-1"):
    fd = libsfp.sfp_i2c_init(device.encode())
    if fd < 0:
        raise HTTPException(status_code=500, detail=f"Failed to open I2C device {device}")
    
    try:
        buffer = (ctypes.c_uint8 * SFP_A0_BASE_SIZE)()
        if not libsfp.sfp_read_block(fd, SFP_I2C_ADDR_A0, 0, buffer, SFP_A0_BASE_SIZE):
            raise HTTPException(status_code=500, detail="Failed to read SFP EEPROM")
        
        a0 = SfpA0hBase()
        libsfp.sfp_parse_a0_base_identifier(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_ext_identifier(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_connector(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_encoding(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_smf(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_om1(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_om2(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_om4_or_copper(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_ext_compliance(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_cc_base(buffer, ctypes.byref(a0))
        
        # Vendor data parsing (manual since C doesn't parse it to strings yet)
        # Bytes 20-35 are vendor name, 40-55 are PN, 56-59 are Rev
        vendor_name = bytes(buffer[20:36]).decode('ascii').strip()
        vendor_pn = bytes(buffer[40:56]).decode('ascii').strip()
        vendor_rev = bytes(buffer[56:60]).decode('ascii').strip()

        return {
            "identifier": a0.identifier,
            "ext_identifier": a0.ext_identifier,
            "connector": a0.connector,
            "encoding": a0.encoding,
            "vendor_name": vendor_name,
            "vendor_pn": vendor_pn,
            "vendor_rev": vendor_rev,
            "smf_length_m": a0.smf_length_m,
            "om1_length_m": a0.om1_length_m,
            "om2_length_m": a0.om2_length_m,
            "om4_or_copper_length_m": a0.om4_or_copper_length_m,
            "cc_base_valid": a0.cc_base_is_valid
        }
    finally:
        libsfp.sfp_i2c_close(fd)

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
