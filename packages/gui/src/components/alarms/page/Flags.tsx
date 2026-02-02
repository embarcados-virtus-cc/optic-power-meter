import { useStore } from '@tanstack/react-store'
import { FlagTriangleRight, Trash2, TriangleAlert } from 'lucide-react'
import { Skeleton } from '../../ui/skeleton'
import { Label } from '../../ui/label'
import {
  CardComponent,
  CardContentComponent,
  CardHeaderComponent,
} from '@/components/ui/card'
import { cn } from '@/lib/utils'
import { alarmStore, removeAlarm } from '@/stores/alarmStore'
import type { AlarmFlag } from '@/stores/alarmStore'

// Re-exporting AlarmFlag for compatibility if needed, but better to import from store
export type { AlarmFlag }

export function Flags({ isLoading }: { isLoading: boolean }) {
  const flags = useStore(alarmStore)

  const handleDelete = (id: string) => {
    removeAlarm(id)
  }

  const getTypeStyles = (type: 'ALTO' | 'BAIXO') => {
    return type === 'ALTO'
      ? 'bg-red-900 text-slate-300'
      : 'bg-yellow-600 text-slate-100'
  }

  return (
    <CardComponent className="relative overflow-hidden w-full flex flex-col h-full">
      <CardHeaderComponent className="pb-8">
        <div className="flex items-center gap-3">
          {isLoading ? (
            <>
              <Skeleton className="w-7 h-7 bg-zinc-800 rounded-md" />
              <Skeleton className="w-72 h-7 bg-zinc-800 rounded-md" />
            </>
          ) : (
            <>
              <FlagTriangleRight className="text-slate-300" size={28} />
              <Label className="text-xl font-bold uppercase tracking-wider text-slate-300">
                Flags Ativas e Avisos Recentes
              </Label>
            </>
          )}
        </div>
      </CardHeaderComponent>

      <CardContentComponent className="flex-1">
        {isLoading ? (
          <div className="bg-zinc-900/50 rounded-lg border border-zinc-800 p-3">
            <div className="flex flex-col gap-2">
              {Array.from({ length: 3 }).map((_, i) => (
                <div
                  key={i}
                  className="flex items-center gap-3 p-4 bg-zinc-900/50 rounded-lg border border-zinc-800"
                >
                  <Skeleton className="w-10 h-10 rounded-md bg-zinc-800 flex-shrink-0" />
                  <div className="flex-1 flex flex-col gap-1 min-w-0">
                    <Skeleton className="w-28 h-5 bg-zinc-800 rounded" />
                    <Skeleton className="w-16 h-3 bg-zinc-800 rounded" />
                  </div>
                  <Skeleton className="w-14 h-6 bg-zinc-800 rounded flex-shrink-0" />
                  <Skeleton className="w-12 h-6 bg-zinc-800 rounded flex-shrink-0" />
                  <Skeleton className="w-8 h-8 bg-zinc-800 rounded flex-shrink-0" />
                </div>
              ))}
            </div>
          </div>
        ) : flags.length === 0 ? (
          <div className="bg-zinc-900/50 rounded-lg border border-zinc-800 p-6">
            <div className="flex flex-col items-center justify-center gap-4 text-slate-500 py-8">
              <FlagTriangleRight size={40} className="opacity-30" />
              <p className="text-sm font-medium">
                Nenhuma flag ativa no momento
              </p>
            </div>
          </div>
        ) : (
          <div className="bg-zinc-900/50 rounded-lg border border-zinc-800 p-3 max-h-[440px] overflow-y-auto custom-scrollbar">
            <div className="flex flex-col gap-2">
              {flags.map((flag) => (
                <div
                  key={flag.id}
                  className={cn(
                    'flex items-center gap-3 p-4 bg-zinc-900/50 rounded-lg border border-zinc-800 hover:border-zinc-700 transition-all',
                  )}
                >
                  {/* Ícone de Warning */}
                  <div
                    className={cn(
                      'flex items-center justify-center w-10 h-10 rounded-md flex-shrink-0',
                      getTypeStyles(flag.type),
                    )}
                  >
                    <TriangleAlert size={20} />
                  </div>

                  {/* Nome do parâmetro e timestamp */}
                  <div className="flex-1 min-w-0">
                    <div className="flex flex-col gap-0.5">
                      <span className="text-base font-bold text-slate-300 truncate">
                        {flag.parameter}
                      </span>
                      <span className="text-xs text-slate-500 font-mono">
                        {flag.timestamp}
                      </span>
                    </div>
                  </div>

                  {/* Valor */}
                  <div className="text-base font-bold text-slate-300 flex-shrink-0">
                    {typeof flag.value === 'number'
                      ? flag.value.toFixed(2)
                      : flag.value}
                    °
                  </div>

                  {/* Badge do tipo */}
                  <div
                    className={cn(
                      'px-2 py-1 rounded text-xs font-black uppercase flex-shrink-0',
                      getTypeStyles(flag.type),
                    )}
                  >
                    {flag.type}
                  </div>

                  {/* Botão de deletar */}
                  <button
                    onClick={() => handleDelete(flag.id)}
                    className="flex items-center justify-center w-8 h-8 rounded hover:bg-zinc-800 transition-colors text-slate-400 hover:text-red-400 flex-shrink-0"
                    aria-label="Remover flag"
                  >
                    <Trash2 size={16} />
                  </button>
                </div>
              ))}
            </div>
          </div>
        )}
      </CardContentComponent>
    </CardComponent>
  )
}
