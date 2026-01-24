import { Store } from '@tanstack/store'

export interface CalibrationState {
  rxPower: {
    offset: string
    slope: string
    pwr4: string
    pwr3: string
  }
  txPower: {
    offset: string
    slope: string
  }
  temperature: {
    offset: string
    slope: string
  }
  status: {
    rx: boolean
    tx: boolean
    bias: boolean
    voltage: boolean
    temp: boolean
  }
  info: {
    ddmType: string
    version: string
    rxPowerType: string
    addressChange: string
    isValid: boolean
  }
}

export const calibrationStore = new Store<CalibrationState>({
  rxPower: {
    offset: '0',
    slope: '1',
    pwr4: '0',
    pwr3: '0',
  },
  txPower: {
    offset: '0',
    slope: '1',
  },
  temperature: {
    offset: '0',
    slope: '1',
  },
  status: {
    rx: true,
    tx: true,
    bias: true,
    voltage: true,
    temp: true,
  },
  info: {
    ddmType: 'Internal',
    version: 'Rev 11.0',
    rxPowerType: 'Average',
    addressChange: 'Required',
    isValid: true,
  },
})
