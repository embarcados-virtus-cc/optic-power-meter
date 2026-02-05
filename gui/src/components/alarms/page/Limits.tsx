import { useStore } from '@tanstack/react-store'
import { Settings } from 'lucide-react'
import { useState } from 'react'
import { Store } from '@tanstack/store'
import { Skeleton } from '../../ui/skeleton'
import { Label } from '../../ui/label'
import { CardComponent, CardContentComponent, CardFooterComponent, CardHeaderComponent } from '@/components/ui/card'
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select'
import { Switch } from '@/components/ui/switch'

interface AlarmLimit {
  id: string
  parameter: string
  alarmHigh: number
  warningHigh: number
  warningLow: number
  alarmLow: number
  unit: string
}

type RxPowerUnit = 'dBm' | 'dB' | 'mW' | 'nW'
const RX_POWER_UNITS: Array<RxPowerUnit> = ['dBm', 'dB', 'mW', 'nW']

// Mock data store
export const limitsStore = new Store<Array<AlarmLimit>>([
  { id: '1', parameter: 'Temperatura (°C)', alarmHigh: 33, warningHigh: 33, warningLow: 33, alarmLow: 33, unit: '°C' },
  { id: '2', parameter: 'Tensão (V)', alarmHigh: 3.33, warningHigh: 3.33, warningLow: 3.33, alarmLow: 3.33, unit: 'V' },
  { id: '3', parameter: 'Corrente Bias (mA)', alarmHigh: 32.04, warningHigh: 32.04, warningLow: 32.04, alarmLow: 32.04, unit: 'mA' },
  { id: '4', parameter: 'RX Power (dBm)', alarmHigh: -8.0, warningHigh: -8.0, warningLow: -8.0, alarmLow: -8.0, unit: 'dBm' },
])

export function Limits({ isLoading }: { isLoading: boolean }) {
  const limits = useStore(limitsStore)
  const [rxPowerUnit, setRxPowerUnit] = useState<RxPowerUnit>('dBm')

  const getDecimalPlaces = (unit: string): number => {
    if (unit === '°C') return 1
    if (['V', 'mA', 'dBm', 'dB', 'mW', 'nW'].includes(unit)) return 2
    return 2
  }

  const formatValue = (value: number, unit: string) => {
    const decimals = getDecimalPlaces(unit)
    return value.toFixed(decimals)
  }

  const convertRxPower = (value: number, fromUnit: string, toUnit: RxPowerUnit): number => {
    let dBm = value
    if (fromUnit === 'mW') { dBm = 10 * Math.log10(value) }
    else if (fromUnit === 'nW') { dBm = 10 * Math.log10(value / 1e6) }
    else if (fromUnit === 'dB') { dBm = value }

    if (toUnit === 'dBm' || toUnit === 'dB') { return dBm }
    else if (toUnit === 'mW') { return Math.pow(10, dBm / 10) }
    else { return Math.pow(10, dBm / 10) * 1e6 }
  }

  const getDisplayUnit = (originalUnit: string): string => {
    if (['dBm', 'dB', 'mW', 'nW'].includes(originalUnit)) { return rxPowerUnit }
    return originalUnit
  }

  const getDisplayValue = (value: number, originalUnit: string): number => {
    if (['dBm', 'dB', 'mW', 'nW'].includes(originalUnit)) { return convertRxPower(value, originalUnit, rxPowerUnit) }
    return value
  }

  return (
    <CardComponent className="relative overflow-hidden w-full flex flex-col h-full">
      <CardHeaderComponent className="pb-8">
        <div className="flex items-center gap-3">
          {isLoading ? (
            <>
              <Skeleton className="w-7 h-7 bg-muted rounded-md" />
              <Skeleton className="w-72 h-7 bg-muted rounded-md" />
            </>
          ) : (
            <>
              <Settings className="text-foreground" size={28} />
              <Label className="text-xl font-bold uppercase tracking-wider text-foreground">Limites dos Alarmes e Avisos</Label>
            </>
          )}
        </div>
      </CardHeaderComponent>

      <CardContentComponent className="flex-1">
        {isLoading ? (
          <div className="w-full border border-border rounded-lg overflow-hidden">
            <div className="grid grid-cols-[220px_1fr_1fr_1fr_1fr] bg-secondary/50">
              <div className="flex items-center justify-center px-3 py-2 border-r border-border">
                <Skeleton className="w-24 h-5 bg-muted rounded" />
              </div>
              {Array.from({ length: 4 }).map((_, i) => (
                <div key={i} className="flex flex-col items-center justify-center py-2 border-r border-border gap-1">
                  <Skeleton className="w-12 h-4 bg-muted rounded" />
                  <Skeleton className="w-10 h-4 bg-muted rounded" />
                </div>
              ))}
            </div>

            {Array.from({ length: 4 }).map((_, i) => (
              <div key={i} className={`grid grid-cols-[220px_1fr_1fr_1fr_1fr] ${i < 3 ? 'border-b border-border' : ''}`}>
                <div className="px-3 py-4 border-r border-border flex items-center gap-2">
                  <Skeleton className="w-32 h-5 bg-muted rounded" />
                </div>
                {Array.from({ length: 4 }).map((__, j) => (
                  <div key={j} className="flex items-center justify-center py-4 px-2 border-r border-border">
                    <Skeleton className="w-full h-7 bg-muted rounded" />
                  </div>
                ))}
              </div>
            ))}
          </div>
        ) : (
          <div className="w-full border border-border rounded-lg overflow-x-auto">
            <div className="min-w-[800px]">
              <div className="grid grid-cols-[220px_1fr_1fr_1fr_1fr] bg-secondary/50">
                <div className="flex items-center justify-center px-3 py-2 border-r border-border">
                  <span className="text-base font-semibold text-muted-foreground">Parâmetro</span>
                </div>
                <div className="flex flex-col items-center justify-center py-2 border-r border-border">
                  <span className="text-sm font-bold text-muted-foreground">Alarme</span>
                  <div className="px-2 py-0.5 bg-red-900 text-white rounded text-xs font-black uppercase mt-1">ALTO</div>
                </div>
                <div className="flex flex-col items-center justify-center py-2 border-r border-border">
                  <span className="text-sm font-bold text-muted-foreground">Aviso</span>
                  <div className="px-2 py-0.5 bg-yellow-500 text-zinc-900 rounded text-xs font-black uppercase mt-1">ALTO</div>
                </div>
                <div className="flex flex-col items-center justify-center py-2 border-r border-border">
                  <span className="text-sm font-bold text-muted-foreground">Aviso</span>
                  <div className="px-2 py-0.5 bg-yellow-500 text-zinc-900 rounded text-xs font-black uppercase mt-1">BAIXO</div>
                </div>
                <div className="flex flex-col items-center justify-center py-2 border-r border-border">
                  <span className="text-sm font-bold text-muted-foreground">Alarme</span>
                  <div className="px-2 py-0.5 bg-red-900 text-white rounded text-xs font-black uppercase mt-1">BAIXO</div>
                </div>
              </div>

              {limits.map((limit, index) => {
                const displayUnit = getDisplayUnit(limit.unit)
                const isRxPower = ['dBm', 'dB', 'mW', 'nW'].includes(limit.unit)
                const isLast = index === limits.length - 1

                return (
                  <div key={limit.id} className={`grid grid-cols-[220px_1fr_1fr_1fr_1fr] ${!isLast ? 'border-b border-border' : ''} hover:bg-secondary/30 transition-colors`}>
                    <div className="px-3 py-4 border-r border-border flex items-center gap-2">
                      <span className="text-base font-medium text-foreground">{limit.parameter.replace(/\([^)]*\)/, `(${displayUnit})`)}</span>
                      {isRxPower && (
                        <Select value={rxPowerUnit} onValueChange={(value) => setRxPowerUnit(value as RxPowerUnit)}>
                          <SelectTrigger size="sm" className="h-6 px-2 py-0 text-xs bg-secondary border-border text-foreground min-w-[60px]"><SelectValue /></SelectTrigger>
                          <SelectContent className="bg-background border-border">
                            {RX_POWER_UNITS.map((unit) => (<SelectItem key={unit} value={unit} className="text-foreground focus:bg-accent focus:text-accent-foreground">{unit}</SelectItem>))}
                          </SelectContent>
                        </Select>
                      )}
                    </div>

                    <div className="flex items-center justify-center py-4 px-2 border-r border-border">
                      <div className="w-full px-2 py-1 bg-red-900 text-white rounded text-base font-bold text-center">
                        {formatValue(getDisplayValue(limit.alarmHigh, limit.unit), displayUnit)}
                      </div>
                    </div>
                    <div className="flex items-center justify-center py-4 px-2 border-r border-border">
                      <div className="w-full px-2 py-1 bg-yellow-500 text-zinc-900 rounded text-base font-bold text-center">
                        {formatValue(getDisplayValue(limit.warningHigh, limit.unit), displayUnit)}
                      </div>
                    </div>
                    <div className="flex items-center justify-center py-4 px-2 border-r border-border">
                      <div className="w-full px-2 py-1 bg-yellow-500 text-zinc-900 rounded text-base font-bold text-center">
                        {formatValue(getDisplayValue(limit.warningLow, limit.unit), displayUnit)}
                      </div>
                    </div>
                    <div className="flex items-center justify-center py-4 px-2 border-r border-border">
                      <div className="w-full px-2 py-1 bg-red-900 text-white rounded text-base font-bold text-center">
                        {formatValue(getDisplayValue(limit.alarmLow, limit.unit), displayUnit)}
                      </div>
                    </div>
                  </div>
                )
              })}
            </div>
          </div>
        )}
      </CardContentComponent>

      <CardFooterComponent className="flex flex-col gap-3 items-end pt-6 border-t border-border">
        {isLoading ? (
          <>
            <div className="flex items-center gap-3"><Skeleton className="w-64 h-4 bg-muted rounded" /><Skeleton className="w-9 h-5 bg-muted rounded-full" /></div>
            <div className="flex items-center gap-3"><Skeleton className="w-80 h-4 bg-muted rounded" /><Skeleton className="w-9 h-5 bg-muted rounded-full" /></div>
          </>
        ) : (
          <>
            <div className="flex items-center gap-3">
              <Label htmlFor="global-notifications" className="text-sm font-medium text-foreground text-right cursor-pointer">Ativar notificações de flags em toda a dashboard</Label>
              <Switch id="global-notifications" defaultChecked />
            </div>
            <div className="flex items-center gap-3">
              <Label htmlFor="auto-calibration" className="text-sm font-medium text-foreground text-right cursor-pointer">Realizar calibração automática ao exceder limites</Label>
              <Switch id="auto-calibration" />
            </div>
          </>
        )}
      </CardFooterComponent>
    </CardComponent>
  )
}
