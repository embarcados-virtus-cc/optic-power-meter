import { Store } from '@tanstack/store'

export interface AlarmFlag {
  id: string
  parameter: string
  value: number | string
  unit: string
  type: 'ALTO' | 'BAIXO'
  timestamp: string
}

// Mock inicial
const initialFlags: Array<AlarmFlag> = [
  {
    id: '1',
    parameter: 'Temperatura (°C)',
    value: 33,
    unit: '°C',
    type: 'ALTO',
    timestamp: '2024-01-15 18:20:43',
  },
  {
    id: '2',
    parameter: 'Tensão (V)',
    value: 1.33,
    unit: 'V',
    type: 'BAIXO',
    timestamp: '2024-01-15 18:20:43',
  },
  {
    id: '3',
    parameter: 'Corrente (mA)',
    value: 32.04,
    unit: 'mA',
    type: 'ALTO',
    timestamp: '2024-01-15 18:20:43',
  },
]

export const alarmStore = new Store<Array<AlarmFlag>>(initialFlags)

// Mock de novos alarmes que podem surgir
const mockAlarms: Array<Omit<AlarmFlag, 'id' | 'timestamp'>> = [
  {
    parameter: 'Temperatura (°C)',
    value: 13,
    unit: '°C',
    type: 'BAIXO',
  },
  {
    parameter: 'RX Power (dBm)',
    value: -8.0,
    unit: 'dBm',
    type: 'ALTO',
  },
  {
    parameter: 'RX Power (dBm)',
    value: -7.5,
    unit: 'dBm',
    type: 'ALTO',
  },
]

let mockIndex = 0

// Iniciar simulação globalmente
// Isso roda assim que o arquivo é importado por qualquer componente
if (typeof window !== 'undefined') {
  setInterval(() => {
    if (mockIndex < mockAlarms.length) {
      const template = mockAlarms[mockIndex]
      const newAlarm: AlarmFlag = {
        ...template,
        id: `${Date.now()}`,
        timestamp: new Date().toLocaleString('pt-BR'),
      }

      alarmStore.setState((state) => [...state, newAlarm])
      mockIndex = mockIndex + 1 // Loop ou para? O original parava, mas para teste continuo melhor loopar? O original parava em len.
      // O codigo original parava: if (mockIndex < mockAlarms.length)
      // Vou manter o comportamento original de parar quando acabar os mocks, mas talvez reiniciar?
      // O usuario pediu "toda vez que ter um alarme global", entao novos alarmes sao esperados.
      // Para manter a consistencia com o codigo anterior vou deixar apenas ir ate o fim do vetor.
    }
  }, 2000)
}

export const removeAlarm = (id: string) => {
  alarmStore.setState((state) => state.filter((flag) => flag.id !== id))
}
