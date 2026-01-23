import { Store } from '@tanstack/store'
import { api } from '@/lib/api'

// Store para RxPower
export const rxPowerStore = new Store(-8.0)

// Store para Parameters
export const parametersStore = new Store({
  corrente: 0,
  tensao: 0,
  temperatura: '—',
  qualidadeSinal: '—',
})

// Store para History (array de valores em dBm) - começa vazio e vai adicionando
export const historyStore = new Store<Array<number>>([])

// Limite máximo de pontos no histórico (30 pontos = 60 segundos de dados a cada 2s)
const MAX_HISTORY_POINTS = 30

let didLoadInitialHistory = false

async function refresh() {
  try {
    if (!didLoadInitialHistory) {
      didLoadInitialHistory = true
      const hist = await api.history(MAX_HISTORY_POINTS)
      const values = hist
        .map((p) => p.rx_power_dbm)
        .filter((v): v is number => typeof v === 'number')
        .slice(-MAX_HISTORY_POINTS)
      if (values.length) historyStore.setState(values)
    }

    const current = await api.current()
    rxPowerStore.setState(Number(current.rx_power_dbm.toFixed(2)))

    parametersStore.setState({
      corrente: Number((current.bias_ma ?? 0).toFixed(2)),
      tensao: Number((current.voltage_v ?? 0).toFixed(2)),
      temperatura:
        current.temperature_c != null ? String(Math.round(current.temperature_c)) : '—',
      qualidadeSinal: current.signal_quality ?? '—',
    })

    // Mantém histórico rolando no frontend (alimenta o gráfico imediatamente)
    const currentHistory = historyStore.state
    const updatedHistory = [...currentHistory, Number(current.rx_power_dbm.toFixed(2))]
    if (updatedHistory.length > MAX_HISTORY_POINTS) updatedHistory.shift()
    historyStore.setState(updatedHistory)
  } catch {
    // Silencioso: UI continua mostrando último valor.
  }
}

// Primeira carga (sem esperar o intervalo)
void refresh()

// Atualizar tudo sincronizado a cada 2 segundos
setInterval(() => {
  void refresh()
}, 2000)
