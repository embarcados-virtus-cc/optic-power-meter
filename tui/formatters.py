"""Formatadores para exibição de dados SFP na TUI"""

from typing import Dict, Any, List, Tuple, Optional
from datetime import datetime
from prompt_toolkit.application import get_app


def get_window_width() -> int:
    """Obtém a largura da janela de conteúdo"""
    try:
        from prompt_toolkit.output import get_default_output
        output = get_default_output()
        if hasattr(output, 'get_size'):
            size = output.get_size()
            # Menu (38) + padding (2) + padding (2) + vertical line (1) + padding (2) = 45
            # Resto é para conteúdo
            width = size.columns - 45
            return max(40, width)  # Mínimo de 40 caracteres
    except:
        pass
    return 60  # Default


def format_section_header(title: str) -> List[Tuple[str, str]]:
    """Cria cabeçalho de seção responsivo"""
    width = get_window_width()
    line = "═" * width
    return [
        ("class:section", line + "\n"),
        ("class:section", title + "\n"),
        ("class:section", line + "\n"),
        ("", "\n")
    ]


def format_timestamp(ts: float) -> str:
    """Formata timestamp Unix para string legível"""
    if ts == 0:
        return "N/A"
    return datetime.fromtimestamp(ts).strftime("%Y-%m-%d %H:%M:%S")


def format_float(value: Any, decimals: int = 2, unit: str = "") -> str:
    """Formata float com decimais e unidade"""
    if value is None:
        return "N/A"
    try:
        return f"{float(value):.{decimals}f}{unit}"
    except (ValueError, TypeError):
        return "N/A"


def format_int(value: Any) -> str:
    """Formata inteiro"""
    if value is None:
        return "N/A"
    try:
        return str(int(value))
    except (ValueError, TypeError):
        return "N/A"


def format_bool(value: Any, true_str: str = "Sim", false_str: str = "Não") -> str:
    """Formata booleano"""
    if value is None:
        return "N/A"
    return true_str if bool(value) else false_str


def format_compliance_codes(compliance: Dict[str, Any]) -> List[str]:
    """Formata códigos de compliance em lista de strings"""
    lines = []
    
    if not compliance:
        return ["Nenhum código de compliance disponível"]
    
    # Byte 3: Ethernet 10G e InfiniBand
    byte3 = compliance.get("byte3_ethernet_infiniband", {})
    if byte3:
        byte3_items = [k.replace("_", " ").title() for k, v in byte3.items() if v]
        if byte3_items:
            lines.append("Ethernet 10G / InfiniBand:")
            for item in byte3_items:
                lines.append(f"  • {item}")
    
    # Byte 4: ESCON e SONET
    byte4 = compliance.get("byte4_escon_sonet", {})
    if byte4:
        byte4_items = [k.replace("_", " ").title() for k, v in byte4.items() if v]
        if byte4_items:
            lines.append("ESCON / SONET:")
            for item in byte4_items:
                lines.append(f"  • {item}")
    
    # Byte 5: SONET
    byte5 = compliance.get("byte5_sonet", {})
    if byte5:
        byte5_items = [k.replace("_", " ").title() for k, v in byte5.items() if v]
        if byte5_items:
            lines.append("SONET:")
            for item in byte5_items:
                lines.append(f"  • {item}")
    
    # Byte 6: Ethernet 1G
    byte6 = compliance.get("byte6_ethernet_1g", {})
    if byte6:
        byte6_items = [k.replace("_", " ").title() for k, v in byte6.items() if v]
        if byte6_items:
            lines.append("Ethernet 1G:")
            for item in byte6_items:
                lines.append(f"  • {item}")
    
    # Byte 7: FC Link Length
    byte7 = compliance.get("byte7_fc_link_length", {})
    if byte7:
        byte7_items = [k.replace("_", " ").title() for k, v in byte7.items() if v]
        if byte7_items:
            lines.append("FC Link Length:")
            for item in byte7_items:
                lines.append(f"  • {item}")
    
    # Byte 8: FC Technology
    byte8 = compliance.get("byte8_fc_technology", {})
    if byte8:
        byte8_items = [k.replace("_", " ").title() for k, v in byte8.items() if v]
        if byte8_items:
            lines.append("FC Technology:")
            for item in byte8_items:
                lines.append(f"  • {item}")
    
    # Byte 9: FC Transmission Media
    byte9 = compliance.get("byte9_fc_transmission_media", {})
    if byte9:
        byte9_items = [k.replace("_", " ").title() for k, v in byte9.items() if v]
        if byte9_items:
            lines.append("FC Transmission Media:")
            for item in byte9_items:
                lines.append(f"  • {item}")
    
    # Byte 10: FC Channel Speed
    byte10 = compliance.get("byte10_fc_channel_speed", {})
    if byte10:
        byte10_items = [k.replace("_", " ").title() for k, v in byte10.items() if v]
        if byte10_items:
            lines.append("FC Channel Speed:")
            for item in byte10_items:
                lines.append(f"  • {item}")
    
    return lines if lines else ["Nenhum código de compliance ativo"]


# ===========================
# Configurações do Transceptor (Apenas Flags)
# ===========================

def format_flags_only(data: Dict[str, Any]) -> List[Tuple[str, str]]:
    """Formata apenas flags (alarmes e avisos) para configurações"""
    lines = []
    
    if not data.get("a2", {}).get("valid", False):
        return [("class:error", "SFP não detectado ou dados A2h inválidos")]
    
    a2 = data.get("a2", {})
    
    # Alarmes
    lines.extend(format_section_header("ALARMES"))
    
    alarms = a2.get("alarms", {})
    if alarms:
        for param, values in alarms.items():
            if isinstance(values, dict):
                high = values.get("high", False)
                low = values.get("low", False)
                param_name = param.replace("_", " ").title()
                
                lines.append(("class:label", f"{param_name}:\n"))
                if high or low:
                    status = []
                    if high:
                        status.append("ALTO")
                    if low:
                        status.append("BAIXO")
                    lines.append(("class:error", f"  {', '.join(status)}\n"))
                else:
                    lines.append(("class:success", "  OK\n"))
                lines.append(("", ""))
    else:
        lines.append(("class:value", "Nenhum alarme configurado\n"))
        lines.append(("", ""))
    
    # Warnings
    lines.extend(format_section_header("AVISOS"))
    
    warnings = a2.get("warnings", {})
    if warnings:
        for param, values in warnings.items():
            if isinstance(values, dict):
                high = values.get("high", False)
                low = values.get("low", False)
                param_name = param.replace("_", " ").title()
                
                lines.append(("class:label", f"{param_name}:\n"))
                if high or low:
                    status = []
                    if high:
                        status.append("ALTO")
                    if low:
                        status.append("BAIXO")
                    lines.append(("class:warning", f"  {', '.join(status)}\n"))
                else:
                    lines.append(("class:success", "  OK\n"))
                lines.append(("", ""))
    else:
        lines.append(("class:value", "Nenhum aviso configurado\n"))
    
    return lines


# ===========================
# Valores Atuais de Potência Óptica (A2h sem flags)
# ===========================

def format_dynamic_values_only(data: Dict[str, Any]) -> List[Tuple[str, str]]:
    """Formata apenas valores dinâmicos A2h (sem flags)"""
    lines = []
    
    if not data.get("a2", {}).get("valid", False):
        return [("class:error", "SFP não detectado ou dados A2h inválidos")]
    
    a2 = data.get("a2", {})
    
    # Temperatura
    lines.extend(format_section_header("TEMPERATURA"))
    temp_valid = a2.get("temperature_valid", False)
    if temp_valid:
        temp_c = a2.get("temperature_c")
        temp_raw = a2.get("temperature_raw")
        lines.append(("class:label", "Temperatura: "))
        lines.append(("class:value", f"{format_float(temp_c, 2, ' °C')}\n"))
        lines.append(("class:label", "Valor Raw: "))
        lines.append(("class:value", f"{temp_raw}\n"))
    else:
        lines.append(("class:label", "Temperatura: "))
        lines.append(("class:error", "N/A\n"))
    lines.append(("", ""))
    
    # Tensão
    lines.extend(format_section_header("TENSÃO"))
    voltage_valid = a2.get("voltage_valid", False)
    if voltage_valid:
        voltage_v = a2.get("voltage_v")
        voltage_raw = a2.get("voltage_raw")
        lines.append(("class:label", "Tensão: "))
        lines.append(("class:value", f"{format_float(voltage_v, 3, ' V')}\n"))
    else:
        lines.append(("class:label", "Tensão: "))
        lines.append(("class:error", "N/A\n"))
    lines.append(("", ""))
    
    # Corrente de Bias
    lines.extend(format_section_header("CORRENTE DE BIAS"))
    bias_valid = a2.get("bias_current_valid", False)
    if bias_valid:
        bias_ma = a2.get("bias_current_ma")
        bias_raw = a2.get("bias_current_raw")
        lines.append(("class:label", "Corrente de Bias: "))
        lines.append(("class:value", f"{format_float(bias_ma, 2, ' mA')}\n"))
        lines.append(("class:label", "Valor Raw: "))
        lines.append(("class:value", f"{bias_raw}\n"))
    else:
        lines.append(("class:label", "Corrente de Bias: "))
        lines.append(("class:error", "N/A\n"))
    lines.append(("", ""))
    
    # Potência TX
    lines.extend(format_section_header("POTÊNCIA DE TRANSMISSÃO (TX)"))
    tx_valid = a2.get("tx_power_valid", False)
    if tx_valid:
        tx_dbm = a2.get("tx_power_dbm")
        tx_mw = a2.get("tx_power_mw")
        tx_raw = a2.get("tx_power_raw")
        lines.append(("class:label", "Potência TX: "))
        lines.append(("class:value", f"{format_float(tx_dbm, 2, ' dBm')} ({format_float(tx_mw, 3, ' mW')})\n"))
        lines.append(("class:label", "Valor Raw: "))
        lines.append(("class:value", f"{tx_raw}\n"))
    else:
        lines.append(("class:label", "Potência TX: "))
        lines.append(("class:error", "N/A\n"))
    lines.append(("", ""))
    
    # Potência RX
    lines.extend(format_section_header("POTÊNCIA DE RECEPÇÃO (RX)"))
    rx_valid = a2.get("rx_power_valid", False)
    if rx_valid:
        rx_dbm = a2.get("rx_power_dbm")
        rx_mw = a2.get("rx_power_mw")
        rx_raw = a2.get("rx_power_raw")
        lines.append(("class:label", "Potência RX: "))
        lines.append(("class:value", f"{format_float(rx_dbm, 2, ' dBm')} ({format_float(rx_mw, 3, ' mW')})\n"))
        lines.append(("class:label", "Valor Raw: "))
        lines.append(("class:value", f"{rx_raw}\n"))
    else:
        lines.append(("class:label", "Potência RX: "))
        lines.append(("class:error", "N/A\n"))
    
    return lines


# ===========================
# Informações do Transceptor (TODOS os campos A0h)
# ===========================

def format_all_a0h_fields(data: Dict[str, Any]) -> List[Tuple[str, str]]:
    """Formata TODOS os campos da página A0h"""
    lines = []
    
    if not data.get("a0", {}).get("valid", False):
        return [("class:error", "SFP não detectado ou dados A0h inválidos")]
    
    a0 = data.get("a0", {})
    
    # Byte 0 - Identifier
    lines.extend(format_section_header("BYTE 0 - IDENTIFIER"))
    identifier = a0.get("identifier")
    identifier_type = a0.get("identifier_type", "N/A")
    lines.append(("class:label", "Identifier (Byte 0): "))
    if identifier is not None:
        lines.append(("class:value", f"{format_int(identifier)} (0x{identifier:02X}) - {identifier_type}\n"))
    else:
        lines.append(("class:value", f"N/A - {identifier_type}\n"))
    lines.append(("", ""))
    
    # Byte 1 - Extended Identifier
    lines.extend(format_section_header("BYTE 1 - EXTENDED IDENTIFIER"))
    ext_identifier = a0.get("ext_identifier")
    ext_identifier_valid = a0.get("ext_identifier_valid", False)
    lines.append(("class:label", "Extended Identifier (Byte 1): "))
    if ext_identifier is not None:
        lines.append(("class:value", f"{format_int(ext_identifier)} (0x{ext_identifier:02X})\n"))
    else:
        lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "Válido: "))
    lines.append(("class:value", f"{format_bool(ext_identifier_valid)}\n"))
    lines.append(("", ""))
    
    # Byte 2 - Connector
    lines.extend(format_section_header("BYTE 2 - CONNECTOR"))
    connector = a0.get("connector")
    connector_type = a0.get("connector_type", "N/A")
    lines.append(("class:label", "Connector (Byte 2): "))
    if connector is not None:
        lines.append(("class:value", f"{format_int(connector)} (0x{connector:02X}) - {connector_type}\n"))
    else:
        lines.append(("class:value", f"N/A - {connector_type}\n"))
    lines.append(("", ""))
    
    # Bytes 3-10 - Compliance Codes
    compliance = a0.get("compliance_codes", {})
    if compliance:
        lines.extend(format_section_header("BYTES 3-10 - COMPLIANCE CODES"))
        compliance_lines = format_compliance_codes(compliance)
        for line in compliance_lines:
            if line.endswith(":"):
                lines.append(("class:label", f"{line}\n"))
            else:
                lines.append(("class:value", f"{line}\n"))
        lines.append(("", ""))
    
    # Byte 11 - Encoding
    lines.extend(format_section_header("BYTE 11 - ENCODING"))
    encoding = a0.get("encoding")
    lines.append(("class:label", "Encoding (Byte 11): "))
    if encoding is not None:
        lines.append(("class:value", f"{format_int(encoding)} (0x{encoding:02X})\n"))
    else:
        lines.append(("class:value", "N/A\n"))
    lines.append(("", ""))
    
    # Byte 12 - Nominal Rate
    lines.extend(format_section_header("BYTE 12 - NOMINAL RATE"))
    nominal_rate_mbd = a0.get("nominal_rate_mbd")
    nominal_rate_status = a0.get("nominal_rate_status", 0)
    status_str = {0: "Not Specified", 1: "Valid", 2: "Extended"}.get(nominal_rate_status, "Unknown")
    lines.append(("class:label", "Nominal Rate (Byte 12): "))
    lines.append(("class:value", f"{format_int(nominal_rate_mbd)} MBd\n"))
    lines.append(("class:label", "Status: "))
    lines.append(("class:value", f"{nominal_rate_status} - {status_str}\n"))
    lines.append(("", ""))
    
    # Byte 13 - Rate Identifier
    lines.extend(format_section_header("BYTE 13 - RATE IDENTIFIER"))
    rate_identifier = a0.get("rate_identifier")
    lines.append(("class:label", "Rate Identifier (Byte 13): "))
    if rate_identifier is not None:
        lines.append(("class:value", f"{format_int(rate_identifier)} (0x{rate_identifier:02X})\n"))
    else:
        lines.append(("class:value", "N/A\n"))
    lines.append(("", ""))
    
    # Byte 14 - SMF Length
    lines.extend(format_section_header("BYTE 14 - SMF LENGTH / COPPER ATTENUATION"))
    smf_length_km = a0.get("smf_length_km")
    smf_length_status = a0.get("smf_length_status", 0)
    smf_attenuation = a0.get("smf_attenuation_db_per_100m")
    status_str = {0: "Not Supported", 1: "Valid", 2: "Extended"}.get(smf_length_status, "Unknown")
    lines.append(("class:label", "SMF Length (Byte 14): "))
    lines.append(("class:value", f"{format_int(smf_length_km)} km\n"))
    lines.append(("class:label", "Status: "))
    lines.append(("class:value", f"{smf_length_status} - {status_str}\n"))
    if smf_attenuation:
        lines.append(("class:label", "Atenuação: "))
        lines.append(("class:value", f"{format_float(smf_attenuation, 2, ' dB/100m')}\n"))
    lines.append(("", ""))

    # Byte 15 - SMF Length (100m)
    lines.extend(format_section_header("BYTE 15 - SMF LENGTH (100m)"))
    smf_length_m = a0.get("smf_length_m")
    smf_length_status_m = a0.get("smf_length_status_m", 0)
    status_str = {0: "Not Supported", 1: "Valid", 2: "Extended"}.get(smf_length_status_m, "Unknown")
    lines.append(("class:label", "SMF Length (Byte 15): "))
    lines.append(("class:value", f"{format_int(smf_length_m)} m\n"))
    lines.append(("class:label", "Status: "))
    lines.append(("class:value", f"{smf_length_status_m} - {status_str}\n"))
    lines.append(("", ""))
    
    # Byte 16 - OM2 Length
    lines.extend(format_section_header("BYTE 16 - OM2 LENGTH"))
    om2_length_m = a0.get("om2_length_m")
    om2_length_status = a0.get("om2_length_status", 0)
    status_str = {0: "Not Supported", 1: "Valid", 2: "Extended"}.get(om2_length_status, "Unknown")
    lines.append(("class:label", "OM2 Length (Byte 16): "))
    lines.append(("class:value", f"{format_int(om2_length_m)} m\n"))
    lines.append(("class:label", "Status: "))
    lines.append(("class:value", f"{om2_length_status} - {status_str}\n"))
    lines.append(("", ""))
    
    # Byte 17 - OM1 Length
    lines.extend(format_section_header("BYTE 17 - OM1 LENGTH"))
    om1_length_m = a0.get("om1_length_m")
    om1_length_status = a0.get("om1_length_status", 0)
    status_str = {0: "Not Supported", 1: "Valid", 2: "Extended"}.get(om1_length_status, "Unknown")
    lines.append(("class:label", "OM1 Length (Byte 17): "))
    lines.append(("class:value", f"{format_int(om1_length_m)} m\n"))
    lines.append(("class:label", "Status: "))
    lines.append(("class:value", f"{om1_length_status} - {status_str}\n"))
    lines.append(("", ""))
    
    # Byte 18 - OM4 or Copper Length
    lines.extend(format_section_header("BYTE 18 - OM4 OR COPPER LENGTH"))
    om4_or_copper_length_m = a0.get("om4_or_copper_length_m")
    om4_or_copper_length_status = a0.get("om4_or_copper_length_status", 0)
    status_str = {0: "Not Supported", 1: "Valid", 2: "Extended"}.get(om4_or_copper_length_status, "Unknown")
    lines.append(("class:label", "OM4/Copper Length (Byte 18): "))
    lines.append(("class:value", f"{format_int(om4_or_copper_length_m)} m\n"))
    lines.append(("class:label", "Status: "))
    lines.append(("class:value", f"{om4_or_copper_length_status} - {status_str}\n"))
    lines.append(("", ""))

    # Byte 19 - OM3 Length
    lines.extend(format_section_header("BYTE 19 - OM3 LENGTH"))
    om3_length_m = a0.get("om3_length_m")
    om3_length_status = a0.get("om3_length_status", 0)
    status_str = {0: "Not Supported", 1: "Valid", 2: "Extended"}.get(om3_length_status, "Unknown")
    lines.append(("class:label", "OM3 Length (Byte 19): "))
    lines.append(("class:value", f"{format_int(om3_length_m)} m\n"))
    lines.append(("class:label", "Status: "))
    lines.append(("class:value", f"{om3_length_status} - {status_str}\n"))
    lines.append(("", ""))
    
    # Bytes 20-35 - Vendor Name
    lines.extend(format_section_header("BYTES 20-35 - VENDOR NAME"))
    vendor_name = a0.get("vendor_name", "")
    vendor_name_valid = a0.get("vendor_name_valid", False)
    lines.append(("class:label", "Vendor Name (Bytes 20-35): "))
    lines.append(("class:value", f"{vendor_name}\n"))
    lines.append(("class:label", "Válido: "))
    lines.append(("class:value", f"{format_bool(vendor_name_valid)}\n"))
    lines.append(("", ""))
    
    # Byte 36 - Extended Compliance
    lines.extend(format_section_header("BYTE 36 - EXTENDED COMPLIANCE"))
    ext_compliance_code = a0.get("ext_compliance_code")
    ext_compliance_desc = a0.get("ext_compliance_desc", "N/A")
    lines.append(("class:label", "Extended Compliance (Byte 36): "))
    if ext_compliance_code is not None:
        lines.append(("class:value", f"{format_int(ext_compliance_code)} (0x{ext_compliance_code:02X})\n"))
    else:
        lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "Descrição: "))
    lines.append(("class:value", f"{ext_compliance_desc}\n"))
    lines.append(("", ""))
    
    # Bytes 37-39 - Vendor OUI
    lines.extend(format_section_header("BYTES 37-39 - VENDOR OUI"))
    vendor_oui = a0.get("vendor_oui", [])
    vendor_oui_u32 = a0.get("vendor_oui_u32")
    vendor_oui_valid = a0.get("vendor_oui_valid", False)
    if vendor_oui and len(vendor_oui) == 3:
        oui_str = f"{vendor_oui[0]:02X}:{vendor_oui[1]:02X}:{vendor_oui[2]:02X}"
        lines.append(("class:label", "Vendor OUI (Bytes 37-39): "))
        lines.append(("class:value", f"{oui_str}\n"))
        lines.append(("class:label", "OUI (uint32): "))
        lines.append(("class:value", f"{format_int(vendor_oui_u32)}\n"))
    else:
        lines.append(("class:label", "Vendor OUI: "))
        lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "Válido: "))
    lines.append(("class:value", f"{format_bool(vendor_oui_valid)}\n"))
    lines.append(("", ""))
    
    # Bytes 40-55 - Vendor Part Number
    lines.extend(format_section_header("BYTES 40-55 - VENDOR PART NUMBER"))
    vendor_pn = a0.get("vendor_pn", "")
    vendor_pn_valid = a0.get("vendor_pn_valid", False)
    lines.append(("class:label", "Vendor Part Number (Bytes 40-55): "))
    lines.append(("class:value", f"{vendor_pn}\n"))
    lines.append(("class:label", "Válido: "))
    lines.append(("class:value", f"{format_bool(vendor_pn_valid)}\n"))
    lines.append(("", ""))
    
    # Bytes 56-59 - Vendor Revision
    lines.extend(format_section_header("BYTES 56-59 - VENDOR REVISION"))
    vendor_rev = a0.get("vendor_rev", "")
    lines.append(("class:label", "Vendor Revision (Bytes 56-59): "))
    lines.append(("class:value", f"{vendor_rev}\n"))
    lines.append(("", ""))
    
    # Bytes 60-61 - Wavelength or Cable Compliance
    lines.extend(format_section_header("BYTES 60-61 - WAVELENGTH OR CABLE COMPLIANCE"))
    variant = a0.get("variant", 0)
    variant_str = {0: "Óptico", 1: "Cabo Passivo", 2: "Cabo Ativo"}.get(variant, "Desconhecido")
    lines.append(("class:label", "Variante: "))
    lines.append(("class:value", f"{variant} - {variant_str}\n"))
    
    if variant == 0:  # OPTICAL
        wavelength_nm = a0.get("wavelength_nm")
        if wavelength_nm:
            lines.append(("class:label", "Wavelength (Bytes 60-61): "))
            lines.append(("class:value", f"{format_int(wavelength_nm)} nm\n"))
    else:  # CABLE
        cable_compliance = a0.get("cable_compliance")
        if cable_compliance is not None:
            lines.append(("class:label", "Cable Compliance (Bytes 60-61): "))
            lines.append(("class:value", f"{format_int(cable_compliance)} (0x{cable_compliance:02X})\n"))
    lines.append(("", ""))
    
    # Byte 62 - Fibre Channel Speed 2
    lines.extend(format_section_header("BYTE 62 - FIBRE CHANNEL SPEED 2"))
    fc_speed_2_valid = a0.get("fc_speed_2_valid", False)
    fc_speed_2 = a0.get("fc_speed_2")
    lines.append(("class:label", "FC Speed 2 Válido: "))
    lines.append(("class:value", f"{format_bool(fc_speed_2_valid)}\n"))
    if fc_speed_2_valid and fc_speed_2 is not None:
        lines.append(("class:label", "FC Speed 2 (Byte 62): "))
        lines.append(("class:value", f"{format_int(fc_speed_2)} (0x{fc_speed_2:02X})\n"))
    else:
        lines.append(("class:label", "FC Speed 2 (Byte 62): "))
        lines.append(("class:value", "N/A\n"))
    lines.append(("", ""))
    
    # Byte 92 - Extended Fields
    lines.extend(format_section_header("BYTE 92 - DIAGNOSTIC MONITORING (EXTENDED)"))
    dmi_implemented = a0.get("dmi_implemented", False)
    change_addr_req = a0.get("change_addr_req", False)
    calibration_type = a0.get("calibration_type", "N/A")
    
    lines.append(("class:label", "DDM Implemented: "))
    lines.append(("class:value", f"{format_bool(dmi_implemented)}\n"))
    lines.append(("class:label", "Change Address Required: "))
    lines.append(("class:value", f"{format_bool(change_addr_req)}\n"))
    lines.append(("class:label", "Calibration: "))
    lines.append(("class:value", f"{calibration_type}\n"))
    lines.append(("", ""))

    # Byte 63 - CC_BASE (Checksum)
    lines.extend(format_section_header("BYTE 63 - CC_BASE (CHECKSUM)"))
    cc_base = a0.get("cc_base")
    cc_base_valid = a0.get("cc_base_valid", False)
    lines.append(("class:label", "CC_BASE (Byte 63): "))
    if cc_base is not None:
        lines.append(("class:value", f"{format_int(cc_base)} (0x{cc_base:02X})\n"))
    else:
        lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "Válido: "))
    lines.append(("class:value", f"{format_bool(cc_base_valid)}\n"))
    
    return lines


# ===========================
# Templates (para modo debug)
# ===========================

def format_flags_template() -> List[Tuple[str, str]]:
    """Template de flags (alarmes e avisos)"""
    lines = []
    lines.extend(format_section_header("ALARMES"))
    lines.append(("class:label", "Temperature: "))
    lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "Voltage: "))
    lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "Bias Current: "))
    lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "TX Power: "))
    lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "RX Power: "))
    lines.append(("class:value", "N/A\n"))
    lines.append(("", ""))
    lines.extend(format_section_header("AVISOS"))
    lines.append(("class:label", "Temperature: "))
    lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "Voltage: "))
    lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "Bias Current: "))
    lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "TX Power: "))
    lines.append(("class:value", "N/A\n"))
    lines.append(("class:label", "RX Power: "))
    lines.append(("class:value", "N/A\n"))
    return lines


def format_dynamic_template() -> List[Tuple[str, str]]:
    """Template de dados dinâmicos A2h (sem flags)"""
    lines = []
    lines.extend(format_section_header("TEMPERATURA"))
    lines.append(("class:label", "Temperatura: "))
    lines.append(("class:error", "N/A\n"))
    lines.append(("", ""))
    lines.extend(format_section_header("TENSÃO"))
    lines.append(("class:label", "Tensão: "))
    lines.append(("class:error", "N/A\n"))
    lines.append(("", ""))
    lines.extend(format_section_header("CORRENTE DE BIAS"))
    lines.append(("class:label", "Corrente de Bias: "))
    lines.append(("class:error", "N/A\n"))
    lines.append(("", ""))
    lines.extend(format_section_header("POTÊNCIA DE TRANSMISSÃO (TX)"))
    lines.append(("class:label", "Potência TX: "))
    lines.append(("class:error", "N/A\n"))
    lines.append(("", ""))
    lines.extend(format_section_header("POTÊNCIA DE RECEPÇÃO (RX)"))
    lines.append(("class:label", "Potência RX: "))
    lines.append(("class:error", "N/A\n"))
    return lines


def format_all_a0h_template() -> List[Tuple[str, str]]:
    """Template completo de todos os campos A0h"""
    lines = []
    sections = [
        "BYTE 0 - IDENTIFIER",
        "BYTE 1 - EXTENDED IDENTIFIER",
        "BYTE 2 - CONNECTOR",
        "BYTES 3-10 - COMPLIANCE CODES",
        "BYTE 11 - ENCODING",
        "BYTE 12 - NOMINAL RATE",
        "BYTE 13 - RATE IDENTIFIER",
        "BYTE 14 - SMF LENGTH / COPPER ATTENUATION",
        "BYTE 15 - SMF LENGTH (100m)",
        "BYTE 16 - OM2 LENGTH",
        "BYTE 17 - OM1 LENGTH",
        "BYTE 18 - OM4 OR COPPER LENGTH",
        "BYTE 19 - OM3 LENGTH",
        "BYTES 20-35 - VENDOR NAME",
        "BYTE 36 - EXTENDED COMPLIANCE",
        "BYTES 37-39 - VENDOR OUI",
        "BYTES 40-55 - VENDOR PART NUMBER",
        "BYTES 56-59 - VENDOR REVISION",
        "BYTES 60-61 - WAVELENGTH OR CABLE COMPLIANCE",
        "BYTE 62 - FIBRE CHANNEL SPEED 2",
        "BYTE 92 - DIAGNOSTIC MONITORING (EXTENDED)",
        "BYTE 63 - CC_BASE (CHECKSUM)"
    ]
    
    for section in sections:
        lines.extend(format_section_header(section))
        lines.append(("class:label", "Valor: "))
        lines.append(("class:value", "N/A\n"))
        lines.append(("", ""))
    
    return lines
