import { useStore } from '@tanstack/react-store'
import { Check, Settings, SquarePen, X } from 'lucide-react'
import { useState } from 'react'
import { Store } from '@tanstack/store'
import { Skeleton } from '../../ui/skeleton'
import { Label } from '../../ui/label'
import {
  CardComponent,
  CardContentComponent,
  CardFooterComponent,
  CardHeaderComponent,
} from '@/components/ui/card'
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select'
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
  {
    id: '1',
    parameter: 'Temperatura (°C)',
    alarmHigh: 33,
    warningHigh: 33,
    warningLow: 33,
    alarmLow: 33,
    unit: '°C',
  },
  {
    id: '2',
    parameter: 'Tensão (V)',
    alarmHigh: 3.33,
    warningHigh: 3.33,
    warningLow: 3.33,
    alarmLow: 3.33,
    unit: 'V',
  },
  {
    id: '3',
    parameter: 'Corrente Bias (mA)',
    alarmHigh: 32.04,
    warningHigh: 32.04,
    warningLow: 32.04,
    alarmLow: 32.04,
    unit: 'mA',
  },
  {
    id: '4',
    parameter: 'RX Power (dBm)',
    alarmHigh: -8.0,
    warningHigh: -8.0,
    warningLow: -8.0,
    alarmLow: -8.0,
    unit: 'dBm',
  },
])

export function Limits({ isLoading }: { isLoading: boolean }) {
  const limits = useStore(limitsStore)
  const [editingId, setEditingId] = useState<string | null>(null)
  const [editValues, setEditValues] = useState<AlarmLimit | null>(null)
  const [rxPowerUnit, setRxPowerUnit] = useState<RxPowerUnit>('dBm')

  const getDecimalPlaces = (unit: string): number => {
    if (unit === '°C') return 1
    if (unit === 'V') return 2
    if (unit === 'mA') return 2
    if (['dBm', 'dB', 'mW', 'nW'].includes(unit)) return 2
    return 2
  }

  const getStep = (unit: string): string => {
    if (unit === '°C') return '0.1'
    return '0.01'
  }

  const formatValue = (value: number, unit: string) => {
    const decimals = getDecimalPlaces(unit)
    return value.toFixed(decimals)
  }

  const convertRxPower = (
    value: number,
    fromUnit: string,
    toUnit: RxPowerUnit,
  ): number => {
    // Convert to dBm first
    let dBm = value
    if (fromUnit === 'mW') {
      dBm = 10 * Math.log10(value)
    } else if (fromUnit === 'nW') {
      dBm = 10 * Math.log10(value / 1e6)
    } else if (fromUnit === 'dB') {
      dBm = value // dB is relative, treating as dBm for simplicity
    }

    // Convert from dBm to target unit
    if (toUnit === 'dBm' || toUnit === 'dB') {
      return dBm
    } else if (toUnit === 'mW') {
      return Math.pow(10, dBm / 10)
    } else {
      return Math.pow(10, dBm / 10) * 1e6
    }
    return value
  }

  const getDisplayUnit = (originalUnit: string): string => {
    if (['dBm', 'dB', 'mW', 'nW'].includes(originalUnit)) {
      return rxPowerUnit
    }
    return originalUnit
  }

  const getDisplayValue = (value: number, originalUnit: string): number => {
    if (['dBm', 'dB', 'mW', 'nW'].includes(originalUnit)) {
      return convertRxPower(value, originalUnit, rxPowerUnit)
    }
    return value
  }

  const handleEdit = (limit: AlarmLimit) => {
    setEditingId(limit.id)
    setEditValues({ ...limit })
  }

  const handleSave = () => {
    if (!editValues) return

    limitsStore.setState(
      limits.map((limit) => (limit.id === editingId ? editValues : limit)),
    )
    setEditingId(null)
    setEditValues(null)
  }

  const handleCancel = () => {
    setEditingId(null)
    setEditValues(null)
  }

  const handleInputChange = (
    field: keyof Omit<AlarmLimit, 'id' | 'parameter' | 'unit'>,
    value: string,
    unit: string,
  ) => {
    if (!editValues) return

    const numValue = parseFloat(value)
    if (!isNaN(numValue)) {
      const decimals = getDecimalPlaces(unit)
      const clampedValue = parseFloat(numValue.toFixed(decimals))
      setEditValues({ ...editValues, [field]: clampedValue })
    }
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
              <Label className="text-xl font-bold uppercase tracking-wider text-foreground">
                Limites dos Alarmes e Avisos
              </Label>
            </>
          )}
        </div>
      </CardHeaderComponent>

      <CardContentComponent className="flex-1">
        {isLoading ? (
          <div className="w-full border border-border rounded-lg overflow-hidden">
            {/* Skeleton Header */}
            <div className="grid grid-cols-[220px_1fr_1fr_1fr_1fr_50px] bg-secondary/50">
              <div className="flex items-center justify-center px-3 py-2 border-r border-border">
                <Skeleton className="w-24 h-5 bg-muted rounded" />
              </div>
              {Array.from({ length: 4 }).map((_, i) => (
                <div
                  key={i}
                  className="flex flex-col items-center justify-center py-2 border-r border-border gap-1"
                >
                  <Skeleton className="w-12 h-4 bg-muted rounded" />
                  <Skeleton className="w-10 h-4 bg-muted rounded" />
                </div>
              ))}
              <div className="flex items-center justify-center">
                {/* Empty header action */}
              </div>
            </div>

            {/* Skeleton Rows */}
            {Array.from({ length: 4 }).map((_, i) => (
              <div
                key={i}
                className={`grid grid-cols-[220px_1fr_1fr_1fr_1fr_50px] ${i < 3 ? 'border-b border-border' : ''}`}
              >
                {/* Parameter */}
                <div className="px-3 py-4 border-r border-border flex items-center gap-2">
                  <Skeleton className="w-32 h-5 bg-muted rounded" />
                </div>

                {/* Values columns */}
                {Array.from({ length: 4 }).map((__, j) => (
                  <div
                    key={j}
                    className="flex items-center justify-center py-4 px-2 border-r border-border"
                  >
                    <Skeleton className="w-full h-7 bg-muted rounded" />
                  </div>
                ))}

                {/* Action */}
                <div className="flex items-center justify-center py-2">
                  <Skeleton className="w-6 h-6 bg-muted rounded" />
                </div>
              </div>
            ))}
          </div>
        ) : (
          <div className="w-full border border-border rounded-lg overflow-x-auto">
            <div className="min-w-[800px]">
              {/* Table Header */}
              <div className="grid grid-cols-[220px_1fr_1fr_1fr_1fr_50px] bg-secondary/50">
                <div className="flex items-center justify-center px-3 py-2 border-r border-border">
                  <span className="text-base font-semibold text-muted-foreground">
                    Parâmetro
                  </span>
                </div>
                <div className="flex flex-col items-center justify-center py-2 border-r border-border">
                  <span className="text-sm font-bold text-muted-foreground">Alarme</span>
                  <div className="px-2 py-0.5 bg-red-900 text-white rounded text-xs font-black uppercase mt-1">
                    ALTO
                  </div>
                </div>
                <div className="flex flex-col items-center justify-center py-2 border-r border-border">
                  <span className="text-sm font-bold text-muted-foreground">Aviso</span>
                  <div className="px-2 py-0.5 bg-yellow-500 text-zinc-900 rounded text-xs font-black uppercase mt-1">
                    ALTO
                  </div>
                </div>
                <div className="flex flex-col items-center justify-center py-2 border-r border-border">
                  <span className="text-sm font-bold text-muted-foreground">Aviso</span>
                  <div className="px-2 py-0.5 bg-yellow-500 text-zinc-900 rounded text-xs font-black uppercase mt-1">
                    BAIXO
                  </div>
                </div>
                <div className="flex flex-col items-center justify-center py-2 border-r border-border">
                  <span className="text-sm font-bold text-muted-foreground">Alarme</span>
                  <div className="px-2 py-0.5 bg-red-900 text-white rounded text-xs font-black uppercase mt-1">
                    BAIXO
                  </div>
                </div>
                <div className="flex items-center justify-center">
                  {/* Empty header for actions */}
                </div>
              </div>

              {/* Table Body */}
              {limits.map((limit, index) => {
                const isEditing = editingId === limit.id
                const currentValues = isEditing ? editValues! : limit
                const displayUnit = getDisplayUnit(limit.unit)
                const isRxPower = ['dBm', 'dB', 'mW', 'nW'].includes(limit.unit)
                const step = getStep(limit.unit)
                const isLast = index === limits.length - 1

                return (
                  <div
                    key={limit.id}
                    className={`grid grid-cols-[220px_1fr_1fr_1fr_1fr_50px] ${!isLast ? 'border-b border-border' : ''} hover:bg-secondary/30 transition-colors`}
                  >
                    {/* Parameter Name */}
                    <div className="px-3 py-4 border-r border-border flex items-center gap-2">
                      <span className="text-base font-medium text-foreground">
                        {limit.parameter.replace(/\([^)]*\)/, `(${displayUnit})`)}
                      </span>
                      {isRxPower && !isEditing && (
                        <Select
                          value={rxPowerUnit}
                          onValueChange={(value) =>
                            setRxPowerUnit(value as RxPowerUnit)
                          }
                        >
                          <SelectTrigger
                            size="sm"
                            className="h-6 px-2 py-0 text-xs bg-secondary border-border text-foreground min-w-[60px]"
                          >
                            <SelectValue />
                          </SelectTrigger>
                          <SelectContent className="bg-background border-border">
                            {RX_POWER_UNITS.map((unit) => (
                              <SelectItem
                                key={unit}
                                value={unit}
                                className="text-foreground focus:bg-accent focus:text-accent-foreground"
                              >
                                {unit}
                              </SelectItem>
                            ))}
                          </SelectContent>
                        </Select>
                      )}
                    </div>

                    {/* Alarme Alto */}
                    <div className="flex items-center justify-center py-4 px-2 border-r border-border">
                      {isEditing ? (
                        <input
                          type="number"
                          step={step}
                          value={currentValues.alarmHigh}
                          onChange={(e) =>
                            handleInputChange(
                              'alarmHigh',
                              e.target.value,
                              limit.unit,
                            )
                          }
                          className="w-full px-1 py-1 bg-red-900 text-white rounded text-base font-bold text-center border border-red-800 focus:outline-none focus:border-red-600"
                        />
                      ) : (
                        <div className="w-full px-2 py-1 bg-red-900 text-white rounded text-base font-bold text-center">
                          {formatValue(
                            getDisplayValue(limit.alarmHigh, limit.unit),
                            displayUnit,
                          )}
                        </div>
                      )}
                    </div>

                    {/* Aviso Alto */}
                    <div className="flex items-center justify-center py-4 px-2 border-r border-border">
                      {isEditing ? (
                        <input
                          type="number"
                          step={step}
                          value={currentValues.warningHigh}
                          onChange={(e) =>
                            handleInputChange(
                              'warningHigh',
                              e.target.value,
                              limit.unit,
                            )
                          }
                          className="w-full px-1 py-1 bg-yellow-500 text-zinc-900 rounded text-base font-bold text-center border border-yellow-300 focus:outline-none focus:border-yellow-200"
                        />
                      ) : (
                        <div className="w-full px-2 py-1 bg-yellow-500 text-zinc-900 rounded text-base font-bold text-center">
                          {formatValue(
                            getDisplayValue(limit.warningHigh, limit.unit),
                            displayUnit,
                          )}
                        </div>
                      )}
                    </div>

                    {/* Aviso Baixo */}
                    <div className="flex items-center justify-center py-4 px-2 border-r border-border">
                      {isEditing ? (
                        <input
                          type="number"
                          step={step}
                          value={currentValues.warningLow}
                          onChange={(e) =>
                            handleInputChange(
                              'warningLow',
                              e.target.value,
                              limit.unit,
                            )
                          }
                          className="w-full px-1 py-1 bg-yellow-500 text-zinc-900 rounded text-base font-bold text-center border border-yellow-300 focus:outline-none focus:border-yellow-200"
                        />
                      ) : (
                        <div className="w-full px-2 py-1 bg-yellow-500 text-zinc-900 rounded text-base font-bold text-center">
                          {formatValue(
                            getDisplayValue(limit.warningLow, limit.unit),
                            displayUnit,
                          )}
                        </div>
                      )}
                    </div>

                    {/* Alarme Baixo */}
                    <div className="flex items-center justify-center py-4 px-2 border-r border-border">
                      {isEditing ? (
                        <input
                          type="number"
                          step={step}
                          value={currentValues.alarmLow}
                          onChange={(e) =>
                            handleInputChange(
                              'alarmLow',
                              e.target.value,
                              limit.unit,
                            )
                          }
                          className="w-full px-1 py-1 bg-red-900 text-white rounded text-base font-bold text-center border border-red-800 focus:outline-none focus:border-red-600"
                        />
                      ) : (
                        <div className="w-full px-2 py-1 bg-red-900 text-white rounded text-base font-bold text-center">
                          {formatValue(
                            getDisplayValue(limit.alarmLow, limit.unit),
                            displayUnit,
                          )}
                        </div>
                      )}
                    </div>

                    {/* Actions */}
                    <div className="flex items-center justify-center py-2">
                      {isEditing ? (
                        <div className="flex flex-col gap-1">
                          <button
                            onClick={handleSave}
                            className="flex items-center justify-center w-5 h-5 rounded hover:bg-green-900/30 transition-colors text-green-500 hover:text-green-400"
                            aria-label="Salvar"
                          >
                            <Check size={20} />
                          </button>
                          <button
                            onClick={handleCancel}
                            className="flex items-center justify-center w-5 h-5 rounded hover:bg-red-900/30 transition-colors text-red-500 hover:text-red-400"
                            aria-label="Cancelar"
                          >
                            <X size={20} />
                          </button>
                        </div>
                      ) : (
                        <button
                          onClick={() => handleEdit(limit)}
                          className="flex items-center justify-center w-6 h-6 rounded hover:bg-zinc-700 transition-colors text-muted-foreground hover:text-foreground"
                          aria-label="Editar limites"
                        >
                          <SquarePen size={20} />
                        </button>
                      )}
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
            <div className="flex items-center gap-3">
              <Skeleton className="w-64 h-4 bg-muted rounded" />
              <Skeleton className="w-9 h-5 bg-muted rounded-full" />
            </div>
            <div className="flex items-center gap-3">
              <Skeleton className="w-80 h-4 bg-muted rounded" />
              <Skeleton className="w-9 h-5 bg-muted rounded-full" />
            </div>
          </>
        ) : (
          <>
            <div className="flex items-center gap-3">
              <Label
                htmlFor="global-notifications"
                className="text-sm font-medium text-foreground text-right cursor-pointer"
              >
                Ativar notificações de flags em toda a dashboard
              </Label>
              <Switch id="global-notifications" defaultChecked />
            </div>
            <div className="flex items-center gap-3">
              <Label
                htmlFor="auto-calibration"
                className="text-sm font-medium text-foreground text-right cursor-pointer"
              >
                Realizar calibração automática ao exceder limites
              </Label>
              <Switch id="auto-calibration" />
            </div>
          </>
        )}
      </CardFooterComponent>
    </CardComponent>
  )
}
