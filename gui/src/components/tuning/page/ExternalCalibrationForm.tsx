import { BookText, Check, SquarePen, SquareSigma, X } from 'lucide-react'
import { useEffect, useRef, useState } from 'react'
import { Skeleton } from '../../ui/skeleton'
import {
  CardComponent as Card,
  CardContentComponent as CardContent,
  CardHeaderComponent as CardHeader,
  CardTitleComponent as CardTitle,
} from '@/components/ui/card'
import { Input } from '@/components/ui/input'
import { Label } from '@/components/ui/label'
import { calibrationStore } from '@/stores/calibrationStore'

interface CalibrationInfo {
  ddmType: string
  version: string
  rxPowerType: string
  addressChange: string
}

interface ExternalCalibrationProps {
  rxPower: { offset: string; slope: string; pwr4: string; pwr3: string }
  txPower: { offset: string; slope: string }
  temperature: { offset: string; slope: string }
  info?: CalibrationInfo
  isLoading: boolean
}

export function ExternalCalibrationForm({
  rxPower,
  txPower,
  temperature,
  info,
  isLoading,
}: ExternalCalibrationProps) {
  // Generic update handler
  const handleUpdate = (
    section: 'rxPower' | 'txPower' | 'temperature',
    key: string,
    value: string,
  ) => {
    calibrationStore.setState((state) => {
      return {
        ...state,
        [section]: {
          ...state[section],
          [key]: value,
        },
      }
    })
  }

  return (
    <Card className="bg-secondary/20 border-border h-full shadow-lg flex flex-col">
      <CardHeader className="text-center pb-8 pt-8">
        {isLoading ? (
          <div className="flex gap-2 justify-center">
            <Skeleton className="w-6 h-6 bg-muted rounded-md" />
            <Skeleton className="w-64 h-6 bg-muted rounded-md" />
          </div>
        ) : (
          <CardTitle className="flex gap-2 justify-center text-foreground text-xl font-bold uppercase">
            <SquareSigma />
            <span className="mt-0.5">Coeficientes de Calibração</span>
          </CardTitle>
        )}
      </CardHeader>
      <CardContent className="p-5 flex-1 flex flex-col justify-between">
        {isLoading ? (
          <>
            {/* RX Power Section Skeleton */}
            <div className="space-y-3">
              <Skeleton className="w-24 h-5 bg-muted rounded-md" />
              <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3">
                <Skeleton className="w-full h-10 bg-muted rounded-md" />
                <Skeleton className="w-full h-10 bg-muted rounded-md" />
                <Skeleton className="w-full h-10 bg-muted rounded-md" />
                <Skeleton className="w-full h-10 bg-muted rounded-md" />
              </div>
            </div>

            {/* TX Power Section Skeleton */}
            <div className="space-y-3">
              <Skeleton className="w-24 h-5 bg-muted rounded-md" />
              <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3">
                <Skeleton className="w-full h-10 bg-muted rounded-md" />
                <div className="col-span-1 lg:col-span-2">
                  <Skeleton className="w-full h-10 bg-muted rounded-md" />
                </div>
              </div>
            </div>

            {/* Temperature Section Skeleton */}
            <div className="space-y-3">
              <Skeleton className="w-32 h-5 bg-muted rounded-md" />
              <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3">
                <Skeleton className="w-full h-10 bg-muted rounded-md" />
                <div className="col-span-1 lg:col-span-2">
                  <Skeleton className="w-full h-10 bg-muted rounded-md" />
                </div>
              </div>
            </div>

            {/* Module Info Section Skeleton */}
            <div className="pt-6 border-t border-border/50 space-y-3">
              <div className="flex gap-2 justify-center pb-5">
                <Skeleton className="w-6 h-5 bg-muted rounded-md" />
                <Skeleton className="w-48 h-5 bg-muted rounded-md" />
              </div>
              <div className="grid grid-cols-2 lg:grid-cols-4 gap-2">
                <Skeleton className="w-full h-10 bg-muted rounded-md" />
                <Skeleton className="w-full h-10 bg-muted rounded-md" />
                <Skeleton className="w-full h-10 bg-muted rounded-md" />
                <Skeleton className="w-full h-10 bg-muted rounded-md" />
              </div>
            </div>
          </>
        ) : (
          <>
            {/* RX Power Section */}
            <div className="space-y-3">
              <div className="text-[0.9rem] font-bold text-muted-foreground uppercase tracking-widest pl-1">
                RX Power
              </div>
              <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3">
                <InputGroup label="Offset" pl="0" val={rxPower.offset} />
                <InputGroup label="Slope" pl="1" val={rxPower.slope} />
                <InputGroup label="RX_pwr(4)" pl="0" val={rxPower.pwr4} />
                <div className="relative">
                  <InputGroup label="RX_pwr(3)" pl="0" val={rxPower.pwr3} hasEdit onSave={(val) => handleUpdate('rxPower', 'pwr3', val)}
                  />
                </div>
              </div>
            </div>

            {/* TX Power Section */}
            <div className="space-y-3">
              <div className="text-[0.9rem] font-bold text-muted-foreground uppercase tracking-widest pl-1">
                TX Power
              </div>
              <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3">
                <div className="col-span-1">
                  <InputGroup label="Offset" pl="0" val={txPower.offset} />
                </div>
                <div className="col-span-1 lg:col-span-2">
                  <InputGroup label="Slope" pl="1" val={txPower.slope} hasEdit onSave={(val) => handleUpdate('txPower', 'slope', val)}
                  />
                </div>
              </div>
            </div>

            {/* Temperature Section */}
            <div className="space-y-3">
              <div className="text-[0.9rem] font-bold text-muted-foreground uppercase tracking-widest pl-1">
                Temperatura
              </div>
              <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3">
                <div className="col-span-1">
                  <InputGroup label="Offset (°C)" pl="0" val={temperature.offset} />
                </div>
                <div className="col-span-1 lg:col-span-2">
                  <InputGroup label="Slope" pl="1" val={temperature.slope} hasEdit onSave={(val) => handleUpdate('temperature', 'slope', val)}
                  />
                </div>
              </div>
            </div>

            {/* Module Info Section (Moved from Selector) */}
            {info && (
              <div className="pt-6 border-t border-border/50 space-y-3">
                <h4 className="flex gap-2 justify-center text-[0.9rem] pb-5 font-bold text-muted-foreground uppercase tracking-widest text-center mb-2">
                  <BookText />
                  Informações do Módulo
                </h4>
                <div className="grid grid-cols-2 lg:grid-cols-4 gap-2">
                  <div className="text-center">
                    <Label className="text-[12px] text-muted-foreground uppercase font-bold tracking-wider block mb-0.5">
                      Tipo DDM
                    </Label>
                    <div className="font-mono text-xs text-foreground">
                      {info.ddmType}
                    </div>
                  </div>
                  <div className="text-center">
                    <Label className="text-[12px] text-muted-foreground uppercase font-bold tracking-wider block mb-0.5">
                      Versão
                    </Label>
                    <div className="font-mono text-xs text-foreground">
                      {info.version}
                    </div>
                  </div>
                  <div className="text-center">
                    <Label className="text-[12px] text-muted-foreground uppercase font-bold tracking-wider block mb-0.5">
                      RX Pwr Type
                    </Label>
                    <div className="font-mono text-xs text-foreground">
                      {info.rxPowerType}
                    </div>
                  </div>
                  <div className="text-center">
                    <Label className="text-[12px] text-muted-foreground uppercase font-bold tracking-wider block mb-0.5">
                      Endereçamento
                    </Label>
                    <div className="font-mono text-xs text-foreground">
                      {info.addressChange}
                    </div>
                  </div>
                </div>
              </div>
            )}
          </>
        )}
      </CardContent>
    </Card>
  )
}

function InputGroup({
  label,
  pl,
  val,
  hasEdit,
  onSave,
}: {
  label: string
  pl: string
  val: string
  hasEdit?: boolean
  onSave?: (newValue: string) => void
}) {
  const [isEditing, setIsEditing] = useState(false)
  const [tempValue, setTempValue] = useState(val)
  const inputRef = useRef<HTMLInputElement>(null)

  // Sync tempValue with prop val when not editing, to handle external updates
  useEffect(() => {
    if (!isEditing) {
      setTempValue(val)
    }
  }, [val, isEditing])

  useEffect(() => {
    if (isEditing && inputRef.current) {
      inputRef.current.focus()
    }
  }, [isEditing])

  const handleSave = () => {
    onSave?.(tempValue)
    setIsEditing(false)
  }

  const handleCancel = () => {
    setTempValue(val)
    setIsEditing(false)
  }

  return (
    <div className="space-y-0.5 flex flex-col">
      <Label className="text-[13px] text-muted-foreground font-normal ml-0.5">
        {label}
      </Label>
      <div className="flex gap-2 relative">
        <Input ref={inputRef} className={`bg-secondary/40 border-border text-foreground h-8 text-xs font-medium ${isEditing ? 'border-primary ring-1 ring-primary/50' : ''} `} placeholder={pl} value={isEditing ? tempValue : val} onChange={(e) => isEditing && setTempValue(e.target.value)}
          readOnly={!isEditing}
        />
        {hasEdit && !isEditing && (
          <button onClick={() => setIsEditing(true)}
            className="h-8 w-8 flex shrink-0 items-center justify-center bg-secondary/40 rounded border border-border hover:bg-secondary/60 cursor-pointer transition-colors"
          >
            <SquarePen className="w-4.5 h-4.5 text-muted-foreground" />
          </button>
        )}
        {hasEdit && isEditing && (
          <div className="flex gap-1">
            <button onClick={handleSave} className="h-8 w-8 flex shrink-0 items-center justify-center bg-green-900/40 rounded border border-green-800 hover:bg-green-900/60 cursor-pointer transition-colors">
              <Check className="w-4 h-4 text-green-400" />
            </button>
            <button onClick={handleCancel} className="h-8 w-8 flex shrink-0 items-center justify-center bg-red-900/40 rounded border border-red-800 hover:bg-red-900/60 cursor-pointer transition-colors">
              <X className="w-4 h-4 text-red-400" />
            </button>
          </div>
        )}
      </div>
    </div>
  )
}
