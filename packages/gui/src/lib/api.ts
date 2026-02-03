export type ModuleInfo = {
  identifier?: number | null
  ext_identifier?: number | null
  connector?: number | null
  encoding?: number | null
  vendor_name?: string | null
  vendor_pn?: string | null
  vendor_rev?: string | null
  cc_base_valid?: boolean | null
}

export type CurrentReading = {
  timestamp: string
  rx_power_dbm: number
  temperature_c?: number | null
  voltage_v?: number | null
  bias_ma?: number | null
  signal_quality: string
  module: ModuleInfo
}

export type HistoryPoint = {
  timestamp: string
  rx_power_dbm?: number | null
}

import { mocks } from '@/mocks'

async function http<T>(path: string): Promise<T> {
  try {
    const res = await fetch(path, { headers: { Accept: 'application/json' } })
    if (!res.ok) {
      const text = await res.text().catch(() => '')
      throw new Error(`HTTP ${res.status} ${res.statusText} em ${path}: ${text}`)
    }
    return (await res.json()) as T
  } catch (error) {
    // Se estiver em desenvolvimento, tenta carregar dos mocks em caso de erro na API
    if (import.meta.env.DEV) {
      console.warn(`API call to ${path} failed. Falling back to mock data...`, error)
      if (path.includes('/api/v1/current')) {
        return mocks.current as unknown as T
      }
      if (path.includes('/api/v1/history')) {
        return mocks.history as unknown as T
      }
    }
    throw error
  }
}

// Tipos para os dados estáticos (A0h)
export type SfpStaticData = {
  // Identificação Básica
  identifier?: number
  identifier_type?: string
  ext_identifier?: number
  ext_identifier_valid?: boolean
  connector?: number
  connector_type?: string

  // Compliance Codes (objetos com booleanos)
  compliance_codes?: {
    byte3_ethernet_infiniband?: Record<string, boolean>
    byte4_escon_sonet?: Record<string, boolean>
    byte5_sonet?: Record<string, boolean>
    byte6_ethernet_1g?: Record<string, boolean>
    byte7_fc_link_length?: Record<string, boolean>
    byte8_fc_technology?: Record<string, boolean>
    byte9_fc_transmission_media?: Record<string, boolean>
    byte10_fc_channel_speed?: Record<string, boolean>
  }

  // Características Técnicas
  encoding?: number
  nominal_rate_mbd?: number
  nominal_rate_status?: number
  rate_identifier?: number
  smf_length_km?: number
  smf_length_status?: number
  smf_attenuation_db_per_100m?: number
  om2_length_m?: number
  om2_length_status?: number
  om1_length_m?: number
  om1_length_status?: number
  om4_or_copper_length_m?: number
  om4_or_copper_length_status?: number

  // Informações do Fabricante
  vendor_name?: string
  vendor_name_valid?: boolean
  vendor_oui?: number[]
  vendor_oui_u32?: number
  vendor_oui_valid?: boolean
  vendor_pn?: string
  vendor_pn_valid?: boolean
  vendor_rev?: string
  ext_compliance_code?: number
  ext_compliance_desc?: string

  // Outros
  variant?: number // 0=OPTICAL, 1=PASSIVE_CABLE, 2=ACTIVE_CABLE
  wavelength_nm?: number
  cable_compliance?: number
  fc_speed_2?: number
  fc_speed_2_valid?: boolean
  cc_base?: number
  cc_base_valid?: boolean
}


export const api = {
  current: () => http<CurrentReading>('/api/v1/current'),
  history: (limit = 30) => http<HistoryPoint[]>(`/api/v1/history?limit=${limit}`),
  static: () => http<SfpStaticData>('/api/static'),
}


