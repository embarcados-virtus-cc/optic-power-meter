import { useStore } from '@tanstack/react-store'
import { FolderClock } from 'lucide-react'
import { Bar, BarChart, CartesianGrid, XAxis, YAxis } from 'recharts'
import { Skeleton } from '../../ui/skeleton'
import { Label } from '../../ui/label'
import {
  CardComponent,
  CardContentComponent,
  CardHeaderComponent,
} from '@/components/ui/card'
import {
  ChartContainer,
  ChartTooltip,
  ChartTooltipContent,
} from '@/components/ui/chart'
import { historyStore } from '@/stores/dataStores'

const chartConfig = {
  power: {
    label: 'Potência',
    color: 'hsl(var(--foreground))',
  },
}

export function History({ isLoading }: { isLoading: boolean }) {
  const historyData = useStore(historyStore)

  // Criar dados do gráfico com tempo em segundos (cada ponto = 2 segundos)
  const chartData = historyData.map((value, index) => {
    // Cada ponto representa 2 segundos, então o tempo é index * 2
    const timeInSeconds = index * 2
    return {
      time: timeInSeconds,
      power: value,
    }
  })

  // Calcular range adaptável baseado nos valores recebidos
  const calculateAdaptiveDomain = () => {
    if (historyData.length === 0) {
      // Range padrão para LANs (0 a -10 dBm)
      return [0, -10]
    }

    const minValue = Math.min(...historyData)
    const maxValue = Math.max(...historyData)

    // Se todos os valores são iguais, usar range padrão
    if (minValue === maxValue) {
      const center = minValue
      const padding = Math.max(2, Math.abs(center) * 0.1) // 10% de padding ou mínimo de 2 dBm
      return [Math.min(3, center + padding), Math.max(-50, center - padding)]
    }

    // Adicionar padding de 10% do range para melhor visualização
    const range = maxValue - minValue
    const padding = Math.max(1, range * 0.1) // Mínimo de 1 dBm de padding

    const top = Math.min(3, maxValue + padding)
    const bottom = Math.max(-50, minValue - padding)

    return [top, bottom]
  }

  const [yAxisTop, yAxisBottom] = calculateAdaptiveDomain()

  return (
    <CardComponent>
      <CardHeaderComponent>
        <div className="flex items-center gap-3">
          {isLoading ? (
            <>
              <Skeleton className="w-6 h-6 bg-muted rounded-md" />
              <Skeleton className="w-64 h-6 bg-muted rounded-md" />
            </>
          ) : (
            <>
              <FolderClock className="text-foreground" size={24} />
              <Label className="text-lg font-bold uppercase tracking-wider text-foreground">
                Histórico de Potência Óptica Recebida (RX)
              </Label>
            </>
          )}
        </div>
      </CardHeaderComponent>
      <CardContentComponent className="p-0 h-[300px] min-h-[300px]">
        {isLoading ? (
          <div className="w-full h-full px-6 pb-6">
            <div className="w-full h-full flex items-center justify-between gap-1 relative">
              {/* Linha de referência no topo (0 dBm) para range padrão de LANs */}
              <div className="absolute top-0 left-0 right-0 h-px bg-none" />
              {Array.from({ length: 15 }).map((_, i) => {
                // Simula valores no range padrão de LANs (0 a -10 dBm)
                const randomValue = Math.random() * 10 - 10 // -10 a 0
                // Converte para percentual: 0 dBm = topo (0%), -10 dBm = base (100%)
                const heightPercent = ((randomValue + 10) / 10) * 100
                return (
                  <Skeleton
                    key={i}
                    className="flex-1 bg-muted rounded-t"
                    style={{
                      height: `${heightPercent}%`,
                      alignSelf: 'flex-end',
                    }}
                  />
                )
              })}
            </div>
          </div>
        ) : (
          <div className="w-full h-full overflow-x-auto">
            <div className="min-w-[600px] h-full px-2 sm:px-6 pb-2 sm:pb-6">
              <ChartContainer
                config={chartConfig}
                className="w-full h-full [&_.recharts-bar-rectangle]:hover:opacity-80 [&_.recharts-bar-rectangle]:transition-all [&_.recharts-bar-rectangle]:cursor-pointer"
              >
                <BarChart
                  data={chartData}
                  margin={{ top: 5, right: 10, left: 10, bottom: 5 }}
                >
                  <CartesianGrid strokeDasharray="3 3" className="stroke-border" />
                  <XAxis
                    dataKey="time"
                    className="[&_text]:fill-foreground [&_line]:stroke-foreground"
                    tickLine={{ stroke: 'currentColor' }}
                    label={{
                      value: 'Tempo (s)',
                      position: 'insideBottom',
                      offset: -5,
                      className: 'fill-foreground',
                    }}
                    tickFormatter={(value) => `${value}s`}
                    interval={historyData.length > 15 ? 2 : 0}
                  />
                  <YAxis
                    domain={[yAxisTop, yAxisBottom]}
                    className="[&_text]:fill-foreground [&_line]:stroke-foreground"
                    tickLine={{ stroke: 'currentColor' }}
                    label={{
                      value: 'dBm',
                      angle: -90,
                      position: 'insideLeft',
                      className: 'fill-foreground',
                    }}
                    allowDataOverflow={false}
                    width={40}
                  />
                  <ChartTooltip
                    content={<ChartTooltipContent />}
                    cursor={{ fill: 'hsl(var(--muted))' }}
                    labelFormatter={(value) => `Tempo: ${value}s`}
                  />
                  <Bar
                    dataKey="power"
                    className="fill-foreground"
                    radius={[4, 4, 0, 0]}
                  />
                </BarChart>
              </ChartContainer>
            </div>
          </div>
        )}
      </CardContentComponent>
    </CardComponent>
  )
}
