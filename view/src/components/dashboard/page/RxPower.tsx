import { useStore } from '@tanstack/react-store'
import { BatteryMedium } from 'lucide-react'
import { useState } from 'react'
import { Skeleton } from '../../ui/skeleton'
import { Label } from '../../ui/label'
import {
  CardComponent,
  CardContentComponent,
  CardHeaderComponent,
} from '@/components/ui/card'
import { rxPowerStore } from '@/stores/dataStores'
import { cn } from '@/lib/utils'

export function RxPower({ isLoading }: { isLoading: boolean }) {
  const powerValue = useStore(rxPowerStore)
  const [viewMode, setViewMode] = useState<'gauge' | 'digital'>('gauge')
  const [unit, setUnit] = useState<'dBm' | 'dB' | 'mW' | 'µW' | 'nW'>('dBm')
  const reference = 0

  // Calculate linear power in mW
  const powerInMW = Math.pow(10, powerValue / 10)

  const getDisplayValue = () => {
    switch (unit) {
      case 'dBm':
        return powerValue.toFixed(2)
      case 'dB':
        return (powerValue - reference).toFixed(2)
      case 'mW':
        return powerInMW.toFixed(4)
      case 'µW':
        return (powerInMW * 1000).toFixed(2)
      case 'nW':
        return (powerInMW * 1000000).toFixed(0)
      default:
        return powerValue.toFixed(2)
    }
  }

  return (
    <CardComponent className="relative overflow-hidden w-full flex flex-col">
      <CardHeaderComponent className="pb-0!">
        <div className="flex flex-col gap-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              {isLoading ? (
                <>
                  <Skeleton className="w-6 h-6 bg-zinc-800 rounded-md" />
                  <Skeleton className="w-48 h-6 bg-zinc-800 rounded-md" />
                </>
              ) : (
                <>
                  <BatteryMedium className="text-slate-300" size={24} />
                  <Label className="text-lg font-bold uppercase tracking-wider text-slate-300">
                    Potência Óptica (RX)
                  </Label>
                </>
              )}
            </div>

            <div className="flex items-center gap-2">
              {/* View Mode Selector */}
              <div className="flex items-center p-1 bg-zinc-900/50 rounded-md border border-zinc-800">
                {isLoading ? (
                  <div className="flex items-center gap-1">
                    <Skeleton className="w-[52px] h-[22px] rounded-sm bg-zinc-800" />
                    <Skeleton className="w-[52px] h-[22px] rounded-sm bg-zinc-800" />
                  </div>
                ) : (
                  <>
                    <button
                      onClick={() => setViewMode('gauge')}
                      className={cn(
                        'px-3 py-1 text-[10px] font-bold uppercase transition-all rounded-sm',
                        viewMode === 'gauge'
                          ? 'bg-zinc-800 text-slate-300 shadow-sm'
                          : 'text-slate-400 hover:text-slate-400',
                      )}
                    >
                      Medidor
                    </button>
                    <button
                      onClick={() => setViewMode('digital')}
                      className={cn(
                        'px-3 py-1 text-[10px] font-bold uppercase transition-all rounded-sm',
                        viewMode === 'digital'
                          ? 'bg-zinc-800 text-slate-300 shadow-sm'
                          : 'text-slate-400 hover:text-slate-400',
                      )}
                    >
                      Digital
                    </button>
                  </>
                )}
              </div>
            </div>
          </div>

          {/* Unit Selector */}
          <div className="flex items-center gap-3 overflow-x-auto no-scrollbar">
            {isLoading
              ? Array.from({ length: 5 }).map((_, i) => (
                  <Skeleton key={i} className="w-10 h-5 rounded bg-zinc-800" />
                ))
              : (['dBm', 'dB', 'mW', 'µW', 'nW'] as const).map((u) => (
                  <button
                    key={u}
                    onClick={() => setUnit(u)}
                    className={cn(
                      'px-2 py-0.5 text-[11px] font-black uppercase border transition-all rounded',
                      unit === u
                        ? 'bg-slate-300 text-zinc-950 border-slate-300'
                        : 'bg-transparent text-slate-400 border-zinc-800 hover:border-zinc-700',
                    )}
                  >
                    {u}
                  </button>
                ))}
          </div>
        </div>
      </CardHeaderComponent>

      <CardContentComponent className="min-h-48 flex-1 flex flex-col items-center justify-center">
        {isLoading ? (
          <div className="flex flex-col items-center justify-center gap-4">
            <div className="relative w-64 flex items-center justify-center">
              <Skeleton className="w-56 h-56 rounded-full bg-zinc-800" />
            </div>
          </div>
        ) : (
          <div className="w-full flex-1 flex flex-col items-center justify-center">
            {viewMode === 'gauge' ? (
              <div className="flex flex-col items-center justify-center animate-in fade-in duration-300">
                <div className="relative w-64 flex items-center justify-center">
                  <svg className="w-56 -rotate-90" viewBox="0 0 100 100">
                    <circle
                      cx="50"
                      cy="50"
                      r="42"
                      fill="none"
                      stroke="#1e1e24"
                      strokeWidth="6"
                      strokeDasharray="1 2"
                    />
                    <circle
                      cx="50"
                      cy="50"
                      r="42"
                      fill="none"
                      stroke="rgb(203 213 225)"
                      strokeWidth="6"
                      strokeDasharray={`${(Math.max(0, 20 + powerValue) / 20) * 263.89} 263.89`}
                      className="transition-all duration-1000"
                    />
                  </svg>
                  <div className="absolute inset-0 flex flex-col items-center justify-center">
                    <span className="text-4xl font-bold text-slate-300 tracking-tighter">
                      {getDisplayValue()}
                    </span>
                    <span className="text-xs text-slate-400 font-bold uppercase tracking-[0.2em] mt-1">
                      {unit}
                    </span>
                  </div>
                </div>
              </div>
            ) : (
              <div className="flex flex-col items-center justify-center p-8 w-full animate-in fade-in duration-300">
                <div className="flex flex-col items-center gap-4">
                  <div className="flex flex-col items-center">
                    <span className="text-7xl font-bold text-slate-300 tracking-tighter leading-none">
                      {getDisplayValue()}
                    </span>
                    <span className="text-xl font-bold text-slate-400 tracking-widest uppercase mt-2">
                      {unit}
                    </span>
                  </div>
                  <div className="h-px w-48 bg-zinc-800" />

                  <div className="grid grid-cols-2 gap-x-12 gap-y-2">
                    <div className="flex flex-col items-center">
                      <span className="text-[9px] text-zinc-600 uppercase font-black tracking-widest">
                        dBm
                      </span>
                      <span className="text-xs font-bold text-slate-400">
                        {powerValue.toFixed(2)}
                      </span>
                    </div>
                    <div className="flex flex-col items-center">
                      <span className="text-[9px] text-zinc-600 uppercase font-black tracking-widest">
                        Linear
                      </span>
                      <span className="text-xs font-bold text-slate-400">
                        {powerInMW >= 1
                          ? `${powerInMW.toFixed(2)} mW`
                          : `${(powerInMW * 1000).toFixed(1)} µW`}
                      </span>
                    </div>
                  </div>
                </div>
              </div>
            )}
          </div>
        )}
      </CardContentComponent>
    </CardComponent>
  )
}
