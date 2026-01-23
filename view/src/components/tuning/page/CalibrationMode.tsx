import { CheckCircle, Cog, SearchCheck, XCircle } from 'lucide-react'
import { Skeleton } from '../../ui/skeleton'
import {
  CardComponent as Card,
  CardContentComponent as CardContent,
  CardHeaderComponent as CardHeader,
  CardTitleComponent as CardTitle,
} from '@/components/ui/card'

interface CalibrationStatus {
  rx: boolean
  tx: boolean
  bias: boolean
  voltage: boolean
  temp: boolean
}

interface CalibrationModeProps {
  activeMode: 'internal' | 'external'
  onModeChange: (mode: 'internal' | 'external') => void
  status?: CalibrationStatus
  isLoading: boolean
}

export function CalibrationMode({
  activeMode,
  onModeChange,
  status,
  isLoading,
}: CalibrationModeProps) {
  return (
    <Card className="bg-zinc-900 border-zinc-800 h-full shadow-lg flex flex-col">
      <CardHeader className="pb-8 pt-8">
        {isLoading ? (
          <div className="flex gap-2 justify-center">
            <Skeleton className="w-6 h-6 bg-zinc-800 rounded-md" />
            <Skeleton className="w-56 h-6 bg-zinc-800 rounded-md" />
          </div>
        ) : (
          <CardTitle className="flex gap-2 justify-center text-center text-xl font-bold uppercase text-slate-300">
            <Cog />
            <span className="mt-0.5">Tipo de Calibração</span>
          </CardTitle>
        )}
      </CardHeader>
      <CardContent className="space-y-6 flex-1 flex flex-col p-6">
        {isLoading ? (
          <>
            {/* Selection Buttons Skeleton */}
            <div className="space-y-3">
              <Skeleton className="w-full h-20 bg-zinc-800 rounded-lg" />
              <Skeleton className="w-full h-20 bg-zinc-800 rounded-lg" />
            </div>

            {/* Status Section Skeleton */}
            <div className="space-y-4 pt-2 mt-8">
              <div className="flex gap-2 justify-center">
                <Skeleton className="w-6 h-6 bg-zinc-800 rounded-md" />
                <Skeleton className="w-48 h-6 bg-zinc-800 rounded-md" />
              </div>
              <div className="grid grid-cols-3 gap-3">
                <Skeleton className="w-full h-20 bg-zinc-800 rounded-lg" />
                <Skeleton className="w-full h-20 bg-zinc-800 rounded-lg" />
                <Skeleton className="w-full h-20 bg-zinc-800 rounded-lg" />
                <div className="col-span-3 flex gap-3 justify-center">
                  <Skeleton className="w-full max-w-[32%] h-20 bg-zinc-800 rounded-lg" />
                  <Skeleton className="w-full max-w-[32%] h-20 bg-zinc-800 rounded-lg" />
                </div>
              </div>
              <Skeleton className="w-full h-12 bg-zinc-800 rounded-lg" />
            </div>
          </>
        ) : (
          <>
            {/* Selection Buttons Container */}
            <div className="space-y-3">
              <div
                onClick={() => onModeChange('internal')}
                className={`
                  p-4 rounded-lg border flex items-center justify-between group cursor-pointer transition-all duration-200
                  ${
                    activeMode === 'internal'
                      ? 'bg-zinc-800/60 border-zinc-600 shadow-md'
                      : 'bg-zinc-950/40 border-zinc-800 hover:border-zinc-700 hover:bg-zinc-900/60'
                  }
                `}
              >
                <div className="flex items-center gap-4">
                  <div className="relative flex items-center justify-center">
                    {activeMode === 'internal' ? (
                      <div className="w-4 h-4 rounded-full border-[2px] border-zinc-500 bg-slate-300 shadow-[0_0_8px_rgba(203,213,225,0.4)]" />
                    ) : (
                      <div className="w-4 h-4 rounded-full border-2 border-zinc-700 group-hover:border-zinc-500" />
                    )}
                  </div>
                  <div>
                    <div
                      className={`font-bold text-base ${activeMode === 'internal' ? 'text-white' : 'text-slate-400 group-hover:text-slate-200'}`}
                    >
                      CALIBRAÇÃO INTERNA
                    </div>
                    <div className="text-[11px] bg-zinc-800 text-zinc-400 px-1.5 py-0.5 rounded w-fit mt-1 uppercase tracking-wider font-bold">
                      RECOMENDADO
                    </div>
                  </div>
                </div>
              </div>

              <div
                onClick={() => onModeChange('external')}
                className={`
                  p-4 rounded-lg border flex items-center justify-between group cursor-pointer transition-all duration-200
                  ${
                    activeMode === 'external'
                      ? 'bg-zinc-800/60 border-zinc-600 shadow-md'
                      : 'bg-zinc-950/40 border-zinc-800 hover:border-zinc-700 hover:bg-zinc-900/60'
                  }
                `}
              >
                <div className="flex items-center gap-4">
                  <div className="relative flex items-center justify-center">
                    {activeMode === 'external' ? (
                      <div className="w-4 h-4 rounded-full border-[2px] border-zinc-500 bg-slate-300 shadow-[0_0_8px_rgba(203,213,225,0.4)]" />
                    ) : (
                      <div className="w-4 h-4 rounded-full border-2 border-zinc-700 group-hover:border-zinc-500" />
                    )}
                  </div>
                  <div>
                    <div
                      className={`font-bold text-base ${activeMode === 'external' ? 'text-white' : 'text-slate-400 group-hover:text-slate-200'}`}
                    >
                      CALIBRAÇÃO EXTERNA
                    </div>
                    <div className="text-[11px] border border-amber-900/30 text-amber-600 px-1.5 py-0.5 rounded w-fit mt-1 uppercase tracking-wider font-bold">
                      AVANÇADO
                    </div>
                  </div>
                </div>
              </div>
            </div>

            {/* Calibration Status */}
            {status && (
              <div className="space-y-4 pt-2 mt-8">
                <h4 className="flex gap-2 justify-center text-[16px] font-bold text-slate-500 uppercase tracking-widest text-center">
                  <SearchCheck />
                  Status da Calibração
                </h4>
                <div className="grid grid-cols-3 gap-3">
                  <StatusBadge label="RX" valid={status.rx} />
                  <StatusBadge label="TX" valid={status.tx} />
                  <StatusBadge label="Bias" valid={status.bias} />

                  {/* Centering the last two items if grid is 3 cols */}
                  <div className="col-span-3 flex gap-3 justify-center">
                    <div className="w-full max-w-[32%]">
                      <StatusBadge label="Volt" valid={status.voltage} />
                    </div>
                    <div className="w-full max-w-[32%]">
                      <StatusBadge label="Temp" valid={status.temp} />
                    </div>
                  </div>
                </div>

                <div className="mt-4 flex items-center justify-center gap-2 bg-zinc-950/50 py-3 rounded-lg border border-green-900/30 shadow-inner">
                  <CheckCircle className="w-4 h-4 text-green-500" />
                  <span className="text-xs text-green-500 font-bold uppercase tracking-wide">
                    Calibração Válida
                  </span>
                </div>
              </div>
            )}
          </>
        )}
      </CardContent>
    </Card>
  )
}

function StatusBadge({ label, valid }: { label: string; valid: boolean }) {
  return (
    <div className="flex flex-col items-center justify-center p-3 rounded-lg bg-zinc-950/50 border border-zinc-800">
      <span className="text-[12px] text-zinc-500 font-bold uppercase tracking-wider mb-2">
        {label}
      </span>
      {valid ? (
        <CheckCircle className="w-5 h-5 text-green-500" />
      ) : (
        <XCircle className="w-5 h-5 text-red-500" />
      )}
    </div>
  )
}
