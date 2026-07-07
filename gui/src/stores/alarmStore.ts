import { Store } from '@tanstack/store'

export interface AlarmFlag {
  id: string
  parameter: string
  value: number | string
  unit: string
  type: 'ALTO' | 'BAIXO'
  timestamp: string
}

export const alarmStore = new Store<Array<AlarmFlag>>([])

export const removeAlarm = (id: string) => {
  alarmStore.setState((state) => state.filter((flag) => flag.id !== id))
}
