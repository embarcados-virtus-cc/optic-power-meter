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

async function http<T>(path: string): Promise<T> {
  const res = await fetch(path, { headers: { Accept: 'application/json' } })
  if (!res.ok) {
    const text = await res.text().catch(() => '')
    throw new Error(`HTTP ${res.status} ${res.statusText} em ${path}: ${text}`)
  }
  return (await res.json()) as T
}

export const api = {
  current: () => http<CurrentReading>('/api/v1/current'),
  history: (limit = 30) => http<HistoryPoint[]>(`/api/v1/history?limit=${limit}`),
}


